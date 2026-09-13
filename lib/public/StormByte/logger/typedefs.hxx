/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Logger.
 *
 * StormByte-Logger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
 *
 * StormByte-Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte-Logger. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#pragma once

#include <StormByte/logger/visibility.h>

#include <string>

/**
 * @namespace StormByte::Logger
 * @brief Logger module of the StormByte suite.
 */
namespace StormByte::Logger {
	/**
	 * @enum Color
	 * @brief ANSI foreground colors supported by the logger.
	 *
	 * @c Default emits no ANSI sequence and leaves the terminal color unchanged.
	 */
	enum class STORMBYTE_LOGGER_PUBLIC Color : unsigned char {
		Default,        ///< No ANSI sequence; preserve the terminal's current color.
		Black,          ///< Standard black foreground.
		Red,            ///< Standard red foreground.
		Green,          ///< Standard green foreground.
		Yellow,         ///< Standard yellow foreground.
		Blue,           ///< Standard blue foreground.
		Magenta,        ///< Standard magenta foreground.
		Cyan,           ///< Standard cyan foreground.
		Gray,           ///< Bright black/gray foreground.
		White,          ///< Standard white foreground.
		BrightBlack,    ///< Bright black foreground.
		BrightRed,      ///< Bright red foreground.
		BrightGreen,    ///< Bright green foreground.
		BrightYellow,   ///< Bright yellow foreground.
		BrightBlue,     ///< Bright blue foreground.
		BrightMagenta,  ///< Bright magenta foreground.
		BrightCyan,     ///< Bright cyan foreground.
		BrightWhite     ///< Bright white foreground.
	};

	/**
	 * @enum Level
	 * @brief Severity levels used by the logger.
	 *
	 * Ordered from least to most severe. Used both as the print floor
	 * and as the level of the current message. Warning, Error and Fatal
	 * are always emitted regardless of the configured print floor.
	 */
	enum class STORMBYTE_LOGGER_PUBLIC Level : unsigned short {
		LowLevel = 0,   ///< Verbose diagnostics
		Debug,          ///< Debug information
		Warning,        ///< Recoverable problems
		Notice,         ///< Significant normal events
		Info,           ///< Informational messages
		Error,          ///< Error conditions
		Fatal           ///< Unrecoverable errors
	};

	/**
	 * @brief Convert a Level to a short name.
	 * @param l Level to convert.
	 * @return Name such as "Info" or "Error".
	 */
	constexpr static std::string LevelToString(const Level& l) noexcept {
		switch (l) {
			case Level::LowLevel:	return "LowLevel";
			case Level::Debug:		return "Debug";
			case Level::Warning:	return "Warning";
			case Level::Notice:		return "Notice";
			case Level::Info:		return "Info";
			case Level::Error:		return "Error";
			case Level::Fatal:		return "Fatal";
			default:				return "Error";
		}
	}
}
