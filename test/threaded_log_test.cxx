#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <thread>
#include <vector>
#include <regex>
#include <future>
#include <chrono>
#include <atomic>

using namespace StormByte::Logger;

int test_threadedlog_basic() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	tlog << Level::Info << "Threaded basic message" << std::endl;

	std::string expected = "Info    : Threaded basic message\n";
	ASSERT_EQUAL("test_threadedlog_basic", expected, output.str());
	RETURN_TEST("test_threadedlog_basic", 0);
}

int test_threadedlog_multithreaded_ordering() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	const int threads = 8;
	const int repeats = 50;

	auto worker = [&](int id) {
		for (int i = 0; i < repeats; ++i) {
			tlog << Level::Info << "T" << id << ":" << i << std::endl;
		}
	};

	std::vector<std::thread> pool;
	for (int t = 0; t < threads; ++t) pool.emplace_back(worker, t);
	for (auto &th : pool) th.join();

	std::istringstream in(output.str());
	std::string line;
	int count = 0;
	std::regex r("^Info\\s+: T\\d+:\\d+$");
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		if (!std::regex_match(line, r)) {
			ASSERT_EQUAL("test_threadedlog_multithreaded_ordering (line_format)", "OK", std::string("BAD: ") + line);
			RETURN_TEST("test_threadedlog_multithreaded_ordering", 1);
		}
		++count;
	}

	int expected = threads * repeats;
	if (count != expected) {
		ASSERT_EQUAL("test_threadedlog_multithreaded_ordering (count)", std::to_string(expected), std::to_string(count));
		RETURN_TEST("test_threadedlog_multithreaded_ordering", 1);
	}

	RETURN_TEST("test_threadedlog_multithreaded_ordering", 0);
}

int test_threadedlog_no_endl_sharing() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	const int threads = 4;
	const int parts = 10;
	auto worker = [&](int id) {
		for (int i = 0; i < parts; ++i) {
			tlog << Level::Info << "p" << id << ":" << i << " ";
		}
		tlog << std::endl;
	};

	std::vector<std::thread> pool;
	for (int t = 0; t < threads; ++t) pool.emplace_back(worker, t);
	for (auto &th : pool) th.join();

	std::istringstream in(output.str());
	std::string line;
	int count = 0;
	while (std::getline(in, line)) {
		if (!line.empty()) ++count;
	}
	int expected = threads;
	ASSERT_EQUAL("test_threadedlog_no_endl_sharing", std::to_string(expected), std::to_string(count));
	RETURN_TEST("test_threadedlog_no_endl_sharing", 0);
}

int test_threadedlog_deterministic_ordering() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	const int threads = 6;

	std::vector<std::promise<void>> start_promises(threads);
	std::vector<std::future<void>> start_futures;
	start_futures.reserve(threads);
	for (int i = 0; i < threads; ++i) start_futures.push_back(start_promises[i].get_future());

	std::vector<std::promise<void>> done_promises(threads);
	std::vector<std::future<void>> done_futures;
	done_futures.reserve(threads);
	for (int i = 0; i < threads; ++i) done_futures.push_back(done_promises[i].get_future());

	std::vector<std::thread> pool;
	for (int i = 0; i < threads; ++i) {
		pool.emplace_back([i, &tlog, &start_futures, &done_promises]() {
			start_futures[i].get();
			tlog << Level::Info << "T" << i << std::endl;
			done_promises[i].set_value();
		});
	}

	for (int i = 0; i < threads; ++i) {
		start_promises[i].set_value();
		done_futures[i].get();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	for (auto &th : pool) th.join();

	std::istringstream in(output.str());
	std::string line;
	int idx = 0;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		std::string expected = "Info    : T" + std::to_string(idx);
		ASSERT_EQUAL("test_threadedlog_deterministic_ordering", expected, line);
		++idx;
	}
	if (idx != threads) {
		ASSERT_EQUAL("test_threadedlog_deterministic_ordering (count)", std::to_string(threads), std::to_string(idx));
		RETURN_TEST("test_threadedlog_deterministic_ordering", 1);
	}

	RETURN_TEST("test_threadedlog_deterministic_ordering", 0);
}

int test_smart_pointer_usage() {
	std::ostringstream output;
	std::shared_ptr<StormByte::Logger::Log> log = std::make_shared<StormByte::Logger::ThreadedLog>(output, Level::Info, "%L:");

	log << Level::Info << "Smart pointer log message" << std::endl;

	std::string expected = "Info    : Smart pointer log message\n";
	ASSERT_EQUAL("test_smart_pointer_usage", expected, output.str());
	RETURN_TEST("test_smart_pointer_usage", 0);
}

// --- New: filtered path must not leak the lock ---

int test_threadedlog_filtered_endl_no_deadlock() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	// These must not hold the lock after endl (filtered messages).
	for (int i = 0; i < 50; ++i) {
		tlog << Level::Debug << "hidden " << i << std::endl;
	}

	// If the lock leaked, this Info line would hang.
	tlog << Level::Info << "after filtered" << std::endl;

	std::string expected = "Info    : after filtered\n";
	ASSERT_EQUAL("test_threadedlog_filtered_endl_no_deadlock", expected, output.str());
	RETURN_TEST("test_threadedlog_filtered_endl_no_deadlock", 0);
}

int test_threadedlog_filtered_multithreaded_then_info() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	const int threads = 4;
	const int repeats = 40;
	std::atomic<int> done{0};

	auto worker = [&](int id) {
		for (int i = 0; i < repeats; ++i) {
			tlog << Level::Debug << "d" << id << ":" << i << std::endl;
		}
		done.fetch_add(1);
	};

	std::vector<std::thread> pool;
	for (int t = 0; t < threads; ++t) pool.emplace_back(worker, t);
	for (auto &th : pool) th.join();

	if (done.load() != threads) {
		ASSERT_EQUAL("test_threadedlog_filtered_multithreaded_then_info (workers)", std::to_string(threads), std::to_string(done.load()));
		RETURN_TEST("test_threadedlog_filtered_multithreaded_then_info", 1);
	}

	tlog << Level::Info << "ok" << std::endl;

	std::string expected = "Info    : ok\n";
	ASSERT_EQUAL("test_threadedlog_filtered_multithreaded_then_info", expected, output.str());
	RETURN_TEST("test_threadedlog_filtered_multithreaded_then_info", 0);
}

int test_threadedlog_level_switch_flush() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Debug, "%L:");

	// Start an Info line without endl, then switch level (Implementation may emit newline).
	tlog << Level::Info << "part1";
	tlog << Level::Debug << "part2" << std::endl;

	std::string out = output.str();
	// Expect at least that both parts appear and there is a newline structure.
	if (out.find("part1") == std::string::npos || out.find("part2") == std::string::npos) {
		ASSERT_EQUAL("test_threadedlog_level_switch_flush", std::string("contains part1 and part2"), out);
		RETURN_TEST("test_threadedlog_level_switch_flush", 1);
	}

	RETURN_TEST("test_threadedlog_level_switch_flush", 0);
}

int main() {
	int result = 0;
	result += test_threadedlog_basic();
	result += test_threadedlog_multithreaded_ordering();
	result += test_threadedlog_no_endl_sharing();
	result += test_threadedlog_deterministic_ordering();
	result += test_smart_pointer_usage();
	result += test_threadedlog_filtered_endl_no_deadlock();
	result += test_threadedlog_filtered_multithreaded_then_info();
	result += test_threadedlog_level_switch_flush();

	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}