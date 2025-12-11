#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <thread>
#include <vector>
#include <regex>
#include <future>
#include <chrono>

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

    // Validate all lines are present and well-formed, and count matches.
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
    // Ensure that writes without std::endl do not accidentally flush ownership.
    std::ostringstream output;
    ThreadedLog tlog(output, Level::Info, "%L:");

    // Write several parts without endl from different threads; since no endl,
    // interleaving is allowed. This test ensures program runs and produces
    // something, but does not assert strong ordering here.
    const int threads = 4;
    const int parts = 10;
    auto worker = [&](int id) {
        for (int i = 0; i < parts; ++i) {
            tlog << Level::Info << "p" << id << ":" << i << " ";
        }
        // terminate the logical line
        tlog << std::endl;
    };

    std::vector<std::thread> pool;
    for (int t = 0; t < threads; ++t) pool.emplace_back(worker, t);
    for (auto &th : pool) th.join();

    // Ensure lines count is correct
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

    // Promises/futures to start each thread in a controlled order and to
    // observe completion before starting the next.
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
            // Wait until main signals this thread to run
            start_futures[i].get();
            // Perform a single logical write (ends with std::endl)
            tlog << Level::Info << "T" << i << std::endl;
            // Signal completion
            done_promises[i].set_value();
        });
    }

    // Sequentially trigger each thread and wait for it to finish before
    // starting the next one to enforce deterministic ordering.
    for (int i = 0; i < threads; ++i) {
        start_promises[i].set_value();
        // Wait until the thread reports completion
        done_futures[i].get();
        // Small pause to avoid scheduling races (not strictly necessary)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    for (auto &th : pool) th.join();

    // Validate the output lines are exactly in order T0..Tn-1
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

int main() {
    int result = 0;
    result += test_threadedlog_basic();
    result += test_threadedlog_multithreaded_ordering();
    result += test_threadedlog_no_endl_sharing();
    result += test_threadedlog_deterministic_ordering();

    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " tests failed." << std::endl;
    }
    return result;
}
