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

			Implementation(const Implementation&) = delete;
			Implementation(Implementation&&) noexcept = delete;
			Implementation& operator=(const Implementation&) = delete;
			Implementation& operator=(Implementation&&) noexcept = delete;
			~Implementation() noexcept = default;

			const Level& PrintLevel() const noexcept {
				return m_print_level;
			}

			const Level& CurrentLevel() const noexcept {
				return m_current_level ? *m_current_level : m_print_level;
			}

			/**
			 * @brief Whether the current message level will be emitted.
			 */
			bool Enabled() const noexcept {
				return m_enabled.load(std::memory_order_acquire);
			}

			/**
			 * @brief Enable or disable redaction for subsequent string-like values.
			 * @param active true to redact text.
			 * @param keep_last 0 = mask all characters; N = keep last N characters visible.
			 */
			void SetRedact(bool active, std::size_t keep_last) noexcept {
				m_redact_active = active;
				m_redact_keep_last = keep_last;
			}

			Implementation& operator<<(const Level& level) noexcept;
			Implementation& operator<<(std::ostream& (*manip)(std::ostream&)) noexcept;

			inline Implementation& operator<<(Implementation& (*manip)(Implementation&) noexcept) {
				return manip(*this);
			}

			template <typename T>
			Implementation& operator<<(const T& value) noexcept
				requires (!std::is_same_v<std::decay_t<T>, Implementation& (*)(Implementation&) noexcept>) {
				using DecayedT = std::decay_t<T>;

				if (!m_enabled.load(std::memory_order_acquire)) [[likely]] {
					return *this;
				}

				if constexpr (std::is_same_v<DecayedT, bool>) {
					ensure_header();
					m_out << (value ? "true" : "false");
				}
				else if constexpr (std::is_same_v<DecayedT, wchar_t>) {
					print_message(value);
				}
				else if constexpr (std::is_integral_v<DecayedT> || std::is_floating_point_v<DecayedT>) {
					if (m_human_readable_format == String::Format::Raw) {
						ensure_header();
						if constexpr (std::is_integral_v<DecayedT>) {
							m_out << value;
						} else {
							m_out << std::to_string(value);
						}
					} else {
						std::string message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
						ensure_header();
						m_out << message;
					}
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
			std::ostream& m_out;
			Level m_print_level;
			std::optional<Level> m_current_level;
			std::atomic<bool> m_enabled;
			bool m_header_displayed;
			const std::string m_format;
			String::Format m_human_readable_format;
			bool m_redact_active;			///< When true, string-like values are redacted
			std::size_t m_redact_keep_last;	///< 0 = all '*'; N = keep last N chars

			void ensure_header() noexcept {
				if (!m_header_displayed) {
					print_header();
					m_header_displayed = true;
				}
			}

			/**
			 * @brief Apply redaction policy to a text view.
			 * @param in Input text.
			 * @param keep_last 0 = all '*'; N = last N characters preserved.
			 * @return Redacted string (same length as @p in when masking).
			 */
			static std::string ApplyRedact(std::string_view in, std::size_t keep_first) {
				if (in.empty())
					return {};
				if (keep_first >= in.size())
					return std::string{in};
				std::string out(in.size(), '*');
				if (keep_first > 0) {
					for (std::size_t i = 0; i < keep_first; ++i)
						out[i] = in[i];
				}
				return out;
			}

			void write_text(std::string_view text) noexcept {
				ensure_header();
				if (m_redact_active)
					m_out << ApplyRedact(text, m_redact_keep_last);
				else
					m_out << text;
			}

			void write_text(const std::string& text) noexcept {
				write_text(std::string_view{text});
			}

			void print_time() const noexcept;
			std::string CurrentTime() const noexcept;
			void print_level() const noexcept;
			void print_thread_id() const noexcept;
			void print_header() const noexcept;

			template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, wchar_t>>>
			void print_message(const T& value) noexcept {
				std::string message;
				if (m_human_readable_format == String::Format::Raw)
					message = std::to_string(value);
				else
					message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
				print_message(message);
			}

			void print_message(const std::string& message) noexcept;
			void print_message(const wchar_t& value) noexcept;
	};

	inline STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_number(Implementation& logger) noexcept {
		logger.m_human_readable_format = String::Format::HumanReadableNumber;
		return logger;
	}

	inline STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_bytes(Implementation& logger) noexcept {
		logger.m_human_readable_format = String::Format::HumanReadableBytes;
		return logger;
	}

	inline STORMBYTE_LOGGER_PRIVATE Implementation& nohumanreadable(Implementation& logger) noexcept {
		logger.m_human_readable_format = String::Format::Raw;
		return logger;
	}

	template <typename Ptr, typename T>
	Ptr& operator<<(Ptr& logger, const T& value) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << value;
		return logger;
	}

	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, const Level& level) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << level;
		return logger;
	}

	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, std::ostream& (*manip)(std::ostream&)) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
		if (logger)
			*logger << manip;
		return logger;
	}
}