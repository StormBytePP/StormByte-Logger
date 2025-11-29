#pragma once

#include <StormByte/logger/manipulators.hxx>
#include <StormByte/logger/typedefs.hxx>

#include <memory>
#include <string>
#include <ostream>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <thread>

/**
 * @namespace StormByte::Logger
 * @brief Logging module for StormByte library
 *
 * This namespace contains the logging types and utilities used throughout the StormByte project.
 */
namespace StormByte::Logger {
	class LogImpl; ///< Forward declaration of internal LogImpl class
	/**
	 * @class Log
	 * @brief Public logging facade.
	 *
	 * `Log` is a small, stable public wrapper that owns a shared pointer to the
	 * internal logger implementation. Use this class for application code — it
	 * forwards streaming operations and manipulators to the internal
	 * implementation (`LogImpl`).
	 *
	 * @note Thread-safety: `Log` provides line-atomic output. The internal
	 * implementation acquires a lock when a logical log entry begins and holds it
	 * while the entry is composed. The lock is released when a line-terminating
	 * manipulator (for example `std::endl`) is applied. To avoid interleaving of
	 * partial log lines from multiple threads, terminate log lines with a stream
	 * manipulator such as `std::endl` when using `Log` concurrently from multiple
	 * threads.
	 */
	class STORMBYTE_LOGGER_PUBLIC Log final {
		// Allow free manipulators to access `m_impl`
		friend STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& atomic_start(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& atomic_end(Log& log) noexcept;

		public:
			using AtomicGuardHandle = std::unique_ptr<void, void(*)(void*)>;

			/**
			 * @brief Create an RAII guard that pairs `atomic_start`/`atomic_end`.
			 *
			 * The returned handle is opaque (private type erased) and will call
			 * `atomic_end` when destroyed. Use this to ensure atomic log sequences
			 * without explicitly calling the manipulators.
			 */
			AtomicGuardHandle make_atomic_guard() noexcept;
			/**
			 * @brief Construct a `Log` writing to `out`.
			 *
			 * @param out Output stream to write log messages to (for example `std::cout`).
			 * @param level Minimum `Level` that will be emitted. Messages below this
			 *        level are suppressed.
			 * @see StormByte::Logger::Level
			 * @param format Format string used for log headers. Supported placeholders:
			 *        - `%L` : level name (e.g. "Info", "Error").
			 *        - `%T` : timestamp (formatted as `dd/mm/YYYY HH:MM:SS`).
			 *        All other characters are copied verbatim into the header.
			 */
			Log(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

			/**
			 * @brief Copy constructor.
			 *
			 * Creates a new `Log` that shares the same underlying implementation.
			 */
			Log(const Log&) = default;

			/**
			 * @brief Move constructor.
			 *
			 * Move a `Log` preserving ownership of the internal implementation.
			 */
			Log(Log&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Log() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @return Reference to this `Log`.
			 */
			Log& operator=(const Log&) = default;

			/**
			 * @brief Move assignment.
			 * @return Reference to this `Log`.
			 */
			Log& operator=(Log&&) noexcept = default;

			// Declarations for `operator<<` overloads (definitions live in the .cxx)
			/**
			 * @brief Insert a boolean value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(bool v);

			/**
			 * @brief Insert a `char` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(char v);

			/**
			 * @brief Insert a `signed char` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(signed char v);

			/**
			 * @brief Insert an `unsigned char` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(unsigned char v);

			/**
			 * @brief Insert a `short` integer value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(short v);

			/**
			 * @brief Insert an `unsigned short` integer value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(unsigned short v);

			/**
			 * @brief Insert an `int` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(int v);

			/**
			 * @brief Insert an `unsigned int` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(unsigned int v);

			/**
			 * @brief Insert a `long` integer value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(long v);

			/**
			 * @brief Insert an `unsigned long` integer value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(unsigned long v);

			/**
			 * @brief Insert a `long long` integer value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(long long v);

			/**
			 * @brief Insert an `unsigned long long` integer value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(unsigned long long v);

			/**
			 * @brief Insert a `float` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(float v);

			/**
			 * @brief Insert a `double` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(double v);

			/**
			 * @brief Insert a `long double` value into the log stream.
			 * @param v Value to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(long double v);

			/**
			 * @brief Insert an `std::string` into the log stream.
			 * @param v String to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(const std::string& v);

			/**
			 * @brief Insert a C-string into the log stream.
			 * @param v C-string to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(const char* v);

			/**
			 * @brief Insert an `std::wstring` into the log stream (will be UTF-8 encoded).
			 * @param v Wide string to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(const std::wstring& v);

			/**
			 * @brief Insert a wide C-string into the log stream (will be UTF-8 encoded).
			 * @param v Wide C-string to insert.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(const wchar_t* v);

			// Level
			/**
			 * @brief Change the current logging level for subsequent messages.
			 * @param level Level to set for following messages. See
			 *        `StormByte::Logger::Level` for available values and ordering.
			 * @see StormByte::Logger::Level
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(const Level& level);

			// Stream manipulators (e.g., std::endl)
			/**
			 * @brief Apply a standard `std::ostream` manipulator (e.g., `std::endl`).
			 * @param manip Manipulator function to apply.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(std::ostream& (*manip)(std::ostream&));

			/**
			 * @brief Apply a `Log` manipulator that receives/returns `Log&`.
			 * @param manip Manipulator function to apply.
			 * @return Reference to this `Log`.
			 */
			Log& operator<<(Log& (*manip)(Log&) noexcept);

		private:
			std::shared_ptr<LogImpl> m_impl;
	};
}