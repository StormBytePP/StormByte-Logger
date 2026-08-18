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
	 * @brief Stateful redaction manipulator (same idea as human-readable flags).
	 *
	 * - keep_last == 0: every character becomes '*'.
	 * - keep_last == N: last N characters stay readable; the rest become '*'.
	 *
	 * Remains active until @ref no_redact.
	 *
	 * Usage:
	 * @code
	 * log << redact << secret << std::endl;     // full mask
	 * log << redact(4) << token << std::endl; // keep last 4
	 * log << no_redact << plain << std::endl;
	 * @endcode
	 */
	struct STORMBYTE_LOGGER_PUBLIC RedactManip {
		std::size_t keep_last = 0;	///< 0 = mask all; N = keep last N chars

		/**
		 * @brief Build a manipulator that keeps the last @p n characters visible.
		 */
		constexpr RedactManip operator()(std::size_t n) const noexcept {
			return RedactManip{ n };
		}
	};

	/**
	 * @brief Full redaction manipulator (`keep_last == 0`).
	 * @see RedactManip
	 */
	inline constexpr RedactManip redact{};

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
	 * @brief Disable redaction until the next redact / redact(n).
	 * @param log The Log instance to modify.
	 * @return Reference to the same Log.
	 */
	STORMBYTE_LOGGER_PUBLIC Log& no_redact(Log& log) noexcept;
}
