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
	 * @brief Public streaming facade for the StormByte logger.
	 *
	 * `Log` is the stable public API used by application code. It owns a
	 * `std::shared_ptr` to the internal implementation (`LogImpl`) and exposes a
	 * set of `operator<<` overloads that mimic `std::ostream` for convenient
	 * formatted logging. The facade performs formatting and forwards the result
	 * to the implementation which performs the actual emission.
	 *
	 */
	class STORMBYTE_LOGGER_PUBLIC Log {
		// Allow free manipulators to access `m_impl`
		friend STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept;

		public:
			/**
			 * @brief Construct a `Log` writing to `out`.
			 *
			 * @param out Output stream to Write log messages to (for example `std::cout`).
			 * @param level Minimum `Level` that will be emitted. Messages below this
			 *        level are suppressed.
			 * @see StormByte::Logger::Level
			 * @param format Format string used for log headers. Supported placeholders:
			 *        - `%L` : level name (e.g. "Info", "Error").
			 *        - `%T` : timestamp (formatted as `dd/mm/YYYY HH:MM:SS`).
			 * 	      - `%i` : thread ID.
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

			/**
			 * @name Streaming Operators
			 * These overloads mirror `std::ostream::operator<<` for common types and
			 * manipulators.
			 */
			//@{
			inline Log& operator<<(bool v) { return Write(v); }
			inline Log& operator<<(char v) { return Write(v); }
			inline Log& operator<<(signed char v) { return Write(v); }
			inline Log& operator<<(unsigned char v) { return Write(v); }
			inline Log& operator<<(short v) { return Write(v); }
			inline Log& operator<<(unsigned short v) { return Write(v); }
			inline Log& operator<<(int v) { return Write(v); }
			inline Log& operator<<(unsigned int v) { return Write(v); }
			inline Log& operator<<(long v) { return Write(v); }
			inline Log& operator<<(unsigned long v) { return Write(v); }
			inline Log& operator<<(long long v) { return Write(v); }
			inline Log& operator<<(unsigned long long v) { return Write(v); }
			inline Log& operator<<(float v) { return Write(v); }
			inline Log& operator<<(double v) { return Write(v); }
			inline Log& operator<<(long double v) { return Write(v); }
			inline Log& operator<<(const std::string& v) { return Write(v); }
			inline Log& operator<<(const char* v) { return Write(v); }
			inline Log& operator<<(const std::wstring& v) { return Write(v); }
			inline Log& operator<<(const wchar_t* v) { return Write(v); }
			inline Log& operator<<(const Level& level) { return Write(level); }
			inline Log& operator<<(std::ostream& (*manip)(std::ostream&)) { return Write(manip); }
			inline Log& operator<<(Log& (*manip)(Log&) noexcept) { return Write(manip); }
			//@}

		protected:
			std::shared_ptr<LogImpl> m_impl;

		private:
			/**
			 * @name Virtual write entry points
			 * These methods are the out-of-line implementations invoked by the
			 * inline `operator<<` wrappers above. They are intentionally private so
			 * that subclasses (if enabled) may override behaviour while keeping the
			 * public streaming API unchanged.
			 */
			//@{
			virtual Log& Write(bool v);
			virtual Log& Write(char v);
			virtual Log& Write(signed char v);
			virtual Log& Write(unsigned char v);		
			virtual Log& Write(short v);
			virtual Log& Write(unsigned short v);
			virtual Log& Write(int v);
			virtual Log& Write(unsigned int v);
			virtual Log& Write(long v);
			virtual Log& Write(unsigned long v);
			virtual Log& Write(long long v);
			virtual Log& Write(unsigned long long v);
			virtual Log& Write(float v);
			virtual Log& Write(double v);
			virtual Log& Write(long double v);
			virtual Log& Write(const std::string& v);
			virtual Log& Write(const char* v);
			virtual Log& Write(const std::wstring& v);
			virtual Log& Write(const wchar_t* v);
			virtual Log& Write(const Level& level);
			virtual Log& Write(std::ostream& (*manip)(std::ostream&));
			virtual Log& Write(Log& (*manip)(Log&) noexcept);
			//@}
	};
}