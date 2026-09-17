/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Logger.
 *
 * StormByte-Logger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
 *
 * StormByte-Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte-Logger. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#pragma once

#include <StormByte/logger/typedefs.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <StormByte/string.hxx>
#include <StormByte/type_traits.hxx>

#include <array>
#include <atomic>
#include <cstdint>
#include <optional>
#include <ostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

/**
 * @namespace StormByte::Logger
 * @brief Logger module of the StormByte suite.
 */
namespace StormByte::Logger {
	struct ThrottleRuleState {
		std::atomic<std::uint64_t> dropped{0};
		std::atomic<std::uint64_t> sample_count{0};
		std::atomic<std::uint64_t> window_count{0};
		std::atomic<std::uint64_t> finite_credits{0};
		std::atomic<std::int64_t> next_token_ns{0};
	};
	struct ThrottleRule {
		ThrottleSpec spec;
		std::shared_ptr<ThrottleRuleState> state;
	};
	struct ThrottleTable {
		std::vector<ThrottleRule> rules;
	};

	struct ColorManip;
	struct NoColorManip;
	struct FormatManip;
	struct PopFormatManip;
	struct GroupManip;
	struct ComponentManip;
	struct ResetComponentManip;

	/**
	 * @class Implementation
	 * @brief Internal logger implementation (private).
	 *
	 * Thread-safety note: `m_enabled` is atomic so `Enabled()` / filtered fast-paths may be
	 * observed concurrently with `operator<<(Level)` (as with `ThreadedLog`). Full multi-threaded
	 * emission still requires `ThreadedLog` (line lock around actual writes).
	 * Throttle rules use atomic shared-pointer publication; Logger does not promise
	 * that the standard library implementation is physically lock-free.
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
			 * @param format Header format string (%L, %T, %i, %c, %g, %%).
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
			~Implementation() noexcept;

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
			const Level& CurrentLevel() const noexcept;

			/**
			 * @brief Whether the current message level will be emitted.
			 * @return true if the message will be written.
			 * @note Warning, Error and Fatal remain enabled even when below the
			 *       configured print level.
			 */
			bool Enabled() const noexcept;

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
			 * @brief Set the configured color for a logging level.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that level.
			 */
			void Color(const Level& level, const StormByte::Logger::Color& color) noexcept;

			/**
			 * @brief Get the configured color for a logging level.
			 * @param level Level whose color is requested.
			 * @return Configured color.
			 */
			StormByte::Logger::Color Color(const Level& level) const noexcept;

