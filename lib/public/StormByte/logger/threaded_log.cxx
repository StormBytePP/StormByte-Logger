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

#include <StormByte/logger/implementation.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/string.hxx>
#include <sstream>
#include <utility>

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
		if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
			return true;
		if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::ends))
			return false;
		try {
			std::ostringstream probe;
			probe << manip;
			return probe.str().find('\n') != std::string::npos;
		} catch (...) {
			return false;
		}
	}
}

ThreadedLog::ThreadedLog(std::ostream& out, const Level& level, const std::string& format):
	Log(out, level, format), m_lock(std::make_shared<ThreadLock>()) {}

Log::PointerType ThreadedLog::Clone() const {
	return std::make_shared<ThreadedLog>(*this);
}

Log::PointerType ThreadedLog::Move() {
	return std::make_shared<ThreadedLog>(*this);
}

Log& ThreadedLog::Color(const Level& level, const StormByte::Logger::Color& color) {
	const bool already_held = t_line_held;
	claim_line(m_lock);
	Log::Color(level, color);
	if (!already_held)
		release_line(m_lock);
	return *this;
}

StormByte::Logger::Color ThreadedLog::Color(const Level& level) const {
	return Log::Color(level);
}

Log& ThreadedLog::Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color) {
	Log::Color(component, level, color);
	return *this;
}

StormByte::Logger::Color ThreadedLog::Color(const std::string& component, const Level& level) const {
	return Log::Color(component, level);
}

Log& ThreadedLog::Format(const std::string& format) {
	const bool already_held = t_line_held;
	claim_line(m_lock);
	try {
		Log::Format(format);
	} catch (...) {
		release_line(m_lock);
		throw;
	}

	if (!already_held)
		release_line(m_lock);
	return *this;
}

const std::string& ThreadedLog::Format() const {
	return Log::Format();
}

Log& ThreadedLog::Format(const std::string& component, const std::string& format) {
	const bool already_held = t_line_held;
	claim_line(m_lock);
	try {
		Log::Format(component, format);
	} catch (...) {
		release_line(m_lock);
		throw;
	}

	if (!already_held)
		release_line(m_lock);
	return *this;
}

const std::string& ThreadedLog::Format(const std::string& component) const {
	return Log::Format(component);
}

Log& ThreadedLog::Throttle(const ThrottleSpec& spec) { return Log::Throttle(spec); }
Log& ThreadedLog::NoThrottle(const ThrottleSpec& spec) { return Log::NoThrottle(spec); }
Log& ThreadedLog::Throttle(double rate, std::size_t burst) { return Log::Throttle(rate, burst); }
Log& ThreadedLog::Throttle(double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period) {
	return Log::Throttle(rate, burst, policy, value, period);
}
Log& ThreadedLog::Throttle(const Level& level, double rate, std::size_t burst) { return Log::Throttle(level, rate, burst); }
Log& ThreadedLog::Throttle(GroupManip group, double rate, std::size_t burst) { return Log::Throttle(group, rate, burst); }
Log& ThreadedLog::Throttle(ComponentManip component, double rate, std::size_t burst) { return Log::Throttle(component, rate, burst); }
Log& ThreadedLog::Throttle(ComponentManip component, const Level& level, GroupManip group, double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period) {
	return Log::Throttle(component, level, group, rate, burst, policy, value, period);
}
Log& ThreadedLog::NoThrottle() { return Log::NoThrottle(); }
Log& ThreadedLog::NoThrottle(const Level& level) { return Log::NoThrottle(level); }
Log& ThreadedLog::NoThrottle(GroupManip group) { return Log::NoThrottle(group); }
Log& ThreadedLog::NoThrottle(ComponentManip component) { return Log::NoThrottle(component); }
Log& ThreadedLog::NoThrottle(ComponentManip component, const Level& level, GroupManip group) {
	return Log::NoThrottle(component, level, group);
}

Log& ThreadedLog::FlushThrottle() {
	const bool already_held = t_line_held;
	claim_line(m_lock);
	Log::FlushThrottle();
	if (!already_held)
		release_line(m_lock);
	return *this;
}

