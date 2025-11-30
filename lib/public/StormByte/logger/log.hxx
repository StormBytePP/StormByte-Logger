#pragma once

#include <StormByte/logger/manipulators.hxx>
#include <StormByte/logger/typedefs.hxx>

#include <memory>
#include <string>
#include <ostream>

/**
 * @namespace StormByte::Logger
 * @brief Logging module for StormByte library
 *
 * This namespace contains the logging types and utilities used throughout the StormByte project.
 */
namespace StormByte::Logger {
	class Implementation; ///< Forward declaration of internal Implementation class
	/**
	 * @class Log
	 * @brief Public streaming facade for the StormByte logger.
	 *
	 * `Log` is the stable public API used by application code. It owns a
	 * `std::shared_ptr` to the internal implementation (`Implementation`) and exposes a
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
			inline Log& operator<<(bool v) { Write(v); return *this; }
			inline Log& operator<<(char v) { Write(v); return *this; }
			inline Log& operator<<(signed char v) { Write(v); return *this; }
			inline Log& operator<<(unsigned char v) { Write(v); return *this; }
			inline Log& operator<<(short v) { Write(v); return *this; }
			inline Log& operator<<(unsigned short v) { Write(v); return *this; }
			inline Log& operator<<(int v) { Write(v); return *this; }
			inline Log& operator<<(unsigned int v) { Write(v); return *this; }
			inline Log& operator<<(long v) { Write(v); return *this; }
			inline Log& operator<<(unsigned long v) { Write(v); return *this; }
			inline Log& operator<<(long long v) { Write(v); return *this; }
			inline Log& operator<<(unsigned long long v) { Write(v); return *this; }
			inline Log& operator<<(float v) { Write(v); return *this; }
			inline Log& operator<<(double v) { Write(v); return *this; }
			inline Log& operator<<(long double v) { Write(v); return *this; }
			inline Log& operator<<(const std::string& v) { Write(v); return *this; }
			inline Log& operator<<(const char* v) { Write(v); return *this; }
			inline Log& operator<<(const std::wstring& v) { Write(v); return *this; }
			inline Log& operator<<(const wchar_t* v) { Write(v); return *this; }
			inline Log& operator<<(const Level& level) { Write(level); return *this; }
			inline Log& operator<<(std::ostream& (*manip)(std::ostream&)) { Write(manip); return *this; }
			inline Log& operator<<(Log& (*manip)(Log&) noexcept) { Write(manip); return *this; }
			//@}

		protected:
			std::shared_ptr<Implementation> m_impl;

			/**
			 * @brief Determine if the logger will write messages at the current level.
			 * @return `true` if messages will be written, `false` otherwise.
			 */
			bool WillWrite() const noexcept;

			/**
			 * @name Virtual write entry points
			 * These methods are the out-of-line implementations invoked by the
			 * inline `operator<<` wrappers above. They are intentionally private so
			 * that subclasses (if enabled) may override behaviour while keeping the
			 * public streaming API unchanged.
			 */
			//@{
			virtual void Write(bool v);
			virtual void Write(char v);
			virtual void Write(signed char v);
			virtual void Write(unsigned char v);		
			virtual void Write(short v);
			virtual void Write(unsigned short v);
			virtual void Write(int v);
			virtual void Write(unsigned int v);
			virtual void Write(long v);
			virtual void Write(unsigned long v);
			virtual void Write(long long v);
			virtual void Write(unsigned long long v);
			virtual void Write(float v);
			virtual void Write(double v);
			virtual void Write(long double v);
			virtual void Write(const std::string& v);
			virtual void Write(const char* v);
			virtual void Write(const std::wstring& v);
			virtual void Write(const wchar_t* v);
			virtual void Write(const Level& level);
			virtual void Write(std::ostream& (*manip)(std::ostream&));
			virtual void Write(Log& (*manip)(Log&) noexcept);
			//@}
	};
}