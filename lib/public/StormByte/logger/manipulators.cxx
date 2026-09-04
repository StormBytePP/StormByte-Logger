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

#include <StormByte/logger/log.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <StormByte/logger/implementation.hxx>
namespace StormByte::Logger {
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept {
		humanreadable_number(*log.m_impl);
		return log;
	}
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept {
		humanreadable_bytes(*log.m_impl);
		return log;
	}
	STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept {
		nohumanreadable(*log.m_impl);
		return log;
	}
	STORMBYTE_LOGGER_PUBLIC Log& no_redact(Log& log) noexcept {
		log.m_impl->SetRedact(false, 0, false);
		return log;
	}
}