Log& ThreadedLog::FlushThrottle(const ThrottleSpec& spec) {
	const bool already_held = t_line_held;
	claim_line(m_lock);
	Log::FlushThrottle(spec);
	if (!already_held)
		release_line(m_lock);
	return *this;
}

void ThreadedLog::Write(bool v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(char v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(signed char v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(unsigned char v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(short v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(unsigned short v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(int v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(unsigned int v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(long v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(unsigned long v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(long long v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(unsigned long long v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(float v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(double v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(long double v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(std::string_view v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(const char* v) {
	if (!WillWrite() || !PrepareLine()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(std::wstring_view v) {
	if (!WillWrite() || !PrepareLine()) return;
	const std::string encoded = StormByte::String::UTF8Encode(v);
	claim_line(m_lock);
	Log::Write(std::string_view{encoded});
}

void ThreadedLog::Write(const wchar_t* v) {
	if (!WillWrite() || !PrepareLine()) return;
	const std::string encoded = v ? StormByte::String::UTF8Encode(std::wstring_view{v}) : std::string{};
	claim_line(m_lock);
	Log::Write(std::string_view{encoded});
}

void ThreadedLog::Write(std::span<const std::byte> v) {
	if (!WillWrite() || !PrepareLine()) return;
	const std::string formatted = m_impl->FormatBinary(v);
	claim_line(m_lock);
	m_impl->WritePrepared(formatted);
}

void ThreadedLog::Write(const Level& level) {
	Log::Write(level);
	if (!WillWrite() && !HasOpenOutputLine())
		return;
	claim_line(m_lock);
}

void ThreadedLog::Write(std::ostream& (*manip)(std::ostream&)) {
	const bool newline = manipulator_writes_newline(manip);
	if (WillWrite() && PrepareLine()) {
		claim_line(m_lock);
		Log::Write(manip);
	} else {
		Log::Write(manip);
	}

	if (newline)
		release_line(m_lock);
}

void ThreadedLog::Write(Log& (*manip)(Log&) noexcept) {
	if ((LineDecided() && !LineAdmitted()) || !WillWrite()) {
		Log::Write(manip);
		return;
	}

	claim_line(m_lock);
	Log::Write(manip);
	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(RedactManip m) {
	if ((LineDecided() && !LineAdmitted()) || !WillWrite()) {
		Log::Write(m);
		return;
	}

	claim_line(m_lock);
	Log::Write(m);
	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(HexManip m) {
	if ((LineDecided() && !LineAdmitted()) || !WillWrite()) {
		Log::Write(m);
		return;
	}

	claim_line(m_lock);
	Log::Write(m);
	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(NoHexManip m) {
	if ((LineDecided() && !LineAdmitted()) || !WillWrite()) {
		Log::Write(m);
		return;
	}

	claim_line(m_lock);
	Log::Write(m);
	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(ColorManip m) {
	if ((LineDecided() && !LineAdmitted()) || !WillWrite()) {
		Log::Write(m);
		return;
	}

	claim_line(m_lock);
	Log::Write(m);
	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(NoColorManip m) {
	if ((LineDecided() && !LineAdmitted()) || !WillWrite()) {
		Log::Write(m);
		return;
	}

	claim_line(m_lock);
	Log::Write(m);
	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(FormatManip m) {
	if (LineDecided() && !LineAdmitted()) {
		Log::Write(std::move(m));
		return;
	}

	claim_line(m_lock);
	try {
		Log::Write(std::move(m));
	} catch (...) {
		release_line(m_lock);
		throw;
	}

	if (!WillWrite())
		release_line(m_lock);
}

void ThreadedLog::Write(PopFormatManip m) {
	const bool already_held = t_line_held;
	if (LineDecided() && !LineAdmitted()) {
		Log::Write(m);
		return;
	}

	claim_line(m_lock);
	Log::Write(m);
	if (!already_held)
		release_line(m_lock);
}

void ThreadedLog::Write(GroupManip m) {
	Log::Write(std::move(m));
}

void ThreadedLog::Write(ComponentManip m) {
	Log::Write(std::move(m));
}

void ThreadedLog::Write(PopComponentManip m) {
	Log::Write(m);
}

void ThreadedLog::Write(ResetComponentManip m) {
	Log::Write(m);
}
