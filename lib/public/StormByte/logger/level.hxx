#pragma once

#include <StormByte/logger/visibility.h>

#include <string>

/**
 * @namespace Logger
 * @brief All the classes for handling logging
 */
namespace StormByte::Logger {
	/**
	 * @enum Level
	 * @brief Log level
	 */
	enum class STORMBYTE_LOGGER_PUBLIC Level {
		Debug = 0,	///< Debug level
		Warning,	///< Warning level
		Notice,		///< Notice level
		Info,		///< Info level
		Error,		///< Error level
		Fatal		///< Fatal level
	};

	/**
	 * Gets Level string
	 * @return string
	 */
	constexpr STORMBYTE_LOGGER_PUBLIC std::string LevelToString(const Level& l) noexcept {
		switch(l) {
			case Level::Debug:		return "Debug";
			case Level::Warning:	return "Warning";
			case Level::Notice:		return "Notice";
			case Level::Info:		return "Info";
			case Level::Error:		return "Error";
			case Level::Fatal:		return "Fatal";
			default:				return "Unknown";
		}
	}
}