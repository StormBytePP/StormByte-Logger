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

#include <StormByte/exception.hxx>
#include <StormByte/logger/visibility.h>

#include <format>
#include <string>
#include <utility>

/**
 * @namespace StormByte::Logger
 * @brief Logger module of the StormByte suite.
 */
namespace StormByte::Logger {
	/**
	 * @class Exception
	 * @brief Root exception for Logger errors.
	 */
	class STORMBYTE_LOGGER_PUBLIC Exception: public StormByte::Exception {
		public:
			/** @brief Construct with an unformatted message. */
			explicit Exception(const std::string& message):
				StormByte::Exception(StormByte::Component{"Logger"}, "{}", message) {}

			/** @brief Construct with a moved message. */
			explicit Exception(std::string&& message):
				StormByte::Exception(StormByte::Component{"Logger"}, "{}", std::move(message)) {}

			/**
			 * @brief Construct with a formatted message.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(std::format_string<Args...> fmt, Args&&... args):
				StormByte::Exception(StormByte::Component{"Logger"}, fmt, std::forward<Args>(args)...) {}
	};

	/**
	 * @class ThrottleError
	 * @brief Thrown when a throttle rule is invalid.
	 */
	class STORMBYTE_LOGGER_PUBLIC ThrottleError: public Exception {
		public:
			using Exception::Exception;
	};
}
