#include <StormByte/logger/threaded_log.hxx>

using namespace StormByte::Logger;

ThreadedLog::ThreadedLog(std::ostream& out, const Level& level, const std::string& format):
	Log(out, level, format), m_lock(std::make_shared<ThreadLock>()) {}

// Basic values: acquire thread lock before forwarding to base implementation.
void ThreadedLog::Write(bool v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(char v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(signed char v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(unsigned char v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(short v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(unsigned short v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(int v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(unsigned int v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(long v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(unsigned long v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(long long v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(unsigned long long v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(float v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(double v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(long double v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(const std::string& v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(const char* v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(const std::wstring& v) { m_lock->Lock(); Log::Write(v); }
void ThreadedLog::Write(const wchar_t* v) { m_lock->Lock(); Log::Write(v); }

// Level change doesn't end a logical line — just forward while holding lock.
void ThreadedLog::Write(const Level& level) { m_lock->Lock(); Log::Write(level); }

// Stream manipulators: apply manipulator and release lock if it is std::endl.
void ThreadedLog::Write(std::ostream& (*manip)(std::ostream&)) {
	m_lock->Lock();
	Log::Write(manip);
	if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
		m_lock->Unlock();
	}
}

// Log manipulators (forwarders that accept Log&). These do not implicitly end
// the logical line — the caller should use std::endl to finalize.
void ThreadedLog::Write(Log& (*manip)(Log&) noexcept) {
	m_lock->Lock();
	Log::Write(manip);
}