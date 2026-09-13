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
#include <StormByte/exception.hxx>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <thread>
#include <utility>
using namespace StormByte::Logger;
namespace {
	thread_local std::string t_component;
	thread_local std::string t_group;
	thread_local std::optional<Level> t_level;
	struct LineState {
		bool decided = false;
		bool admitted = true;
		bool header_displayed = false;
		bool close_before_header = false;
		Level level = Level::Info;
		std::string component;
		std::string group;
		std::uint64_t dropped = 0;
	};
	thread_local LineState t_line;

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
	int PatternSpecificity(const ThrottleSpec& spec) noexcept {
		const bool component = spec.Component.has_value();
		const bool level = spec.Level.has_value();
		const bool group = spec.Group.has_value();
		if (component && level && group) return 7;
		if (component && group) return 6;
		if (component && level) return 5;
		if (component) return 4;
		if (level && group) return 3;
		if (group) return 2;
		if (level) return 1;
		return 0;
	}
	bool SameSelectors(const ThrottleSpec& left, const ThrottleSpec& right) noexcept {
		return left.Component == right.Component && left.Level == right.Level && left.Group == right.Group;
	}
	bool Matches(const ThrottleSpec& spec, const std::string& component, const Level level, const std::string& group) {
		return (!spec.Component || *spec.Component == component) &&
			(!spec.Level || *spec.Level == level) &&
			(!spec.Group || *spec.Group == group);
	}
	void ValidateThrottle(const ThrottleSpec& spec) {
		if (!std::isfinite(spec.Rate) || spec.Rate < 0.0 || (spec.Rate > 0.0 && spec.Burst == 0))
			throw StormByte::Exception("Invalid throttle rate or burst");
		if (spec.Policy == ThrottlePolicy::Sample && spec.SampleN < 2)
			throw StormByte::Exception("Invalid throttle sample period");
		if (spec.Policy == ThrottlePolicy::Window &&
			(spec.WindowPeriod == 0 || spec.WindowKeep == 0 || spec.WindowKeep > spec.WindowPeriod))
			throw StormByte::Exception("Invalid throttle window");
	}
	std::int64_t NowNanoseconds() noexcept {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}
	bool ConsumeFiniteCredit(ThrottleRuleState& state) noexcept {
		auto credits = state.finite_credits.load(std::memory_order_relaxed);
		while (credits != 0 && !state.finite_credits.compare_exchange_weak(
			credits, credits - 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {}
		return credits != 0;
	}
	bool ConsumeRateCredit(ThrottleRuleState& state, const ThrottleSpec& spec) noexcept {
		if (spec.Rate == 0.0 && spec.Burst == 0)
			return true;
		if (spec.Rate == 0.0)
			return ConsumeFiniteCredit(state);
		const auto interval = std::max<std::int64_t>(1,	static_cast<std::int64_t>(1'000'000'000.0 / spec.Rate));
		const auto now = NowNanoseconds();
		const auto capacity = static_cast<std::int64_t>(spec.Burst - 1) * interval;
		auto next = state.next_token_ns.load(std::memory_order_relaxed);
		for (;;) {
			const auto minimum = now - capacity;
			const auto reservation = std::max(next, minimum);
			if (reservation > now)
				return false;
			if (state.next_token_ns.compare_exchange_weak(
				next, reservation + interval, std::memory_order_acq_rel, std::memory_order_relaxed))
				return true;
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
	m_format(format),
	m_human_readable_format(String::Format::Raw),
	m_redact_active(false),
	m_redact_count(0),
	m_redact_keep_first(false),
	m_throttle_table(std::make_shared<const ThrottleTable>()) {
}
Implementation::~Implementation() noexcept {
	reset_color();
}
const Level& Implementation::CurrentLevel() const noexcept {
	return t_level ? *t_level : m_print_level;
}
bool Implementation::Enabled() const noexcept {
	if (!t_level)
		return m_enabled.load(std::memory_order_acquire);
	return IsAlwaysVisible(*t_level) || *t_level >= m_print_level;
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
const std::string& Implementation::effective_format() const noexcept {
	if (!m_format_stack.empty())
		return m_format_stack.back();
	const auto& component = t_line.decided ? t_line.component : t_component;
	if (const auto found = m_component_formats.find(component); found != m_component_formats.end())
		return found->second;
	return m_format;
}
const std::string& Implementation::Format() const noexcept {
	return effective_format();
}
const std::string& Implementation::Format(const std::string& component) const noexcept {
	if (const auto found = m_component_formats.find(component); found != m_component_formats.end())
		return found->second;
	return m_format;
}
void Implementation::Format(const std::string& format) {
	if (t_line.header_displayed) {
		reset_color();
		m_out << std::endl;
		t_line.header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		t_group.clear();
	}
	m_format = format;
}
void Implementation::Format(const std::string& component, const std::string& format) {
	if (component.empty()) {
		Format(format);
		return;
	}
	if (t_line.header_displayed) {
		reset_color();
		m_out << std::endl;
		t_line.header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		t_group.clear();
	}
	if (format.empty())
		m_component_formats.erase(component);
	else
		m_component_formats[component] = format;
}
void Implementation::Throttle(const ThrottleSpec& spec) {
	ValidateThrottle(spec);
	auto current = std::atomic_load_explicit(&m_throttle_table, std::memory_order_acquire);
	auto next = std::make_shared<ThrottleTable>(*current);
	const auto state = std::make_shared<ThrottleRuleState>();
	state->finite_credits.store(spec.Burst, std::memory_order_relaxed);
	const ThrottleRule rule{spec, state};
	auto found = std::find_if(next->rules.begin(), next->rules.end(), [&](const ThrottleRule& candidate) {
		return SameSelectors(candidate.spec, spec);
	});
	if (found == next->rules.end())
		next->rules.push_back(rule);
	else
		*found = rule;
	std::atomic_store_explicit(&m_throttle_table, std::shared_ptr<const ThrottleTable>(std::move(next)), std::memory_order_release);
}
void Implementation::NoThrottle(const ThrottleSpec& spec) {
	auto current = std::atomic_load_explicit(&m_throttle_table, std::memory_order_acquire);
	auto next = std::make_shared<ThrottleTable>(*current);
	next->rules.erase(std::remove_if(next->rules.begin(), next->rules.end(), [&](const ThrottleRule& candidate) {
		return SameSelectors(candidate.spec, spec);
	}), next->rules.end());
	std::atomic_store_explicit(&m_throttle_table, std::shared_ptr<const ThrottleTable>(std::move(next)), std::memory_order_release);
}
void Implementation::NoThrottleAll() noexcept {
	std::atomic_store_explicit(&m_throttle_table, std::make_shared<const ThrottleTable>(), std::memory_order_release);
}
bool Implementation::PrepareLine() {
	if (t_line.decided)
		return t_line.admitted;
	t_line.decided = true;
	t_line.level = t_level.value_or(m_print_level);
	t_line.component = t_component;
	t_line.group = t_group;
	t_line.admitted = true;
	t_line.dropped = 0;
	if (t_line.level == Level::Error || t_line.level == Level::Fatal)
		return true;
	const auto table = std::atomic_load_explicit(&m_throttle_table, std::memory_order_acquire);
	const ThrottleRule* selected = nullptr;
	int selected_specificity = -1;
	for (const auto& rule : table->rules) {
		if (Matches(rule.spec, t_line.component, t_line.level, t_line.group)) {
			const int specificity = PatternSpecificity(rule.spec);
			if (specificity > selected_specificity) {
				selected = &rule;
				selected_specificity = specificity;
			}
		}
	}
	if (selected == nullptr)
		return true;
	auto& state = *selected->state;
	bool admitted = true;
	if (selected->spec.Policy == ThrottlePolicy::Sample) {
		const auto index = state.sample_count.fetch_add(1, std::memory_order_relaxed);
		admitted = index % selected->spec.SampleN == 0;
	} else if (selected->spec.Policy == ThrottlePolicy::Window) {
		const auto index = state.window_count.fetch_add(1, std::memory_order_relaxed);
		admitted = index % selected->spec.WindowPeriod < selected->spec.WindowKeep;
	}
	if (admitted)
		admitted = ConsumeRateCredit(state, selected->spec);
	if (!admitted) {
		state.dropped.fetch_add(1, std::memory_order_relaxed);
		t_line.admitted = false;
		return false;
	}
	t_line.dropped = state.dropped.exchange(0, std::memory_order_acq_rel);
	return true;
}
bool Implementation::LineAdmitted() const noexcept {
	return t_line.admitted;
}
bool Implementation::HasOpenOutputLine() const noexcept {
	return t_line.header_displayed;
}
void Implementation::BeginOutputLine() noexcept {
	if (t_line.close_before_header) {
		reset_color();
		m_out.put('\n');
		t_line.close_before_header = false;
		t_line.header_displayed = false;
	}
	t_line.header_displayed = true;
}
void Implementation::reset_line_state() noexcept {
	t_line = {};
}
void Implementation::write_drop_summary() noexcept {
	if (t_line.dropped == 0)
		return;
	print_header();
	sync_content_color();
	m_out << "dropped " << t_line.dropped << " messages";
	reset_color();
	m_out.put('\n');
	t_line.dropped = 0;
}
Implementation& Implementation::operator<<(const Level& level) noexcept {
	if (t_level) {
		if (level != *t_level && (IsAlwaysVisible(*t_level) || *t_level >= m_print_level) && t_line.header_displayed) {
			reset_color();
			m_out << std::endl;
			t_line.close_before_header = true;
			t_group.clear();
			reset_line_state();
		}
		else if (t_line.decided)
			reset_line_state();
	}
	t_level = level;
	m_content_color.reset();
	m_content_nocolor = false;
	m_enabled.store(IsAlwaysVisible(level) || level >= m_print_level, std::memory_order_release);
	return *this;
}
Implementation& Implementation::operator<<(std::ostream& (*manip)(std::ostream&)) noexcept {
	if (ManipulatorWritesNewline(manip)) {
		if (Enabled() && PrepareLine()) {
			write_drop_summary();
			reset_color();
			if (t_line.admitted)
				m_out << manip;
		}
		t_line.header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		t_group.clear();
		reset_line_state();
		return *this;
	}
	if (Enabled())
		m_out << manip;
	return *this;
}
Implementation& Implementation::operator<<(ColorManip manip) noexcept {
	if (!Enabled())
		return *this;
	m_content_nocolor = false;
	m_content_color = manip.value;
	if (t_line.header_displayed)
		sync_content_color();
	return *this;
}
Implementation& Implementation::operator<<(NoColorManip) noexcept {
	if (!Enabled())
		return *this;
	m_content_color.reset();
	m_content_nocolor = true;
	if (t_line.header_displayed)
		sync_content_color();
	return *this;
}
Implementation& Implementation::operator<<(FormatManip manip) {
	if (t_line.header_displayed) {
		reset_color();
		m_out << std::endl;
		t_line.header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		t_group.clear();
		reset_line_state();
	}
	m_format_stack.push_back(std::move(manip.format));
	return *this;
}
Implementation& Implementation::operator<<(PopFormatManip) noexcept {
	if (m_format_stack.empty())
		return *this;
	if (t_line.header_displayed) {
		reset_color();
		m_out << std::endl;
		t_line.header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		t_group.clear();
		reset_line_state();
	}
	m_format_stack.pop_back();
	return *this;
}
Implementation& Implementation::operator<<(GroupManip manip) {
	if (t_line.header_displayed) {
		reset_color();
		m_out << std::endl;
		t_line.header_displayed = false;
		m_content_color.reset();
		m_content_nocolor = false;
		reset_line_state();
	}
	t_group = std::move(manip.name);
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
	const std::string level_str = LevelToString(t_line.decided ? t_line.level : t_level.value_or(m_print_level));
	m_out << level_str;
	for (std::size_t i = level_str.size(); i < fixed_width; ++i)
		m_out.put(' ');
}
void Implementation::print_thread_id() const noexcept {
	m_out << std::this_thread::get_id();
}
void Implementation::print_header() noexcept {
	const std::string& fmt = effective_format();
	constexpr std::size_t fixed_width = 8;
	const auto& component = t_line.decided ? t_line.component : t_component;
	emit_color(Color(component, t_line.level));
	for (std::size_t i = 0; i < fmt.size(); ++i) {
		if (fmt[i] == '%' && (i + 1) < fmt.size()) {
			const char spec = fmt[i + 1];
			switch (spec) {
				case '%':
					m_out.put('%');
					++i;
					break;
				case 'L': {
					const Level lvl = t_line.decided ? t_line.level : t_level.value_or(m_print_level);
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
					m_out << (t_line.decided ? t_line.group : t_group);
					++i;
					break;
				case 'c':
					m_out << component;
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
	const auto level = t_line.decided ? t_line.level : t_level.value_or(m_print_level);
	const auto& component = t_line.decided ? t_line.component : t_component;
	const auto configured = Color(component, level);
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
