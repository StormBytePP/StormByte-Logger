#include <StormByte/logger/log.hxx>
#include <StormByte/logger/implementation.hxx>

#include <chrono>
#include <iostream>
#include <thread>

using namespace StormByte::Logger;

Log::Log(std::ostream& out, const Level& level, const std::string& format) {
	m_impl = std::make_shared<Implementation>(out, level, format);
}

// Basic types — implemented as virtual Write methods
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
void Log::Write(const std::string& v) { m_impl << v; }
void Log::Write(const char* v) { m_impl << v; }
void Log::Write(const std::wstring& v) { m_impl << v; }
void Log::Write(const wchar_t* v) { m_impl << v; }

// Level
void Log::Write(const Level& level) { m_impl << level; }

// Stream manipulators
void Log::Write(std::ostream& (*manip)(std::ostream&)) { m_impl << manip; }

// Log manipulators (forwarders that accept Log&)
void Log::Write(Log& (*manip)(Log&) noexcept) { manip(*this); }

bool Log::WillWrite() const noexcept {
	return m_impl->CurrentLevel() >= m_impl->PrintLevel();
}