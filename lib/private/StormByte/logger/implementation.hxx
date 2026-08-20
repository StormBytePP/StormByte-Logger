/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/logger/typedefs.hxx>
#include <StormByte/string.hxx>

#include <atomic>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

/**
 * @namespace StormByte::Logger
 * @brief Logging utilities for StormByte.
 */
namespace StormByte::Logger {
	/**
	 * @class Implementation
	 * @brief Internal logger implementation (private).
	 *
	 * Thread-safety note: `m_enabled` is atomic so `Enabled()` / filtered fast-paths may be
	 * observed concurrently with `operator<<(Level)` (as with `ThreadedLog`). Full multi-threaded
	 * emission still requires `ThreadedLog` (line lock around actual writes).
	 */
	class STORMBYTE_LOGGER_PRIVATE Implementation final {
		friend STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_number(Implementation& logger) noexcept;
		friend STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_bytes(Implementation& logger) noexcept;
		friend STORMBYTE_LOGGER_PRIVATE Implementation& nohumanreadable(Implementation& logger) noexcept;

		public:
			/**
			 * @brief Construct the internal logger implementation.
			 * @param out Output stream to write log messages to.
			 * @param level Initial minimum Level that will be emitted.
			 * @param format Header format string (%L, %T, %i, %%).
			 */
			Implementation(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

			/**
			 * @brief Copy constructor (deleted).
			 */
			Implementation(const Implementation&) = delete;

			/**
			 * @brief Move constructor (deleted).
			 */
			Implementation(Implementation&&) noexcept = delete;

			/**
			 * @brief Copy assignment operator (deleted).
			 * @return Reference to this object.
			 */
			Implementation& operator=(const Implementation&) = delete;

			/**
			 * @brief Move assignment operator (deleted).
			 * @return Reference to this object.
			 */
			Implementation& operator=(Implementation&&) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Implementation() noexcept = default;

			/**
			 * @brief Get the minimum print level.
			 * @return Current minimum Level.
			 */
			const Level& PrintLevel() const noexcept {
				return m_print_level;
			}

			/**
			 * @brief Get the level of the current message.
			 * @return Current message Level (or print level if none set).
			 */
			const Level& CurrentLevel() const noexcept {
				return m_current_level ? *m_current_level : m_print_level;
			}

			/**
			 * @brief Whether the current message level will be emitted.
			 * @return true if the message will be written.
			 */
			bool Enabled() const noexcept {
				return m_enabled.load(std::memory_order_acquire);
			}

			/**
			 * @brief Enable or disable redaction for subsequent values.
			 * @param active true to redact text and numbers.
			 * @param count 0 = mask all characters; N = keep N characters.
			 * @param keep_first true = keep first N characters, false = keep last N characters.
			 */
			void SetRedact(bool active, std::size_t count, bool keep_first) noexcept {
				m_redact_active = active;
				m_redact_count = count;
				m_redact_keep_first = keep_first;
			}

			/**
			 * @brief Set the current logging level.
			 * @param level New Level for subsequent messages.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(const Level& level) noexcept;

			/**
			 * @brief Forward a standard stream manipulator.
			 * @param manip Stream manipulator (e.g. std::endl).
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(std::ostream& (*manip)(std::ostream&)) noexcept;

			/**
			 * @brief Apply an Implementation-specific manipulator.
			 * @param manip Manipulator function.
			 * @return Reference to this Implementation.
			 */
			inline Implementation& operator<<(Implementation& (*manip)(Implementation&) noexcept) {
				return manip(*this);
			}

			/**
			 * @brief Stream a value into the log.
			 * @tparam T Type of the value.
			 * @param value Value to write.
			 * @return Reference to this Implementation.
			 */
			template <typename T>
			Implementation& operator<<(const T& value) noexcept
				requires (!std::is_same_v<std::decay_t<T>, Implementation& (*)(Implementation&) noexcept>) {
				using DecayedT = std::decay_t<T>;

				if (!m_enabled.load(std::memory_order_acquire)) [[likely]] {
					return *this;
				}

				if constexpr (std::is_same_v<DecayedT, bool>) {
					write_text(std::string_view{value ? "true" : "false"});
				}
				else if constexpr (std::is_same_v<DecayedT, wchar_t>) {
					print_message(value);
				}
				else if constexpr (std::is_integral_v<DecayedT> || std::is_floating_point_v<DecayedT>) {
					std::string message;
					if (m_human_readable_format == String::Format::Raw) {
						message = std::to_string(value);
					} else {
						message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
					}
					write_text(message);
				}
				else if constexpr (std::is_same_v<DecayedT, std::string>) {
					write_text(value);
				}
				else if constexpr (std::is_same_v<DecayedT, const char*>) {
					write_text(value ? std::string_view{value} : std::string_view{});
				}
				else if constexpr (std::is_same_v<DecayedT, std::wstring>) {
					write_text(String::UTF8Encode(value));
				}
				else if constexpr (std::is_same_v<DecayedT, const wchar_t*>) {
					write_text(value ? String::UTF8Encode(std::wstring(value)) : std::string{});
				}
				else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>) {
					write_text(std::string_view{value});
				}
				else {
					static_assert(!std::is_same_v<T, T>, "Unsupported type for Implementation::operator<<");
				}
				return *this;
			}

