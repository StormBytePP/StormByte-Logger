#include <StormByte/logger/log.hxx>
#include <StormByte/logger/log_impl.hxx>

#include <iostream>
#include <thread>
#include <chrono>

namespace StormByte::Logger {

Log::Log(std::ostream& out, const Level& level, const std::string& format) {
	m_impl = std::make_shared<LogImpl>(out, level, format);
}

// Basic types
Log& Log::operator<<(bool v) { m_impl << v; return *this; }
Log& Log::operator<<(char v) { m_impl << v; return *this; }
Log& Log::operator<<(signed char v) { m_impl << v; return *this; }
Log& Log::operator<<(unsigned char v) { m_impl << v; return *this; }
Log& Log::operator<<(short v) { m_impl << v; return *this; }
Log& Log::operator<<(unsigned short v) { m_impl << v; return *this; }
Log& Log::operator<<(int v) { m_impl << v; return *this; }
Log& Log::operator<<(unsigned int v) { m_impl << v; return *this; }
Log& Log::operator<<(long v) { m_impl << v; return *this; }
Log& Log::operator<<(unsigned long v) { m_impl << v; return *this; }
Log& Log::operator<<(long long v) { m_impl << v; return *this; }
Log& Log::operator<<(unsigned long long v) { m_impl << v; return *this; }
Log& Log::operator<<(float v) { m_impl << v; return *this; }
Log& Log::operator<<(double v) { m_impl << v; return *this; }
Log& Log::operator<<(long double v) { m_impl << v; return *this; }
Log& Log::operator<<(const std::string& v) { m_impl << v; return *this; }
Log& Log::operator<<(const char* v) { m_impl << v; return *this; }
Log& Log::operator<<(const std::wstring& v) { m_impl << v; return *this; }
Log& Log::operator<<(const wchar_t* v) { m_impl << v; return *this; }

// Level
Log& Log::operator<<(const Level& level) { m_impl << level; return *this; }

// Stream manipulators
Log& Log::operator<<(std::ostream& (*manip)(std::ostream&)) { m_impl << manip; return *this; }

// Log manipulators (forwarders that accept Log&)
Log& Log::operator<<(Log& (*manip)(Log&) noexcept) { return manip(*this); }

}
