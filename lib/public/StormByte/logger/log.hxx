#pragma once

#include <StormByte/logger/level.hxx>

#include <memory>
#include <ostream>

/**
 * @namespace Logger
 * @brief All the classes for handling logging
 */
namespace StormByte::Logger {
	/**
	 * @class Log
	 * @brief Log class
	 */
	class STORMBYTE_LOGGER_PUBLIC Log {
		public:
			/**
			 * Constructor
			 * @param out output stream
			 * @param level log level
			 * @param format custom format
			 */
			Log(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T") noexcept;

			/**
			 * Copy constructor
			 */
			Log(const Log&)										= delete;

			/**
			 * Move constructor
			 */
			Log(Log&&) noexcept									= default;

			/**
			 * Assignment operator
			 */
			Log& operator=(const Log&)							= delete;

			/**
			 * Move operator
			 */
			Log& operator=(Log&&) noexcept						= default;

			/**
			 * Destructor
			 */
			virtual ~Log() noexcept								= default;

			/**
			 * Sets the log level
			 * @param level log level
			 */
			Log& operator<<(const Level& level) noexcept;

			/**
			 * Logs a string
			 * @param str string
			 */
			Log& operator<<(const std::string& str) noexcept;

			/**
			 * Logs a const char*
			 * @param str string
			 */
			Log& operator<<(const char* str) noexcept;

			/**
			 * Logs an integer
			 * @param value integer
			 */
			Log& operator<<(const int& value) noexcept;

			/**
			 * Logs a double
			 * @param value double
			 */
			Log& operator<<(const double& value) noexcept;

			/**
			 * Logs a boolean
			 * @param value boolean
			 */
			Log& operator<<(const bool& value) noexcept;

		protected:
			/**
			 * Prints the time
			 */
			void print_time() const noexcept;

			/**
			 * Prints the log level
			 */
			void print_level() const noexcept;

			/**
			 * Prints the header
			 */
			void print_header() const noexcept;

			/** Prints the message
			 * @param message message
			 */
			void print_message(const std::string& message) noexcept;

			/**
			 * Output stream
			 */
			std::ostream& m_out;

			/**
			 * Print level and current level
			 */
			Level m_print_level;								///< Print level
			Level m_current_level;								///< Current level

			/**
			 * Line started
			 */
			bool m_line_started;								///< Line started
			
			/**
			 * Custom user Format
			 */
			const std::string m_format;							///< Custom user format %L for Level and %T for Time
	};

	STORMBYTE_LOGGER_PUBLIC std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const Level& level) noexcept;
	STORMBYTE_LOGGER_PUBLIC std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const std::string& str) noexcept;
	STORMBYTE_LOGGER_PUBLIC std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const char* str) noexcept;
	STORMBYTE_LOGGER_PUBLIC std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const int& value) noexcept;
	STORMBYTE_LOGGER_PUBLIC std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const double& value) noexcept;
	STORMBYTE_LOGGER_PUBLIC std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const bool& value) noexcept;
}