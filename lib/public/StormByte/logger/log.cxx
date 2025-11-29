#include <StormByte/logger/log.hxx>
#include <StormByte/logger/log_impl.hxx>

#include <iostream>
#include <thread>
#include <chrono>

using namespace StormByte::Logger;

Log::Log(std::ostream& out, const Level& level, const std::string& format) {
	m_impl = std::make_shared<LogImpl>(out, level, format);
}

// Basic types — implemented as virtual Write methods
Log& Log::Write(bool v) { m_impl << v; return *this; }
Log& Log::Write(char v) { m_impl << v; return *this; }
Log& Log::Write(signed char v) { m_impl << v; return *this; }
Log& Log::Write(unsigned char v) { m_impl << v; return *this; }
Log& Log::Write(short v) { m_impl << v; return *this; }
Log& Log::Write(unsigned short v) { m_impl << v; return *this; }
Log& Log::Write(int v) { m_impl << v; return *this; }
Log& Log::Write(unsigned int v) { m_impl << v; return *this; }
Log& Log::Write(long v) { m_impl << v; return *this; }
Log& Log::Write(unsigned long v) { m_impl << v; return *this; }
Log& Log::Write(long long v) { m_impl << v; return *this; }
Log& Log::Write(unsigned long long v) { m_impl << v; return *this; }
Log& Log::Write(float v) { m_impl << v; return *this; }
Log& Log::Write(double v) { m_impl << v; return *this; }
Log& Log::Write(long double v) { m_impl << v; return *this; }
Log& Log::Write(const std::string& v) { m_impl << v; return *this; }
Log& Log::Write(const char* v) { m_impl << v; return *this; }
Log& Log::Write(const std::wstring& v) { m_impl << v; return *this; }
Log& Log::Write(const wchar_t* v) { m_impl << v; return *this; }

// Level
Log& Log::Write(const Level& level) { m_impl << level; return *this; }

// Stream manipulators
Log& Log::Write(std::ostream& (*manip)(std::ostream&)) { m_impl << manip; return *this; }

// Log manipulators (forwarders that accept Log&)
Log& Log::Write(Log& (*manip)(Log&) noexcept) { return manip(*this); }
