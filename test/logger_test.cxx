#include <StormByte/logger/logger.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>

using namespace StormByte::Log;

// Function to test basic logging at different levels
int test_basic_logging() {
    std::ostringstream output;
    Logger logger(output, Level::Debug, "%L: ");

    logger << Level::Info << "Info message";
    logger << Level::Debug << "Debug message";
    logger << Level::Error << "Error message";

    std::string expected = "Info: Info message\nDebug: Debug message\nError: Error message";
    ASSERT_EQUAL("test_basic_logging", expected, output.str());
    RETURN_TEST("test_basic_logging", 0);
}

// Function to test log level filtering
int test_log_level_filtering() {
    std::ostringstream output;
    Logger logger(output, Level::Error, "%L: ");

    logger << Level::Info << "Info message";
    logger << Level::Warning << "Warning message";
    logger << Level::Error << "Error message";

    std::string expected = "Error: Error message";
    ASSERT_EQUAL("test_log_level_filtering", expected, output.str());
    RETURN_TEST("test_log_level_filtering", 0);
}

// Test several data logging
int test_log_data() {
	std::ostringstream output;
	Logger logger(output, Level::Info, "%L: ");

	int i = 42;
	bool b = true;
	double d = 3.141596;

	logger << Level::Info << "Info message with sample integer " << i << ", a bool " << b << " and a double " << d;

	std::string expected = "Info: Info message with sample integer 42, a bool true and a double 3.141596";
	ASSERT_EQUAL("test_log_data", expected, output.str());
	RETURN_TEST("test_log_data", 0);
}

// Test log to stdout
int log_to_stdout() {
	Logger logger(std::cout, Level::Info, "%L: ");
	logger << Level::Info << "Info message";
	logger << Level::Debug << "Debug message";
	logger << Level::Error << "Error message";
	logger << "\n";
	RETURN_TEST("log_to_stdout", 0);
}

int log_as_shared_ptr() {
	std::ostringstream output;
	std::shared_ptr<Logger> logger = std::make_shared<Logger>(output, Level::Debug, "%L: ");

	logger << Level::Info << "Info message";
	logger << Level::Debug << "Debug message";
	logger << Level::Error << "Error message";

	std::string expected = "Info: Info message\nDebug: Debug message\nError: Error message";
	ASSERT_EQUAL("log_as_shared_ptr", expected, output.str());
	RETURN_TEST("log_as_shared_ptr", 0);
}

int main() {
    int result = 0;
    try {
		result += test_basic_logging();
		result += test_log_level_filtering();
		result += test_log_data();
		result += log_to_stdout();
		result += log_as_shared_ptr();
        std::cout << "All tests passed successfully.\n";
    } catch (...) {
        result++;
	}

    return result;
}
