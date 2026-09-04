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
#include <StormByte/logger/implementation.hxx>
using namespace StormByte::Logger;
Log::Log(std::ostream& out, const Level& level, const std::string& format) {
	m_impl = std::make_shared<Implementation>(out, level, format);
}
void Log::Write(bool v) { m_impl << v; }
void Log::Write(char v) { m_impl << v; }
void Log::Write(signed char v) { m_impl << v; }
void Log::Write(unsigned char v) { m_impl << v; }
void Log::Write(short v) { m_impl << v; }
void Log::Write(unsigned short v) { m_impl << v; }
void Log::Write(int v) { m_impl << v; }
void Log::Write(unsigned int v) { m_impl << v; }
void Log::Write(long v) { m_impl << v; }
void Log::Write(unsigned long v) { m_impl << v; }
void Log::Write(long long v) { m_impl << v; }
void Log::Write(unsigned long long v) { m_impl << v; }
void Log::Write(float v) { m_impl << v; }
void Log::Write(double v) { m_impl << v; }
void Log::Write(long double v) { m_impl << v; }
void Log::Write(const std::string& v) { m_impl << v; }
void Log::Write(const char* v) { m_impl << v; }
void Log::Write(const std::wstring& v) { m_impl << v; }
void Log::Write(const wchar_t* v) { m_impl << v; }
void Log::Write(const Level& level) { m_impl << level; }
void Log::Write(std::ostream& (*manip)(std::ostream&)) { m_impl << manip; }
void Log::Write(Log& (*manip)(Log&) noexcept) { manip(*this); }
void Log::Write(RedactManip m) {
    m_impl->SetRedact(true, m.count, m.keep_first);
}
bool Log::WillWrite() const noexcept {
	return m_impl->Enabled();
}
