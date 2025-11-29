#pragma once

#include <StormByte/logger/visibility.h>

/**
 * @namespace StormByte::Logger
 * @brief Logging module for StormByte library
 *
 * This namespace contains the logging types and utilities used throughout the StormByte project.
 */
namespace StormByte::Logger {
	class Log;
	/**
	 * @brief Enable human-readable formatting for numeric values on the provided `Log`.
	 *
	 * Numeric values streamed after this manipulator will be formatted using
	 * a human-friendly representation (e.g. group separators for large numbers).
	 *
	 * @param log The `Log` instance to modify.
	 * @return Reference to the same `Log` (allows chaining).
	 */
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept;
	
	/**
	 * @brief Enable human-readable formatting for byte counts on the provided `Log`.
	 *
	 * Numeric values streamed after this manipulator will be formatted as
	 * human-readable byte sizes (for example: "1.46 MiB").
	 *
	 * @param log The `Log` instance to modify.
	 * @return Reference to the same `Log` (allows chaining).
	 */
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept;
	
	/**
	 * @brief Disable human-readable formatting and print raw numeric values.
	 *
	 * Reverts formatting previously enabled by `humanreadable_number` or
	 * `humanreadable_bytes` so values are printed using their raw numeric
	 * representation.
	 *
	 * @param log The `Log` instance to modify.
	 * @return Reference to the same `Log` (allows chaining).
	 */
	STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept;
}