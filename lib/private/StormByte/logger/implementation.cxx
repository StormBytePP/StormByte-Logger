#include <StormByte/logger/implementation.hxx>

#include <chrono>
#include <thread>

using namespace StormByte::Logger;

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

Implementation& Implementation::operator<<(const Level& level) noexcept {
	if (m_current_level) {
		if (level != *m_current_level && *m_current_level >= m_print_level && m_header_displayed) {
			m_out << std::endl;
			m_header_displayed = false;
		}
	}

	m_current_level = level;
	m_enabled.store(level >= m_print_level, std::memory_order_release);
	return *this;
}

Implementation& Implementation::operator<<(std::ostream& (*manip)(std::ostream&)) noexcept {
	if (m_enabled.load(std::memory_order_acquire)) {
		m_out << manip;
		m_header_displayed = false;
	}
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

void Implementation::print_header() const noexcept {
	const std::string& fmt = m_format;
	constexpr std::size_t fixed_width = 8;

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

void Implementation::print_message(const std::string& message) noexcept {
	if (!m_enabled.load(std::memory_order_acquire))
		return;
	write_text(message);
}

void Implementation::print_message(const wchar_t& value) noexcept {
	print_message(String::UTF8Encode(std::wstring(1, value)));
}

namespace StormByte::Logger {
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<bool>(const bool& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<short>(const short& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned short>(const unsigned short& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<int>(const int& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned int>(const unsigned int& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long>(const long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long>(const unsigned long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long long>(const long long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long long>(const unsigned long long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<float>(const float& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<double>(const double& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long double>(const long double& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<char>(const char& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned char>(const unsigned char& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<wchar_t>(const wchar_t& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::string>(const std::string& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::wstring>(const std::wstring& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const char*>(const char* const& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const wchar_t*>(const wchar_t* const& value) noexcept;
}
