#include <chrono>
#include <iomanip>
#include <ostream>
#include <thread>
#include <sstream>

#include <StormByte/logger/implementation.hxx>

using namespace StormByte::Logger;

// Helper: return current time formatted as "dd/mm/YYYY HH:MM:SS".
std::string Implementation::CurrentTime() const noexcept {
	try {
		auto now = std::chrono::system_clock::now();
		std::time_t rawtime = std::chrono::system_clock::to_time_t(now);
		struct tm timeinfo;

#ifdef LINUX
		timeinfo = *std::localtime(&rawtime);
#else
		localtime_s(&timeinfo, &rawtime);
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
	m_header_displayed(false),
	m_format(format),
	m_human_readable_format(String::Format::Raw) {
}

Implementation& Implementation::operator<<(const Level& level) noexcept {
	if (m_current_level) {
		if (level != m_current_level && m_current_level >= m_print_level && m_header_displayed) {
			m_out << std::endl;
			m_header_displayed = false;
		}
	}

	m_current_level = level;
	return *this;
}

Implementation& Implementation::operator<<(std::ostream& (*manip)(std::ostream&)) noexcept {
	// Always apply the manipulator; release the current line lock afterwards
	if (m_current_level) {
		if (m_current_level >= m_print_level) {
			m_out << manip;
			m_header_displayed = false;
		}
	}
	// end of logical write sequence; no per-thread lock handling here

	return *this;
}

void Implementation::print_time() const noexcept {
	m_out << CurrentTime();
}

void Implementation::print_level() const noexcept {
	constexpr std::size_t fixed_width = 8; // Set a fixed width for all level strings
	const std::string level_str = LevelToString(*m_current_level);
	// Avoid allocating a temporary padding string: write the level and then put spaces.
	m_out << level_str;
	for (std::size_t i = level_str.size(); i < fixed_width; ++i) {
		m_out.put(' ');
	}
}

void Implementation::print_thread_id() const noexcept {
	m_out << std::this_thread::get_id();
}

void Implementation::print_header() const noexcept {
	// Build the header into a single string using std::format for
	// efficient composition and then write it once to the output stream.
	const std::string& fmt = m_format;
	std::string buf;
	buf.reserve(fmt.size() + 64);

	constexpr std::size_t fixed_width = 8;

	// Simple sequential replacements: handle escaped "%%" first, then
	// replace the small set of tokens we support. This is clearer and
	// avoids regex overhead for only a few replaces.
	std::string working = fmt;

	// Use a temporary placeholder for literal '%%' so replacements don't
	// accidentally touch them. Choose an unlikely character (unit separator).
	constexpr char ESC = '\x1F';
	// replace all "%%" -> ESC
	for (std::size_t p = 0; (p = working.find("%%", p)) != std::string::npos; p += 1) {
		working.replace(p, 2, 1, ESC);
	}

	// Helper to replace all occurrences of 'from' with 'to'
	auto replace_all = [](std::string& s, const std::string& from, const std::string& to) {
		if (from.empty()) return;
		for (std::size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos;) {
			s.replace(pos, from.size(), to);
			pos += to.size();
		}
	};

	// %L -> padded level
	{
		const Level& lvl = CurrentLevel();
		std::string level_str = LevelToString(lvl);
		if (level_str.size() < fixed_width) level_str.append(fixed_width - level_str.size(), ' ');
		replace_all(working, "%L", level_str);
	}

	// %T -> current time
	replace_all(working, "%T", CurrentTime());

	// %i -> thread id
	{
		std::ostringstream oss;
		oss << std::this_thread::get_id();
		replace_all(working, "%i", oss.str());
	}

	// restore escaped percent signs
	for (std::size_t p = 0; (p = working.find(ESC, p)) != std::string::npos; p += 1) {
		working.replace(p, 1, "%");
	}

	// done: assign to buffer and write
	buf = std::move(working);

	buf.push_back(' ');
	m_out.write(buf.data(), buf.size());
}

void Implementation::print_message(const std::string& message) noexcept {
	if (m_current_level.value_or(m_print_level) >= m_print_level) {
		if (!m_header_displayed) {
			print_header();
			m_header_displayed = true;
		}
		m_out << message;
	}
}

void Implementation::print_message(const wchar_t& value) noexcept {
	print_message(String::UTF8Encode(std::wstring(1, value)));
}

namespace StormByte::Logger {
	// Explicit instantiation for normalized (decayed) types
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<bool>(const bool& value) noexcept;

	// Numeric types
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<short>(const short& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned short>(const unsigned short& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<int>(const int& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned int>(const unsigned int& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long>(const long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long>(const unsigned long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long long>(const long long& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned long long>(const unsigned long long& value) noexcept;

	// Floating-point types
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<float>(const float& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<double>(const double& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<long double>(const long double& value) noexcept;

	// Character types
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<char>(const char& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<unsigned char>(const unsigned char& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<wchar_t>(const wchar_t& value) noexcept;

	// String types
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::string>(const std::string& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<std::wstring>(const std::wstring& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const char*>(const char* const& value) noexcept;
	template STORMBYTE_LOGGER_PUBLIC Implementation& Implementation::operator<<<const wchar_t*>(const wchar_t* const& value) noexcept;
}

// No synchronization helpers — logging is not responsible for cross-thread locking