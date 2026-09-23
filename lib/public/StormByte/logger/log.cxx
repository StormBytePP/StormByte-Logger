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

	std::string JoinPath(const std::string& base, std::string add) {
		if (add.empty())
			return base;
		if (base.empty())
			return add;
		return base + "/" + add;
	}

	void BindStickyComponent(ThrottleSpec& spec, const std::string& path) {
		if (!spec.Component && !path.empty())
			spec.Component = StormByte::String::String{std::string_view{path}};
	}
}

Log::Log(std::ostream& out, const Level& level, const std::string& format) {
	m_impl = std::make_shared<Implementation>(out, level, format);
}

Log::PointerType Log::Clone() const {
	return std::make_shared<Log>(*this);
}

Log::PointerType Log::Move() {
	return std::make_shared<Log>(*this);
}

Log::PointerType Log::Scope(std::string path) {
	auto facade = Clone();
	facade->m_scope_path = JoinPath(m_scope_path, std::move(path));
	return facade;
}

bool Log::Enabled(const Level& level) const noexcept {
	return AlwaysVisible(level) || level >= m_impl->PrintLevel();
}

void Log::Write(bool v) {
	m_impl << v;
}

void Log::Write(char v) {
	m_impl << v;
}

void Log::Write(signed char v) {
	m_impl << v;
}

void Log::Write(unsigned char v) {
	m_impl << v;
}

void Log::Write(short v) {
	m_impl << v;
}

void Log::Write(unsigned short v) {
	m_impl << v;
}

void Log::Write(int v) {
	m_impl << v;
}

void Log::Write(unsigned int v) {
	m_impl << v;
}

void Log::Write(long v) {
	m_impl << v;
}

void Log::Write(unsigned long v) {
	m_impl << v;
}

void Log::Write(long long v) {
	m_impl << v;
}

void Log::Write(unsigned long long v) {
	m_impl << v;
}

void Log::Write(float v) {
	m_impl << v;
}

void Log::Write(double v) {
	m_impl << v;
}

void Log::Write(long double v) {
	m_impl << v;
}

void Log::Write(std::string_view v) {
	m_impl << v;
}

void Log::Write(const char* v) {
	m_impl << v;
}

void Log::Write(std::wstring_view v) {
	m_impl << v;
}

void Log::Write(const wchar_t* v) {
	m_impl << v;
}

void Log::Write(std::span<const std::byte> v) {
	m_impl << v;
}

void Log::Write(const Level& level) {
	m_impl->SetFacadePath(m_scope_path);
	m_impl << level;
}

void Log::Write(std::ostream& (*manip)(std::ostream&)) {
	m_impl << manip;
}

void Log::Write(Log& (*manip)(Log&) noexcept) {
	manip(*this);
}

void Log::Write(RedactManip m) {
	m_impl->SetRedact(true, m.count, m.keep_first);
}

void Log::Write(HexManip m) {
	m_impl->SetHex(true, m.columns);
}

void Log::Write(NoHexManip) {
	m_impl->SetHex(false, 0);
}

Log& Log::Color(const Level& level, const StormByte::Logger::Color& color) {
	if (m_scope_path.empty())
		m_impl->Color(level, color);
	else
		m_impl->Color(m_scope_path, level, color);
	return *this;
}

StormByte::Logger::Color Log::Color(const Level& level) const {
	if (m_scope_path.empty())
		return m_impl->Color(level);
	return m_impl->Color(m_scope_path, level);
}

Log& Log::Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color) {
	m_impl->Color(component, level, color);
	return *this;
}

StormByte::Logger::Color Log::Color(const std::string& component, const Level& level) const {
	return m_impl->Color(component, level);
}

Log& Log::Format(const std::string& format) {
	if (m_scope_path.empty())
		m_impl->Format(format);
	else
		m_impl->Format(m_scope_path, format);
	return *this;
}

const std::string& Log::Format() const {
	if (m_scope_path.empty())
		return m_impl->Format();
	return static_cast<const Implementation&>(*m_impl).Format(m_scope_path);
}

Log& Log::Format(const std::string& component, const std::string& format) {
	m_impl->Format(component, format);
	return *this;
}

const std::string& Log::Format(const std::string& component) const {
	return static_cast<const Implementation&>(*m_impl).Format(component);
}

Log& Log::Throttle(const ThrottleSpec& spec) {
	ThrottleSpec bound = spec;
	BindStickyComponent(bound, m_scope_path);
	m_impl->Throttle(bound);
	return *this;
}

Log& Log::NoThrottle(const ThrottleSpec& spec) {
	ThrottleSpec bound = spec;
	BindStickyComponent(bound, m_scope_path);
	m_impl->NoThrottle(bound);
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

void Log::Write(ColorManip manip) {
	*m_impl << manip;
}

void Log::Write(NoColorManip manip) {
	*m_impl << manip;
}

void Log::Write(FormatManip manip) {
	*m_impl << std::move(manip);
}

void Log::Write(PopFormatManip manip) {
	*m_impl << manip;
}

void Log::Write(GroupManip manip) {
	*m_impl << std::move(manip);
}

void Log::Write(ComponentManip manip) {
	*m_impl << std::move(manip);
}

void Log::Write(PopComponentManip manip) {
	*m_impl << manip;
}

void Log::Write(ResetComponentManip manip) {
	*m_impl << manip;
}

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
