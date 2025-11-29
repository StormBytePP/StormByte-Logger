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
 * API; implementation types (for example `LogImpl`) are documented for maintainers and
 * for easier re-export, but may change across releases.
 */
namespace StormByte::Logger {

/**
 * @class LogImpl
 * @brief Internal logger implementation (private).
 *
 * Implements the core logging behaviour used by higher-level facades. Provides support for
 * multiple levels, formatting and human-readable numeric/byte output. This is an
 * implementation type; prefer public facades for stable APIs.
 */
class STORMBYTE_LOGGER_PRIVATE LogImpl final {
    // Friend declarations for manipulators
    friend STORMBYTE_LOGGER_PRIVATE LogImpl& humanreadable_number(LogImpl& logger) noexcept;
    friend STORMBYTE_LOGGER_PRIVATE LogImpl& humanreadable_bytes(LogImpl& logger) noexcept;
    friend STORMBYTE_LOGGER_PRIVATE LogImpl& nohumanreadable(LogImpl& logger) noexcept;

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
        LogImpl(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

    // Non-copyable, movable
    LogImpl(const LogImpl&) = delete;
    LogImpl(LogImpl&&) noexcept = default;
    LogImpl& operator=(const LogImpl&) = delete;
    LogImpl& operator=(LogImpl&&) noexcept = default;
    ~LogImpl() noexcept = default;

    // Level selector
        /**
         * @brief Change the current logging level for subsequent messages.
         * @param level Level to set for following messages.
         * @see StormByte::Logger::Level
         */
        LogImpl& operator<<(const Level& level) noexcept;

    // Stream manipulators like std::endl
    LogImpl& operator<<(std::ostream& (*manip)(std::ostream&)) noexcept;

    // Custom manipulators that accept LogImpl& and return it
    inline LogImpl& operator<<(LogImpl& (*manip)(LogImpl&) noexcept) {
        return manip(*this);
    }

    // Generic streaming operator for supported types
    template <typename T>
    LogImpl& operator<<(const T& value) noexcept
        requires (!std::is_same_v<std::decay_t<T>, LogImpl& (*)(LogImpl&) noexcept>) {
        using DecayedT = std::decay_t<T>;

        if constexpr (std::is_same_v<DecayedT, bool>) {
            print_message(value ? "true" : "false");
        }
        else if constexpr (std::is_integral_v<DecayedT> || std::is_floating_point_v<DecayedT>) {
            print_message(value);
        }
        else if constexpr (std::is_same_v<DecayedT, std::string> || std::is_same_v<DecayedT, const char*>) {
            print_message(std::string(value));
        }
        else if constexpr (std::is_same_v<DecayedT, std::wstring> || std::is_same_v<DecayedT, const wchar_t*>) {
            print_message(String::UTF8Encode(value));
        }
        else if constexpr (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>) {
            print_message(std::string(value));
        }
        else {
            static_assert(!std::is_same_v<T, T>, "Unsupported type for LogImpl::operator<<");
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
inline STORMBYTE_LOGGER_PRIVATE LogImpl& humanreadable_number(LogImpl& logger) noexcept {
    logger.m_human_readable_format = String::Format::HumanReadableNumber;
    return logger;
}

inline STORMBYTE_LOGGER_PRIVATE LogImpl& humanreadable_bytes(LogImpl& logger) noexcept {
    logger.m_human_readable_format = String::Format::HumanReadableBytes;
    return logger;
}

inline STORMBYTE_LOGGER_PRIVATE LogImpl& nohumanreadable(LogImpl& logger) noexcept {
    logger.m_human_readable_format = String::Format::Raw;
    return logger;
}

// Helper overloads for pointer-wrapped loggers
template <typename Ptr, typename T>
Ptr& operator<<(Ptr& logger, const T& value) noexcept
    requires std::is_same_v<Ptr, std::shared_ptr<LogImpl>> || std::is_same_v<Ptr, std::unique_ptr<LogImpl>> {
    if (logger) {
        *logger << value;
    }
    return logger;
}

template <typename Ptr>
Ptr& operator<<(Ptr& logger, const Level& level) noexcept
    requires std::is_same_v<Ptr, std::shared_ptr<LogImpl>> || std::is_same_v<Ptr, std::unique_ptr<LogImpl>> {
    if (logger) {
        *logger << level;
    }
    return logger;
}

template <typename Ptr>
Ptr& operator<<(Ptr& logger, std::ostream& (*manip)(std::ostream&)) noexcept
    requires std::is_same_v<Ptr, std::shared_ptr<LogImpl>> || std::is_same_v<Ptr, std::unique_ptr<LogImpl>> {
    if (logger) {
        *logger << manip;
    }
    return logger;
}

}
