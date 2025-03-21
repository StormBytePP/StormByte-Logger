#include <StormByte/logger/log.hxx>

#include <ostream>
#include <chrono>
#include <iomanip>

using namespace StormByte::Logger;

Log::Log(std::ostream& out, const Level& level, const std::string& format) noexcept:m_out(out),
m_print_level(level), m_current_level(level), m_line_started(false), m_format(format) {}

Log& Log::operator<<(const Level& level) noexcept {
	if (m_current_level >= m_print_level && m_line_started)
		m_out << std::endl;

	m_current_level = level;
	m_line_started = false;
	return *this;
}

Log& Log::operator<<(const std::string& str) noexcept {
	print_message(str);
	return *this;
}

Log& Log::operator<<(const char* str) noexcept {
	print_message(str);
	return *this;
}

Log& Log::operator<<(const int& value) noexcept {
	print_message(std::to_string(value));
	return *this;
}

Log& Log::operator<<(const double& value) noexcept {
	print_message(std::to_string(value));
	return *this;
}

Log& Log::operator<<(const bool& value) noexcept {
	print_message(value ? "true" : "false");
	return *this;
}

void Log::print_time() const noexcept {
	auto now = std::chrono::system_clock::now();
    std::time_t rawtime = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo;
	#ifdef LINUX
    timeinfo = *std::localtime(&rawtime);
	#else
    localtime_s(&timeinfo, &rawtime);
	#endif
    m_out << std::put_time(&timeinfo, "%d/%m/%Y %H:%M:%S");
}

void Log::print_level() const noexcept {
	m_out << LevelToString(m_current_level);
}

void Log::print_header() const noexcept {
	for (char c : m_format) {
        if (c == '%') {
            continue;
        }
        switch (c) {
            case 'L':
                print_level();
                break;
            case 'T':
                print_time();
                break;
            default:
                m_out << c;
                break;
        }
    }
}

void Log::print_message(const std::string& message) noexcept {
	if (m_current_level >= m_print_level) {
		if (!m_line_started) {
			print_header();
			m_line_started = true;
		}
		m_out << message;
	}
}

namespace StormByte::Logger {
	// Forward operator<< for Level
	std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const Level& level) noexcept {
		if (logger) {
			*logger << level; // Forward the call to Log
		}
		return logger;
	}

	// Forward operator<< for std::string
	std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const std::string& str) noexcept {
		if (logger) {
			*logger << str; // Forward the call to Log
		}
		return logger;
	}

	// Forward operator<< for const char*
	std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const char* str) noexcept {
		if (logger) {
			*logger << str; // Forward the call to Log
		}
		return logger;
	}

	// Forward operator<< for int
	std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const int& value) noexcept {
		if (logger) {
			*logger << value; // Forward the call to Log
		}
		return logger;
	}

	// Forward operator<< for double
	std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const double& value) noexcept {
		if (logger) {
			*logger << value; // Forward the call to Log
		}
		return logger;
	}

	// Forward operator<< for bool
	std::shared_ptr<Log>& operator<<(std::shared_ptr<Log>& logger, const bool& value) noexcept {
		if (logger) {
			*logger << value; // Forward the call to Log
		}
		return logger;
	}
}