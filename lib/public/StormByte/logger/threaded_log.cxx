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

#include <StormByte/logger/threaded_log.hxx>
#include <sstream>
using namespace StormByte::Logger;
namespace {
	thread_local bool t_line_held = false;
	void claim_line(const std::shared_ptr<StormByte::ThreadLock>& lock) {
		if (!t_line_held) {
			lock->Lock();
			t_line_held = true;
		}
	}
	void release_line(const std::shared_ptr<StormByte::ThreadLock>& lock) {
		if (t_line_held) {
			lock->Unlock();
			t_line_held = false;
		}
	}
	bool manipulator_writes_newline(std::ostream& (*manip)(std::ostream&)) {
		try {
			std::ostringstream probe;
			manip(probe);
			const auto& s = probe.str();
			return !s.empty() && s.find('\n') != std::string::npos;
		} catch (...) {
			return false;
		}
	}
}
ThreadedLog::ThreadedLog(std::ostream& out, const Level& level, const std::string& format):
	Log(out, level, format), m_lock(std::make_shared<ThreadLock>()) {}
void ThreadedLog::Write(bool v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(char v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(signed char v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned char v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(short v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned short v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(int v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned int v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(long long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned long long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(float v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(double v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(long double v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const std::string& v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const char* v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const std::wstring& v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const wchar_t* v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const Level& level) {
	claim_line(m_lock);
	Log::Write(level);
	if (!WillWrite())
		release_line(m_lock);
}
void ThreadedLog::Write(std::ostream& (*manip)(std::ostream&)) {
	if (WillWrite()) {
		claim_line(m_lock);
		Log::Write(manip);
		if (manipulator_writes_newline(manip))
			release_line(m_lock);
	} else {
		Log::Write(manip);
	}
}
void ThreadedLog::Write(Log& (*manip)(Log&) noexcept) {
	claim_line(m_lock);
	Log::Write(manip);
	if (!WillWrite())
		release_line(m_lock);
}
void ThreadedLog::Write(RedactManip m) {
	// State change on Implementation; serialize like other manipulators.
	claim_line(m_lock);
	Log::Write(m);
	if (!WillWrite())
		release_line(m_lock);
}
