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
#include <utility>

using namespace StormByte::Logger;

namespace {
	StormByte::Logger::ThrottleSpec make_spec(const double rate, const std::size_t burst,
		const StormByte::Logger::ThrottlePolicy policy = StormByte::Logger::ThrottlePolicy::Drop,
		const std::size_t value = 0, const std::size_t period = 0) {
		StormByte::Logger::ThrottleSpec spec;
		spec.Rate = rate;
		spec.Burst = burst;
		spec.Policy = policy;
		if (policy == StormByte::Logger::ThrottlePolicy::Sample)
			spec.SampleN = value;
		if (policy == StormByte::Logger::ThrottlePolicy::Window) {
			spec.WindowKeep = value;
			spec.WindowPeriod = period;
		}

		return spec;
	}

	bool AlwaysVisible(const Level level) noexcept {
		return level == Level::Warning || level == Level::Error || level == Level::Fatal;
	}
}

Log::Log(std::ostream& out, const Level& level, const std::string& format) {
	m_impl = std::make_shared<Implementation>(out, level, format);
}

bool Log::Enabled(const Level& level) const noexcept {
	return AlwaysVisible(level) || level >= m_impl->PrintLevel();
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
void Log::Write(std::string_view v) { m_impl << v; }
void Log::Write(const char* v) { m_impl << v; }
void Log::Write(std::wstring_view v) { m_impl << v; }
void Log::Write(const wchar_t* v) { m_impl << v; }
void Log::Write(const Level& level) { m_impl << level; }
void Log::Write(std::ostream& (*manip)(std::ostream&)) { m_impl << manip; }
void Log::Write(Log& (*manip)(Log&) noexcept) { manip(*this); }
void Log::Write(RedactManip m) {
	m_impl->SetRedact(true, m.count, m.keep_first);
}

Log& Log::Color(const Level& level, const StormByte::Logger::Color& color) {
	m_impl->Color(level, color);
	return *this;
}

StormByte::Logger::Color Log::Color(const Level& level) const {
	return m_impl->Color(level);
}

Log& Log::Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color) {
	m_impl->Color(component, level, color);
	return *this;
}

StormByte::Logger::Color Log::Color(const std::string& component, const Level& level) const {
	return m_impl->Color(component, level);
}

Log& Log::Format(const std::string& format) {
	m_impl->Format(format);
	return *this;
}

const std::string& Log::Format() const {
	return m_impl->Format();
}

Log& Log::Format(const std::string& component, const std::string& format) {
	m_impl->Format(component, format);
	return *this;
}

const std::string& Log::Format(const std::string& component) const {
	return static_cast<const Implementation&>(*m_impl).Format(component);
}

Log& Log::Throttle(const ThrottleSpec& spec) {
	m_impl->Throttle(spec);
	return *this;
}

Log& Log::NoThrottle(const ThrottleSpec& spec) {
	m_impl->NoThrottle(spec);
	return *this;
}

Log& Log::Throttle(double rate, std::size_t burst) {
	return Throttle(make_spec(rate, burst));
}

Log& Log::Throttle(double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period) {
	return Throttle(make_spec(rate, burst, policy, value, period));
}

Log& Log::Throttle(const Level& level, double rate, std::size_t burst) {
	auto spec = make_spec(rate, burst);
	spec.Level = level;
	return Throttle(spec);
}

Log& Log::Throttle(GroupManip group, double rate, std::size_t burst) {
	auto spec = make_spec(rate, burst);
	spec.Group = std::move(group.name);
	return Throttle(spec);
}

Log& Log::Throttle(ComponentManip component, double rate, std::size_t burst) {
	auto spec = make_spec(rate, burst);
	spec.Component = std::move(component.name);
	return Throttle(spec);
}

Log& Log::Throttle(ComponentManip component, const Level& level, GroupManip group, double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period) {
	auto spec = make_spec(rate, burst, policy, value, period);
	spec.Component = std::move(component.name);
	spec.Level = level;
	spec.Group = std::move(group.name);
	return Throttle(spec);
}

Log& Log::NoThrottle() {
	m_impl->NoThrottleAll();
	return *this;
}

Log& Log::NoThrottle(const Level& level) {
	ThrottleSpec spec;
	spec.Level = level;
	return NoThrottle(spec);
}

Log& Log::NoThrottle(GroupManip group) {
	ThrottleSpec spec;
	spec.Group = std::move(group.name);
	return NoThrottle(spec);
}

Log& Log::NoThrottle(ComponentManip component) {
	ThrottleSpec spec;
	spec.Component = std::move(component.name);
	return NoThrottle(spec);
}

Log& Log::NoThrottle(ComponentManip component, const Level& level, GroupManip group) {
	ThrottleSpec spec;
	spec.Component = std::move(component.name);
	spec.Level = level;
	spec.Group = std::move(group.name);
	return NoThrottle(spec);
}

Log& Log::FlushThrottle() {
	m_impl->FlushThrottle();
	return *this;
}

Log& Log::FlushThrottle(const ThrottleSpec& spec) {
	m_impl->FlushThrottle(spec);
	return *this;
}

void Log::Write(ColorManip manip) { *m_impl << manip; }
void Log::Write(NoColorManip manip) { *m_impl << manip; }
void Log::Write(FormatManip manip) { *m_impl << std::move(manip); }
void Log::Write(PopFormatManip manip) { *m_impl << manip; }
void Log::Write(GroupManip manip) { *m_impl << std::move(manip); }
void Log::Write(ComponentManip manip) { *m_impl << std::move(manip); }
void Log::Write(ResetComponentManip manip) { *m_impl << manip; }
bool Log::WillWrite() const noexcept {
	return m_impl->Enabled();
}

bool Log::PrepareLine() {
	return m_impl->PrepareLine();
}

bool Log::HasOpenOutputLine() const noexcept {
	return m_impl->HasOpenOutputLine();
}

bool Log::LineDecided() const noexcept {
	return m_impl->LineDecided();
}

bool Log::LineAdmitted() const noexcept {
	return m_impl->LineAdmitted();
}
