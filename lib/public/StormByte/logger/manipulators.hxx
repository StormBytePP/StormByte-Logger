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

#include <StormByte/logger/typedefs.hxx>
#include <StormByte/logger/visibility.h>

#include <cstddef>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Logger
 * @brief Logger module of the StormByte suite.
 */
namespace StormByte::Logger {
	class Log;

	/**
	 * @struct GroupManip
	 * @brief Labels the current logging line with a producer group.
	 */
	struct STORMBYTE_LOGGER_PUBLIC GroupManip {
		std::string name; ///< Group name; an empty name clears the current group.
	};

	/**
	 * @brief Set the producer group for the current line.
	 * @param name Group name, or an empty string to clear the group.
	 * @return Group manipulator carrying the requested name.
	 */
	STORMBYTE_LOGGER_PUBLIC GroupManip group(std::string name);

	/**
	 * @struct ComponentManip
	 * @brief Selects the sticky component for the current thread.
	 *
	 * The component is thread-local to the calling thread, not tied to a
	 * particular Log instance. Two Log instances used by one thread therefore
	 * observe the same component; this is intentional for shared logger use.
	 */
	struct STORMBYTE_LOGGER_PUBLIC ComponentManip {
		std::string name; ///< Component name; empty selects the root component.
	};

	/**
	 * @brief Select the component associated with subsequent log lines on this thread.
	 * @param name Component name; empty selects the root component.
	 * @return Component manipulator carrying the requested name.
	 * @note An empty component is allowed for compatibility, but @ref reset_component
	 *       is preferred when returning to the root component explicitly.
	 */
	STORMBYTE_LOGGER_PUBLIC ComponentManip component(std::string name);

	/**
	 * @struct ResetComponentManip
	 * @brief Clears the component associated with the current thread.
	 *
	 * This is the canonical way to return to the root component.
	 */
	struct STORMBYTE_LOGGER_PUBLIC ResetComponentManip {};

	/**
	 * @brief Clear the current thread's component.
	 * @note The reset is thread-local and does not affect other threads.
	 */
	inline constexpr ResetComponentManip reset_component{};

	/**
	 * @struct FormatManip
	 * @brief Temporarily replaces the logger format and saves the previous one.
	 */
	struct STORMBYTE_LOGGER_PUBLIC FormatManip {
		std::string format; ///< Temporary format, including an empty format if requested.
	};

	/**
	 * @struct PopFormatManip
	 * @brief Restores the most recently saved logger format.
	 */
	struct STORMBYTE_LOGGER_PUBLIC PopFormatManip {};

	/**
	 * @brief Save the current format and activate a temporary format.
	 * @param format Format to activate until pop_format is streamed.
	 * @return Format manipulator containing the requested format.
	 */
	STORMBYTE_LOGGER_PUBLIC FormatManip push_format(std::string format);

	/**
	 * @brief Restore the most recently saved format, or do nothing if empty.
	 */
	inline constexpr PopFormatManip pop_format{};

	/**
	 * @struct ColorManip
	 * @brief Temporarily selects a configured or explicit content color.
	 */
	struct STORMBYTE_LOGGER_PUBLIC ColorManip {
		std::optional<Color> value; ///< Explicit color, or empty for the configured level color.

		/**
		 * @brief Select an explicit color for subsequent content.
		 * @param selected Color to use until another color manipulator or endl.
		 * @return A color manipulator carrying the selected color.
		 */
		constexpr ColorManip operator()(Color selected) const noexcept {
			return ColorManip{selected};
		}
	};

	/**
	 * @brief Restore the configured color for the current level.
	 */
	inline constexpr ColorManip color{};

	/**
	 * @struct NoColorManip
	 * @brief Disables color for subsequent content until changed.
	 */
	struct STORMBYTE_LOGGER_PUBLIC NoColorManip {};

	/**
	 * @brief Disable color for subsequent content in the current line.
	 */
	inline constexpr NoColorManip nocolor{};

	/**
	 * @brief Stateful redaction manipulator.
	 *
	 * - count == 0: every character becomes '*'.
	 * - keep_first == false (default): last `count` characters stay readable.
	 * - keep_first == true: first `count` characters stay readable.
	 *
	 * Applies to both text and numbers (numbers are converted to string first).
	 * Remains active until @ref no_redact.
	 *
	 * Usage:
	 * @code
	 * log << redact << secret << std::endl;              // full mask
	 * log << redact(4) << token << std::endl;            // keep last 4
	 * log << redact_first(4) << token << std::endl;      // keep first 4
	 * log << no_redact << plain << std::endl;
	 * @endcode
	 */
	struct STORMBYTE_LOGGER_PUBLIC RedactManip {
		std::size_t count = 0;     ///< 0 = mask all; N = keep N characters
		bool keep_first = false;   ///< true = keep first N, false = keep last N characters

		/**
		 * @brief Build a manipulator that keeps the last @p n characters visible.
		 * @param n Number of trailing characters to keep unmasked.
		 * @return A new RedactManip configured for keep-last.
		 */
		constexpr RedactManip operator()(std::size_t n) const noexcept {
			return RedactManip{ n, false };
		}
	};

	/**
	 * @brief Full redaction manipulator (mask everything).
	 * @see RedactManip
	 */
	inline constexpr RedactManip redact{};

	/**
	 * @brief Build a manipulator that keeps the first @p n characters visible.
	 * @param n Number of leading characters to keep unmasked.
	 * @return A RedactManip configured for keep-first.
	 */
	constexpr RedactManip redact_first(std::size_t n) noexcept {
		return RedactManip{ n, true };
	}

	/**
	 * @brief Enable human-readable formatting for numeric values.
	 * @param log The Log instance to modify.
	 * @return Reference to the same Log.
	 */
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept;

	/**
	 * @brief Enable human-readable formatting for byte counts.
	 * @param log The Log instance to modify.
	 * @return Reference to the same Log.
	 */
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept;

	/**
	 * @brief Disable human-readable formatting (raw numbers).
	 * @param log The Log instance to modify.
	 * @return Reference to the same Log.
	 */
	STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept;

	/**
	 * @brief Disable redaction until the next redact / redact(n) / redact_first(n).
	 * @param log The Log instance to modify.
	 * @return Reference to the same Log.
	 */
	STORMBYTE_LOGGER_PUBLIC Log& no_redact(Log& log) noexcept;
}
