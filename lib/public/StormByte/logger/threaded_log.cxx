#include <StormByte/logger/threaded_log.hxx>

using namespace StormByte::Logger;

#include <sstream>

namespace {
	// Per-thread line ownership for this process. Safe with a shared ThreadedLog
	// because only the thread that claimed the lock touches its own flag.
	thread_local bool t_line_held = false;

	void claim_line(const std::shared_ptr<StormByte::ThreadLock>& lock) {
		if (!t_line_held) {
			lock->Lock();
			t_line_held = true;
		}
	}

	void release_line(const std::shared_ptr<StormByte::ThreadLock>& lock) {
		if (t_line_held) {
			lock->Unlock();
			t_line_held = false;
		}
	}

	bool manipulator_writes_newline(std::ostream& (*manip)(std::ostream&)) {
		try {
			std::ostringstream probe;
			manip(probe);
			const auto& s = probe.str();
			return !s.empty() && s.find('\n') != std::string::npos;
		} catch (...) {
			return false;
		}
	}
}

ThreadedLog::ThreadedLog(std::ostream& out, const Level& level, const std::string& format):
	Log(out, level, format), m_lock(std::make_shared<ThreadLock>()) {}

// Data: pure early-out when filtered — no lock.
void ThreadedLog::Write(bool v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(char v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(signed char v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned char v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(short v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned short v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(int v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned int v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(long long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(unsigned long long v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(float v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(double v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(long double v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const std::string& v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const char* v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const std::wstring& v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}
void ThreadedLog::Write(const wchar_t* v) {
	if (!WillWrite()) return;
	claim_line(m_lock);
	Log::Write(v);
}

void ThreadedLog::Write(const Level& level) {
	// Must serialize access to Implementation (Enabled / possible flush).
	// If after the update the line is filtered, release immediately so a
	// disabled hot path does not hold the lock until endl.
	claim_line(m_lock);
	Log::Write(level);
	if (!WillWrite()) {
		release_line(m_lock);
	}
}

void ThreadedLog::Write(std::ostream& (*manip)(std::ostream&)) {
	if (WillWrite()) {
		claim_line(m_lock);
		Log::Write(manip);
		if (manipulator_writes_newline(manip)) {
			release_line(m_lock);
		}
	} else {
		Log::Write(manip);
		// Filtered: lock already released in Write(Level); no newline probe.
	}
}

void ThreadedLog::Write(Log& (*manip)(Log&) noexcept) {
    // Manipulators (humanreadable_*, etc.) mutate Implementation.
    // Always serialize; release immediately if the line is still filtered.
    claim_line(m_lock);
    Log::Write(manip);
    if (!WillWrite()) {
        release_line(m_lock);
    }
}
