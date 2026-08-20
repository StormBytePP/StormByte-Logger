/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/logger/visibility.h>

#include <string>

/**
 * @namespace StormByte::Logger
 * @brief Logging module for StormByte library
 *
 * This namespace contains the logging types and utilities used throughout the StormByte project.
 */
namespace StormByte::Logger {
	/**
	 * @enum Level
	 * @brief Severity levels used by the logger.
	 *
	 * Levels are ordered from least to most severe. Use these values to
	 * indicate the importance of log messages and to filter output.
	 */
	enum class STORMBYTE_LOGGER_PRIVATE Level : unsigned short {
		LowLevel = 0,                            	///< Very low-level (verbose) diagnostic messages
		Debug,                                    	///< Debugging information
		Warning,                                  	///< Potentially problematic situations
		Notice,                                   	///< Normal but significant events
		Info,                                     	///< General informational messages
		Error,                                    	///< Error conditions
		Fatal                                     	///< Fatal errors (usually unrecoverable)
	};

	/**
	 * @brief Convert a `Level` value into a human-readable name.
	 *
	 * @param l The logging level to convert.
	 * @return A short, human-readable string representing `l` (e.g. "Info").
	 */
	constexpr static std::string LevelToString(const Level& l) noexcept {
		switch (l) {
			case Level::LowLevel:    return "LowLevel";
			case Level::Debug:       return "Debug";
			case Level::Warning:     return "Warning";
			case Level::Notice:      return "Notice";
			case Level::Info:        return "Info";
			case Level::Fatal:       return "Fatal";
			case Level::Error:
			default:                  return "Error";
		}
	}
}