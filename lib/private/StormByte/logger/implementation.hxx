#pragma once

#include <StormByte/logger/typedefs.hxx>
#include <StormByte/string.hxx>

#include <optional>
#include <ostream>
#include <string>

/**
 * @namespace StormByte::Logger
 * @brief Logging utilities for StormByte.
 *
 * Core logging types and helpers used across StormByte. Prefer public facades for a stable
 * API; implementation types (for example `Implementation`) are documented for maintainers and
 * for easier re-export, but may change across releases.
 */
namespace StormByte::Logger {

/**
 * @class Implementation
 * @brief Internal logger implementation (private).
 *
 * Implements the core logging behaviour used by higher-level facades. Provides support for
 * multiple levels, formatting and human-readable numeric/byte output. This is an
 * implementation type; prefer public facades for stable APIs.
 */
class STORMBYTE_LOGGER_PRIVATE Implementation final {
    // Friend declarations for manipulators
    friend STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_number(Implementation& logger) noexcept;
    friend STORMBYTE_LOGGER_PRIVATE Implementation& humanreadable_bytes(Implementation& logger) noexcept;
    friend STORMBYTE_LOGGER_PRIVATE Implementation& nohumanreadable(Implementation& logger) noexcept;

	public:
		// Constructor
			/**
			 * @brief Construct the internal logger implementation.
			 *
			 * @param out Output stream to write log messages to.
			 * @param level Initial minimum `Level` that will be emitted by this logger.
			 *        Messages below this level are suppressed.
			 * @param format Header format string (see public `Log` for format placeholders).
			 * @see StormByte::Logger::Level
			 */
			Implementation(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

		// Non-copyable, movable
		Implementation(const Implementation&) = delete;
		Implementation(Implementation&&) noexcept = default;
		Implementation& operator=(const Implementation&) = delete;
		Implementation& operator=(Implementation&&) noexcept = default;
		~Implementation() noexcept = default;

		const Level& PrintLevel() const noexcept {
			return m_print_level;
		}
		
		const Level& CurrentLevel() const noexcept {
			// Return a reference to the currently set message level if present,
			// otherwise return the configured print level. Avoid using
			// `value_or` because it returns a temporary, which would make the
			// function return a reference to a transient object (warning C4172
			// on MSVC).
			return m_current_level ? *m_current_level : m_print_level;
		}

		// Level selector
		/**
		 * @brief Change the current logging level for subsequent messages.
		 * @param level Level to set for following messages.
		 * @see StormByte::Logger::Level
		 */
		Implementation& operator<<(const Level& level) noexcept;

		// Stream manipulators like std::endl
		Implementation& operator<<(std::ostream& (*manip)(std::ostream&)) noexcept;

		// Custom manipulators that accept Implementation& and return it
		inline Implementation& operator<<(Implementation& (*manip)(Implementation&) noexcept) {
			return manip(*this);
		}

		// Generic streaming operator for supported types
		template <typename T>
		Implementation& operator<<(const T& value) noexcept
			requires (!std::is_same_v<std::decay_t<T>, Implementation& (*)(Implementation&) noexcept>) {
			using DecayedT = std::decay_t<T>;

			// Fast paths: avoid allocating temporary std::string when possible.
			const Level effective = m_current_level.value_or(m_print_level);
			if (effective < m_print_level) {
				// Message suppressed; keep state unchanged and return.
				return *this;
			}

			if constexpr (std::is_same_v<DecayedT, bool>) {
				if (!m_header_displayed) { print_header(); m_header_displayed = true; }
				m_out << (value ? "true" : "false");
			}
			else if constexpr (std::is_same_v<DecayedT, wchar_t>) {
				// Single wide character: convert and print as UTF-8
				print_message(value);
			}
			else if constexpr (std::is_integral_v<DecayedT> || std::is_floating_point_v<DecayedT>) {
				// Human-readable formatting is relatively expensive; only use it when enabled.
				if (m_human_readable_format == String::Format::Raw) {
					if (!m_header_displayed) { print_header(); m_header_displayed = true; }
					if constexpr (std::is_integral_v<DecayedT>) {
						m_out << value;
					} else {
						// Preserve previous textual formatting for floating-point numbers
						// which used std::to_string(value).
						m_out << std::to_string(value);
					}
				} else {
					std::string message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
					if (!m_header_displayed) { print_header(); m_header_displayed = true; }
					m_out << message;
				}
			}
			else if constexpr (std::is_same_v<DecayedT, std::string>) {
				if (!m_header_displayed) { print_header(); m_header_displayed = true; }
				m_out << value;
			}
			else if constexpr (std::is_same_v<DecayedT, const char*>) {
				if (!m_header_displayed) { print_header(); m_header_displayed = true; }
				m_out << value;
			}
			else if constexpr (std::is_same_v<DecayedT, std::wstring>) {
				std::string utf8 = String::UTF8Encode(value);
				if (!m_header_displayed) { print_header(); m_header_displayed = true; }
				m_out << utf8;
			}
			else if constexpr (std::is_same_v<DecayedT, const wchar_t*>) {
				std::string utf8 = String::UTF8Encode(std::wstring(value));
				if (!m_header_displayed) { print_header(); m_header_displayed = true; }
				m_out << utf8;
			}
			else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>) {
				if (!m_header_displayed) { print_header(); m_header_displayed = true; }
				m_out << value;
			}
			else {
				static_assert(!std::is_same_v<T, T>, "Unsupported type for Implementation::operator<<");
			}
			return *this;
		}

	private:
		std::ostream& m_out;
			/** @brief Print level. See `StormByte::Logger::Level` for values. */
			Level m_print_level;
			/** @brief Current level for the in-progress message (if any). */
			std::optional<Level> m_current_level;
		bool m_header_displayed;                        ///< Line started
		const std::string m_format;                     ///< Custom user format %L for Level and %T for Time
		String::Format m_human_readable_format;         ///< Human readable size
		// Line state

		// Internal helpers
		void print_time() const noexcept;
		void print_level() const noexcept;
		void print_thread_id() const noexcept;
		void print_header() const noexcept;

		// (No synchronization helpers — logging is not responsible for cross-thread locking)

		template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, wchar_t>>>
		void print_message(const T& value) noexcept {
			std::string message;
			if (m_human_readable_format == String::Format::Raw) {
				message = std::to_string(value);
			}
			else {
				message = String::HumanReadable(value, m_human_readable_format, "en_US.UTF-8");
			}
			print_message(message);
		}

		void print_message(const std::string& message) noexcept;
		void print_message(const wchar_t& value) noexcept;
};

// Manipulators
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

// Helper overloads for pointer-wrapped loggers
template <typename Ptr, typename T>
Ptr& operator<<(Ptr& logger, const T& value) noexcept
    requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
    if (logger) {
        *logger << value;
    }
    return logger;
}

template <typename Ptr>
Ptr& operator<<(Ptr& logger, const Level& level) noexcept
    requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
    if (logger) {
        *logger << level;
    }
    return logger;
}

template <typename Ptr>
Ptr& operator<<(Ptr& logger, std::ostream& (*manip)(std::ostream&)) noexcept
    requires std::is_same_v<Ptr, std::shared_ptr<Implementation>> || std::is_same_v<Ptr, std::unique_ptr<Implementation>> {
    if (logger) {
        *logger << manip;
    }
    return logger;
}

}
