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
#include <StormByte/logger/manipulators.hxx>
#include <chrono>
#include <thread>
#include <utility>
using namespace StormByte::Logger;
namespace {
	thread_local std::string t_component;

	bool IsAlwaysVisible(const Level level) noexcept {
		return level == Level::Warning || level == Level::Error || level == Level::Fatal;
	}
	std::size_t ColorIndex(const Level level) noexcept {
		return static_cast<std::size_t>(level);
	}
	const char* AnsiColor(const StormByte::Logger::Color color) noexcept {
		switch (color) {
			case StormByte::Logger::Color::Black: return "\033[30m";
			case StormByte::Logger::Color::Red: return "\033[31m";
			case StormByte::Logger::Color::Green: return "\033[32m";
			case StormByte::Logger::Color::Yellow: return "\033[33m";
			case StormByte::Logger::Color::Blue: return "\033[34m";
			case StormByte::Logger::Color::Magenta: return "\033[35m";
			case StormByte::Logger::Color::Cyan: return "\033[36m";
			case StormByte::Logger::Color::Gray: return "\033[90m";
			case StormByte::Logger::Color::White: return "\033[37m";
			case StormByte::Logger::Color::BrightBlack: return "\033[90m";
			case StormByte::Logger::Color::BrightRed: return "\033[91m";
			case StormByte::Logger::Color::BrightGreen: return "\033[92m";
			case StormByte::Logger::Color::BrightYellow: return "\033[93m";
			case StormByte::Logger::Color::BrightBlue: return "\033[94m";
			case StormByte::Logger::Color::BrightMagenta: return "\033[95m";
			case StormByte::Logger::Color::BrightCyan: return "\033[96m";
			case StormByte::Logger::Color::BrightWhite: return "\033[97m";
			case StormByte::Logger::Color::Default: return "";
		}
		return "";
	}
	bool ManipulatorWritesNewline(std::ostream& (*manip)(std::ostream&)) {
		try {
			std::ostringstream probe;
			manip(probe);
			return probe.str().find('\n') != std::string::npos;
		} catch (...) {
			return false;
		}
	}
}
std::string Implementation::CurrentTime() const noexcept {
	try {
		auto now = std::chrono::system_clock::now();
		std::time_t rawtime = std::chrono::system_clock::to_time_t(now);
		struct tm timeinfo{};
#ifdef WINDOWS
		localtime_s(&timeinfo, &rawtime);
#elifdef UNIX
		localtime_r(&rawtime, &timeinfo);
#else
		#error "Unsupported platform for CurrentTime()"
#endif
		char timebuf[64];
		std::size_t tn = std::strftime(timebuf, sizeof(timebuf), "%d/%m/%Y %H:%M:%S", &timeinfo);
		return std::string(timebuf, tn);
	} catch (...) {
		return std::string();
	}
}
Implementation::Implementation(std::ostream& out, const Level& level, const std::string& format):
	m_out(out),
	m_print_level(level),
	m_current_level(std::nullopt),
	m_enabled(true),
	m_header_displayed(false),
	m_format(format),
	m_human_readable_format(String::Format::Raw),
	m_redact_active(false),
	m_redact_count(0),
	m_redact_keep_first(false) {
}
Implementation::~Implementation() noexcept {
	reset_color();
}
void Implementation::Color(const Level& level, const StormByte::Logger::Color& color) noexcept {
	if (ColorIndex(level) < m_level_colors.size())
		m_level_colors[ColorIndex(level)] = color;
}
void Implementation::Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color) {
	if (!component.empty())
		m_component_colors[component][ColorIndex(level)] = color;
}
StormByte::Logger::Color Implementation::Color(const Level& level) const noexcept {
	if (ColorIndex(level) < m_level_colors.size())
		return m_level_colors[ColorIndex(level)];
	return StormByte::Logger::Color::Default;
}
StormByte::Logger::Color Implementation::Color(const std::string& component, const Level& level) const noexcept {
	if (const auto found = m_component_colors.find(component); found != m_component_colors.end())
		return found->second[ColorIndex(level)];
	return Color(level);
}
Implementation& Implementation::operator<<(const Level& level) noexcept {
	if (m_current_level) {
		if (level != *m_current_level && (IsAlwaysVisible(*m_current_level) || *m_current_level >= m_print_level) && m_header_displayed) {
			reset_color();
			m_out << std::endl;
			m_header_displayed = false;
			m_group.clear();
		}
	}
	m_current_level = level;
	m_content_color.reset();
	m_content_nocolor = false;
	m_enabled.store(IsAlwaysVisible(level) || level >= m_print_level, std::memory_order_release);
	return *this;
}
Implementation& Implementation::operator<<(std::ostream& (*manip)(std::ostream&)) noexcept {
	if (ManipulatorWritesNewline(manip)) {
		if (m_enabled.load(std::memory_order_acquire)) {
			reset_color();
			m_out << manip;
		}
		m_header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		m_group.clear();
		return *this;
	}
	if (m_enabled.load(std::memory_order_acquire))
		m_out << manip;
	return *this;
}
Implementation& Implementation::operator<<(ColorManip manip) noexcept {
	if (!m_enabled.load(std::memory_order_acquire))
		return *this;
	m_content_nocolor = false;
	m_content_color = manip.value;
	if (m_header_displayed)
		sync_content_color();
	return *this;
}
Implementation& Implementation::operator<<(NoColorManip) noexcept {
	if (!m_enabled.load(std::memory_order_acquire))
		return *this;
	m_content_color.reset();
	m_content_nocolor = true;
	if (m_header_displayed)
		sync_content_color();
	return *this;
}
Implementation& Implementation::operator<<(FormatManip manip) {
	if (m_header_displayed) {
		reset_color();
		m_out << std::endl;
		m_header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		m_group.clear();
	}
	m_format_stack.push_back(m_format);
	m_format = std::move(manip.format);
	return *this;
}
Implementation& Implementation::operator<<(PopFormatManip) noexcept {
	if (m_format_stack.empty())
		return *this;
	if (m_header_displayed) {
		reset_color();
		m_out << std::endl;
		m_header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		m_group.clear();
	}
	m_format = std::move(m_format_stack.back());
	m_format_stack.pop_back();
	return *this;
}
Implementation& Implementation::operator<<(GroupManip manip) {
	if (m_header_displayed) {
		reset_color();
		m_out << std::endl;
		m_header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
	}
	m_group = std::move(manip.name);
	return *this;
}
Implementation& Implementation::operator<<(ComponentManip manip) {
	t_component = std::move(manip.name);
	return *this;
}
Implementation& Implementation::operator<<(ResetComponentManip) {
	t_component.clear();
	return *this;
}
void Implementation::print_time() const noexcept {
	m_out << CurrentTime();
}
void Implementation::print_level() const noexcept {
	constexpr std::size_t fixed_width = 8;
	const std::string level_str = LevelToString(*m_current_level);
	m_out << level_str;
	for (std::size_t i = level_str.size(); i < fixed_width; ++i)
		m_out.put(' ');
}
void Implementation::print_thread_id() const noexcept {
	m_out << std::this_thread::get_id();
}
void Implementation::print_header() noexcept {
	const std::string& fmt = m_format;
	constexpr std::size_t fixed_width = 8;
	emit_color(Color(t_component, *m_current_level));
	for (std::size_t i = 0; i < fmt.size(); ++i) {
		if (fmt[i] == '%' && (i + 1) < fmt.size()) {
			const char spec = fmt[i + 1];
			switch (spec) {
				case '%':
					m_out.put('%');
					++i;
					break;
				case 'L': {
					const Level lvl = m_current_level ? *m_current_level : m_print_level;
					std::string level_str = LevelToString(lvl);
					m_out << level_str;
					for (std::size_t p = level_str.size(); p < fixed_width; ++p)
						m_out.put(' ');
					++i;
					break;
				}
				case 'T':
					print_time();
					++i;
					break;
				case 'i':
					print_thread_id();
					++i;
					break;
				case 'g':
					m_out << m_group;
					++i;
					break;
				case 'c':
					m_out << t_component;
					++i;
					break;
				default:
					m_out.put('%');
					break;
			}
		} else {
			m_out.put(fmt[i]);
		}
	}
	m_out.put(' ');
}
void Implementation::sync_content_color() noexcept {
	const auto level = m_current_level.value_or(m_print_level);
	const auto configured = Color(t_component, level);
	if (m_content_nocolor)
		emit_color(StormByte::Logger::Color::Default);
	else if (m_content_color)
		emit_color(*m_content_color);
	else
		emit_color(configured);
}
void Implementation::emit_color(const StormByte::Logger::Color color) noexcept {
	if (m_active_color == std::optional<StormByte::Logger::Color>{color})
		return;
	if (m_active_color) {
		m_out << "\033[0m";
		m_active_color.reset();
	}
	if (color != StormByte::Logger::Color::Default) {
		m_out << AnsiColor(color);
		m_active_color = color;
	}
}
void Implementation::reset_color() noexcept {
	if (m_active_color) {
		m_out << "\033[0m";
		m_active_color.reset();
	}
}
void Implementation::print_message(const std::string& message) noexcept {
	if (!m_enabled.load(std::memory_order_acquire))
		return;
	write_text(message);
}
void Implementation::print_message(const wchar_t& value) {
	print_message(String::UTF8Encode(std::wstring(1, value)));
}
namespace StormByte::Logger {
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<bool>(const bool& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<short>(const short& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned short>(const unsigned short& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<int>(const int& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned int>(const unsigned int& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long>(const long& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long>(const unsigned long& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long long>(const long long& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long long>(const unsigned long long& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<float>(const float& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<double>(const double& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long double>(const long double& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<char>(const char& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned char>(const unsigned char& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<wchar_t>(const wchar_t& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::string>(const std::string& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::wstring>(const std::wstring& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const char*>(const char* const& value);
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const wchar_t*>(const wchar_t* const& value);
}