		private:
			std::ostream& m_out;						///< Output stream
			Level m_print_level;						///< Minimum level that will be printed
			std::optional<Level> m_current_level;		///< Level of the current message
			std::atomic<bool> m_enabled;				///< Whether the current level is enabled
			bool m_header_displayed;					///< Whether the header has already been written
			const std::string m_format;					///< Header format string
			String::Format m_human_readable_format;		///< Current human-readable format
			bool m_redact_active;						///< When true, text and numbers are redacted
			std::size_t m_redact_count;					///< 0 = all '*'; N = keep N chars
			bool m_redact_keep_first;					///< true = keep first N, false = keep last N

			/**
			 * @brief Ensure the header has been printed for the current line.
			 */
			void ensure_header() noexcept {
				if (!m_header_displayed) {
					print_header();
					m_header_displayed = true;
				}
			}

			/**
			 * @brief Apply redaction policy.
			 * @param in Input text.
			 * @param count 0 = all '*'; N = keep N characters.
			 * @param keep_first true = keep first N, false = keep last N.
			 * @return Redacted string of the same length.
			 */
			static std::string ApplyRedact(std::string_view in, std::size_t count, bool keep_first) {
				if (in.empty())
					return {};
				if (count >= in.size())
					return std::string{in};

				std::string out(in.size(), '*');
				if (keep_first) {
					for (std::size_t i = 0; i < count; ++i)
						out[i] = in[i];
				} else {
					const std::size_t start = in.size() - count;
					for (std::size_t i = 0; i < count; ++i)
						out[start + i] = in[start + i];
				}
				return out;
			}

			/**
			 * @brief Write text, applying redaction if active.
			 * @param text Text to write.
			 */
			void write_text(std::string_view text) noexcept {
				ensure_header();
				if (m_redact_active)
					m_out << ApplyRedact(text, m_redact_count, m_redact_keep_first);
				else
					m_out << text;
			}

			/**
			 * @brief Write a std::string, applying redaction if active.
			 * @param text Text to write.
			 */
			void write_text(const std::string& text) noexcept {
				write_text(std::string_view{text});
			}

			/**
			 * @brief Print the current timestamp.
			 */
			void print_time() const noexcept;

			/**
			 * @brief Get the current time as a formatted string.
			 * @return Formatted time string.
			 */
			std::string CurrentTime() const noexcept;

			/**
			 * @brief Print the current level name (padded).
			 */
			void print_level() const noexcept;

			/**
			 * @brief Print the current thread id.
			 */
			void print_thread_id() const noexcept;

			/**
			 * @brief Print the configured header.
			 */
			void print_header() const noexcept;

			/**
			 * @brief Helper to print an arithmetic value (with optional human-readable formatting).
			 * @tparam T Arithmetic type.
			 * @param value Value to print.
			 */
			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, wchar_t>>>
			void print_message(const T& value) noexcept {
				std::string message;
				if (m_human_readable_format == String::Format::Raw)
					message = std::to_string(value);
				else
					message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
				print_message(message);
			}

			/**
			 * @brief Print a string message.
			 * @param message Message to print.
			 */
			void print_message(const std::string& message) noexcept;

			/**
			 * @brief Print a wide character.
			 * @param value Wide character to print.
			 */
			void print_message(const wchar_t& value) noexcept;
	};

	/**
	 * @brief Enable human-readable number formatting.
	 * @param logger Implementation to modify.
	 * @return Reference to the same Implementation.
	 */
	inline STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_number(Implementation& logger) noexcept {
		logger.m_human_readable_format = String::Format::HumanReadableNumber;
		return logger;
	}

	/**
	 * @brief Enable human-readable byte formatting.
	 * @param logger Implementation to modify.
	 * @return Reference to the same Implementation.
	 */
	inline STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_bytes(Implementation& logger) noexcept {
		logger.m_human_readable_format = String::Format::HumanReadableBytes;
		return logger;
	}

	/**
	 * @brief Disable human-readable formatting.
	 * @param logger Implementation to modify.
	 * @return Reference to the same Implementation.
	 */
	inline STORMBYTE_LOGGER_PRIVATE Implementation& nohumanreadable(Implementation& logger) noexcept {
		logger.m_human_readable_format = String::Format::Raw;
		return logger;
	}

	/**
	 * @brief Stream a value into a smart pointer to Implementation.
	 * @tparam Ptr Smart pointer type.
	 * @tparam T Value type.
	 * @param logger Smart pointer to Implementation.
	 * @param value Value to stream.
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr, typename T>
	Ptr& operator<<(Ptr& logger, const T& value) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << value;
		return logger;
	}

	/**
	 * @brief Stream a Level into a smart pointer to Implementation.
	 * @tparam Ptr Smart pointer type.
	 * @param logger Smart pointer to Implementation.
	 * @param level Level to set.
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, const Level& level) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << level;
		return logger;
	}

	/**
	 * @brief Stream a stream manipulator into a smart pointer to Implementation.
	 * @tparam Ptr Smart pointer type.
	 * @param logger Smart pointer to Implementation.
	 * @param manip Stream manipulator.
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, std::ostream& (*manip)(std::ostream&)) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << manip;
		return logger;
	}
}
