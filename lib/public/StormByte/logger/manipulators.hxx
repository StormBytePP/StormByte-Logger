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

#include <cstddef>

/**
 * @namespace StormByte::Logger
 * @brief Logging module for StormByte library.
 */
namespace StormByte::Logger {
	class Log;

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
		std::size_t count = 0;		///< 0 = mask all; N = keep N characters
		bool keep_first = false;	///< true = keep first N, false = keep last N

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
