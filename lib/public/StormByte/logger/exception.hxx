/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Logger.
 *
 * StormByte-Logger original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-Logger source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte-String tree and
 * the StormByte Base tree it vendors), which remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-Logger. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
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
