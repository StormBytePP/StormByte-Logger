#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>

using namespace StormByte::Logger;

// High-volume filtered Log: must produce no output and not hang.
int test_log_filtered_high_volume() {
	std::ostringstream output;
	Log log(output, Level::Error, "%L:");

	constexpr int N = 100000;
	const auto t0 = std::chrono::steady_clock::now();
	for (int i = 0; i < N; ++i) {
		log << Level::Debug << "x=" << i << " b=" << true << " d=" << 1.5 << std::endl;
		log << Level::Info << "info " << i << std::endl;
		log << Level::Warning << "warn " << i << std::endl;
	}
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - t0).count();

	ASSERT_EQUAL("test_log_filtered_high_volume (output)", std::string(""), output.str());
	std::cout << "  [perf] Log filtered " << (N * 3) << " lines in " << ms << " ms\n";
	RETURN_TEST("test_log_filtered_high_volume", 0);
}

// Filtered ThreadedLog then one visible line (lock must not leak).
int test_threaded_filtered_high_volume() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Error, "%L:");

	constexpr int N = 50000;
	const auto t0 = std::chrono::steady_clock::now();
	for (int i = 0; i < N; ++i) {
		tlog << Level::Debug << "hidden " << i << std::endl;
	}
	tlog << Level::Error << "only" << std::endl;
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - t0).count();

	ASSERT_EQUAL("test_threaded_filtered_high_volume (output)",
		std::string("Error   : only\n"), output.str());
	std::cout << "  [perf] ThreadedLog filtered " << N << " lines in " << ms << " ms\n";
	RETURN_TEST("test_threaded_filtered_high_volume", 0);
}

// Concurrent filtered spam, then one Info.
int test_threaded_filtered_multithreaded_volume() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	constexpr int threads = 8;
	constexpr int per_thread = 8000;
	std::atomic<int> finished{0};

	auto worker = [&](int id) {
		for (int i = 0; i < per_thread; ++i) {
			tlog << Level::Debug << "t" << id << ":" << i << std::endl;
		}
		finished.fetch_add(1, std::memory_order_relaxed);
	};

	std::vector<std::thread> pool;
	pool.reserve(threads);
	const auto t0 = std::chrono::steady_clock::now();
	for (int t = 0; t < threads; ++t) {
		pool.emplace_back(worker, t);
	}
	for (auto& th : pool) {
		th.join();
	}
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - t0).count();

	ASSERT_EQUAL("test_threaded_filtered_multithreaded_volume (workers)",
		std::to_string(threads), std::to_string(finished.load()));

	tlog << Level::Info << "done" << std::endl;
	ASSERT_EQUAL("test_threaded_filtered_multithreaded_volume (output)",
		std::string("Info    : done\n"), output.str());

	std::cout << "  [perf] ThreadedLog filtered "
			<< (threads * per_thread) << " lines (" << threads << " threads) in "
			<< ms << " ms\n";
	RETURN_TEST("test_threaded_filtered_multithreaded_volume", 0);
}

int main() {
	int result = 0;
	result += test_log_filtered_high_volume();
	result += test_threaded_filtered_high_volume();
	result += test_threaded_filtered_multithreaded_volume();

	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