			/**
			 * @brief Set a color override for a component and level.
			 * @param component Component name.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that component and level.
			 */
			void Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color);

			/**
			 * @brief Get a component color, falling back to the general color.
			 * @param component Component name.
			 * @param level Level whose color is requested.
			 * @return Component override or general color.
			 */
			StormByte::Logger::Color Color(const std::string& component, const Level& level) const noexcept;

			/**
			 * @brief Set the general header format.
			 * @param format Format used when no component or temporary override applies.
			 */
			void Format(const std::string& format);

			/**
			 * @brief Get the effective current header format.
			 * @return Temporary, component-specific or general format.
			 */
			const std::string& Format() const noexcept;

			/**
			 * @brief Set or remove a component-specific header format.
			 * @param component Component name; empty selects the general format.
			 * @param format Format, or empty to remove the component override.
			 */
			void Format(const std::string& component, const std::string& format);

			/**
			 * @brief Get a component-specific format, falling back to the general format.
			 * @param component Component name.
			 * @return Component format or general format.
			 */
			const std::string& Format(const std::string& component) const noexcept;

			/**
			 * @brief Install a throttle rule.
			 * @param spec Rule to install.
			 */
			void Throttle(const ThrottleSpec& spec);

			/**
			 * @brief Remove a throttle rule with the same selectors.
			 * @param spec Selectors of the rule to remove.
			 */
			void NoThrottle(const ThrottleSpec& spec);

			/**
			 * @brief Remove all throttle rules.
			 */
			void NoThrottleAll() noexcept;
			/** @brief Flush all dropped summaries without changing throttle rules. */
			void FlushThrottle();
			/** @brief Flush matching dropped summaries without changing throttle rules. */
			void FlushThrottle(const ThrottleSpec& spec);

			/**
			 * @brief Decide whether the current line may emit.
			 * @return true when the line is admitted.
			 */
			bool PrepareLine();

			/**
			 * @brief Whether the current thread's line was admitted.
			 * @return true when payload output is allowed.
			 */
			bool LineAdmitted() const noexcept;
			/** @brief Whether throttle has already decided the current line. */
			bool LineDecided() const noexcept;

			/**
			 * @brief Whether a header/output line is currently open.
			 * @return true when output has started for the line.
			 */
			bool HasOpenOutputLine() const noexcept;
			void BeginOutputLine() noexcept;

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
			 * @brief Apply a temporary content color manipulator.
			 * @param manip Color selection to apply.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(ColorManip manip) noexcept;

			/**
			 * @brief Disable content color until another color manipulator or endl.
			 * @param manip No-color manipulator.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(NoColorManip manip) noexcept;

			/**
			 * @brief Push and activate a temporary format.
			 * @param manip Format manipulator.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(FormatManip manip);

			/**
			 * @brief Restore the last saved format when one exists.
			 * @param manip Pop-format manipulator.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(PopFormatManip manip) noexcept;

			/**
			 * @brief Set the producer group for the current line.
			 * @param manip Group manipulator.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(GroupManip manip);

			/**
			 * @brief Select the current thread's component.
			 * @param manip Component manipulator.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(ComponentManip manip);

			/**
			 * @brief Clear the current thread's component.
			 * @param manip Reset-component manipulator.
			 * @return Reference to this Implementation.
			 */
			Implementation& operator<<(ResetComponentManip manip);

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
				Implementation& operator<<(const T& value)
				requires (!StormByte::Type::SameAs<T, Implementation& (*)(Implementation&) noexcept>) {
				using DecayedT = std::decay_t<T>;

				if (!Enabled()) [[likely]] {
					return *this;
				}

				if constexpr (StormByte::Type::SameAs<DecayedT, bool>) {
					write_text(std::string_view{value ? "true" : "false"});
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, wchar_t>) {
					print_message(value);
				}
				else if constexpr (StormByte::Type::Arithmetic<DecayedT>) {
					std::string message;
					if (m_human_readable_format == String::Format::Raw) {
						message = std::to_string(value);
					} else {
						message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
					}
					write_text(message);
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, std::string_view>) {
					write_text(value);
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, std::string>) {
					write_text(value);
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, const char*>) {
					write_text(value ? std::string_view{value} : std::string_view{});
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, std::wstring_view>) {
					write_text(String::UTF8Encode(value));
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, std::wstring>) {
					write_text(String::UTF8Encode(value));
				}
				else if constexpr (StormByte::Type::SameAs<DecayedT, const wchar_t*>) {
					write_text(value ? String::UTF8Encode(std::wstring_view{value}) : std::string{});
				}
				else {
					static_assert(!StormByte::Type::SameAs<T, T>, "Unsupported type for Implementation::operator<<");
				}
				return *this;
			}

		private:
			std::ostream& m_out;                                      ///< Output stream
			Level m_print_level;                                      ///< Minimum level that will be printed
			std::optional<Level> m_current_level;                     ///< Level of the current message
			std::atomic<bool> m_enabled;                              ///< Whether the current level is enabled
			std::string m_format;                                     ///< Header format string
			std::vector<std::string> m_format_stack;                  ///< Temporary formats for push/pop
			std::unordered_map<std::string, std::string> m_component_formats; ///< Persistent component formats
			String::Format m_human_readable_format;                   ///< Current human-readable format
			bool m_redact_active;                                     ///< When true, text and numbers are redacted
			std::size_t m_redact_count;                               ///< 0 = all '*'; N = keep N chars
			bool m_redact_keep_first;                                 ///< true = keep first N, false = keep last N
			std::array<StormByte::Logger::Color, 7> m_level_colors{}; ///< Configured color per level
			std::unordered_map<std::string, std::array<StormByte::Logger::Color, 7>> m_component_colors; ///< Component color overrides
			std::optional<StormByte::Logger::Color> m_content_color;  ///< Temporary content color override
			bool m_content_nocolor = false;                           ///< Whether content color is suppressed
			std::optional<StormByte::Logger::Color> m_active_color;   ///< Color currently emitted to the stream
			#if defined(WINDOWS) || defined(__GLIBCXX__)
			std::atomic<std::shared_ptr<const ThrottleTable>> m_throttle_table; ///< Immutable rules snapshot
			#else
			std::shared_ptr<const ThrottleTable> m_throttle_table;        ///< Immutable rules snapshot, atomically accessed
			#endif

			/**
			 * @brief Ensure the header has been printed for the current line.
			 */
			void ensure_header() noexcept {
				if (!PrepareLine())
					return;
				close_deferred_line();
				if (!HasOpenOutputLine()) {
					BeginOutputLine();
					write_drop_summary();
					print_header();
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
				if (!LineAdmitted())
					return;
				sync_content_color();
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
			void print_header() noexcept;

			/**
			 * @brief Synchronize the stream color with the current content mode.
			 */
			void sync_content_color() noexcept;

			/**
			 * @brief Emit a color transition when needed.
			 * @param color Desired ANSI color.
			 */
			void emit_color(StormByte::Logger::Color color) noexcept;

			/**
			 * @brief Reset any ANSI color currently emitted to the stream.
			 */
			void reset_color() noexcept;
			/** @brief Close a previous partial line before opening this one. */
			void close_deferred_line() noexcept;

			/** @brief Reset all line-local throttle and snapshot state. */
			void reset_line_state() noexcept;

			/** @brief Write the pending dropped summary without throttling it. */
			void write_drop_summary() noexcept;

			/** @brief Load the immutable throttle table atomically. */
			std::shared_ptr<const ThrottleTable> LoadThrottleTable() const noexcept;
			/** @brief Publish the immutable throttle table atomically. */
			void StoreThrottleTable(std::shared_ptr<const ThrottleTable> table) noexcept;

			/**
			 * @brief Resolve the format selected by the current component and stack.
			 * @return Effective header format.
			 */
			const std::string& effective_format() const noexcept;

			/**
			 * @brief Helper to print an arithmetic value (with optional human-readable formatting).
			 * @tparam T Arithmetic type.
			 * @param value Value to print.
			 */
			template <typename T>
			requires StormByte::Type::Arithmetic<T> && (!StormByte::Type::SameAs<T, wchar_t>)
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
			void print_message(const wchar_t& value);
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

	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<bool>(const bool& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<short>(const short& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned short>(const unsigned short& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<int>(const int& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned int>(const unsigned int& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long>(const long& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long>(const unsigned long& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long long>(const long long& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long long>(const unsigned long long& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<float>(const float& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<double>(const double& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long double>(const long double& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<char>(const char& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<signed char>(const signed char& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned char>(const unsigned char& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<wchar_t>(const wchar_t& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::string>(const std::string& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::wstring>(const std::wstring& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const char*>(const char* const& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const wchar_t*>(const wchar_t* const& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::string_view>(const std::string_view& value);
	extern template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::wstring_view>(const std::wstring_view& value);

	/**
	 * @brief Stream a value into a smart pointer to Implementation.
	 * @tparam Ptr Smart pointer type.
	 * @tparam T Value type.
	 * @param logger Smart pointer to Implementation.
	 * @param value Value to stream.
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr, typename T>
	Ptr& operator<<(Ptr& logger, const T& value)
		requires StormByte::Type::SameAs<Ptr, std::shared_ptr<Implementation>> || StormByte::Type::SameAs<Ptr, std::unique_ptr<Implementation>> {
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
		requires StormByte::Type::SameAs<Ptr, std::shared_ptr<Implementation>> || StormByte::Type::SameAs<Ptr, std::unique_ptr<Implementation>> {
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
		requires StormByte::Type::SameAs<Ptr, std::shared_ptr<Implementation>> || StormByte::Type::SameAs<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << manip;
		return logger;
	}
}
