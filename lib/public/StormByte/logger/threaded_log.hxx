#pragma once

#include <StormByte/logger/log.hxx>
#include <StormByte/thread_lock.hxx>

#include <memory>

/**
 * @namespace StormByte::Logger
 * @brief Logging module for StormByte library
 *
 * This namespace contains the logging types and utilities used throughout the StormByte project.
 */
namespace StormByte::Logger {
	/**
	 * @class ThreadedLog
	 * @brief Thread-safe logging facade.
	 *
	 * `ThreadedLog` extends the basic `Log` facade to provide thread-safe
	 * logging from multiple threads. It ensures that log messages from different
	 * threads do not interleave, preserving message integrity.
	 * The first thread writing to the log acquires a lock, blocking other threads
	 * until the logical write sequence is complete (indicated by `std::endl` manipulator).
	 */
	class STORMBYTE_LOGGER_PUBLIC ThreadedLog : public Log {
		public:
			/**
			 * @brief Construct a `ThreadedLog` writing to `out`.
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
			ThreadedLog(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");
			ThreadedLog(const ThreadedLog&) = default;
			ThreadedLog(ThreadedLog&&) noexcept = default;
			~ThreadedLog() noexcept = default;
			ThreadedLog& operator=(const ThreadedLog&) = default;
			ThreadedLog& operator=(ThreadedLog&&) noexcept = default;

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

		private:
			std::shared_ptr<ThreadLock> m_lock;

			/**
			 * @name Virtual write entry points
			 * These methods are the out-of-line implementations invoked by the
			 * inline `operator<<` wrappers above. They are intentionally private so
			 * that subclasses (if enabled) may override behaviour while keeping the
			 * public streaming API unchanged.
			 */
			//@{
			void Write(bool v) override;
			void Write(char v) override;
			void Write(signed char v) override;
			void Write(unsigned char v) override;		
			void Write(short v) override;
			void Write(unsigned short v) override;
			void Write(int v) override;
			void Write(unsigned int v) override;
			void Write(long v) override;
			void Write(unsigned long v) override;
			void Write(long long v) override;
			void Write(unsigned long long v) override;
			void Write(float v) override;
			void Write(double v) override;
			void Write(long double v) override;
			void Write(const std::string& v) override;
			void Write(const char* v) override;
			void Write(const std::wstring& v) override;
			void Write(const wchar_t* v) override;
			void Write(const Level& level) override;
			void Write(std::ostream& (*manip)(std::ostream&)) override;
			void Write(Log& (*manip)(Log&) noexcept) override;
			//@}
	};
}