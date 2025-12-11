#include <StormByte/logger/log.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <thread>
#include <vector>
#include <cstdio>

using namespace StormByte::Logger;

// Function to test basic logging at different levels
int test_basic_logging() {
	std::ostringstream output;
	Log log(output, Level::Debug, "%L:");

	log << Level::Info << "Info message" << std::endl;
	log << Level::Debug << "Debug message" << std::endl;
	log << Level::Error << "Error message" << std::endl;

	std::string expected = "Info    : Info message\nDebug   : Debug message\nError   : Error message\n";
	ASSERT_EQUAL("test_basic_logging", expected, output.str());
	RETURN_TEST("test_basic_logging", 0);
}

// Function to test log level filtering
int test_log_level_filtering() {
	std::ostringstream output;
	Log log(output, Level::Error, "%L:");

	log << Level::Info << "Info message" << std::endl;
	log << Level::Warning << "Warning message" << std::endl;
	log << Level::Error << "Error message" << std::endl;

	std::string expected = "Error   : Error message\n";
	ASSERT_EQUAL("test_log_level_filtering", expected, output.str());
	RETURN_TEST("test_log_level_filtering", 0);
}

// Test several data logging
int test_log_data() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	int i = 42;
	bool b = true;
	double d = 3.141596;

	log << Level::Info << "Info message with sample integer " << i << ", a bool " << b << " and a double " << d << std::endl;

	std::string expected = "Info    : Info message with sample integer 42, a bool true and a double 3.141596\n";
	ASSERT_EQUAL("test_log_data", expected, output.str());
	RETURN_TEST("test_log_data", 0);
}

// Test log to stdout
int log_to_stdout() {
	Log log(std::cout, Level::Info, "%L:");
	log << Level::Info << "Info message" << std::endl;
	log << Level::Debug << "Debug message" << std::endl;
	log << Level::Error << "Error message" << std::endl;
	RETURN_TEST("log_to_stdout", 0);
}

int test_log_with_std_endl() {
	std::ostringstream output;
	Log log(output, Level::Debug, "%L:");

	log << Level::Info << "Info message" << std::endl;
	log << Level::Debug << "Debug message" << std::endl;
	log << Level::Error << "Error message" << std::endl;

	std::string expected = "Info    : Info message\nDebug   : Debug message\nError   : Error message\n";
	ASSERT_EQUAL("test_log_with_std_endl", expected, output.str());
	RETURN_TEST("test_log_with_std_endl", 0);
}

int test_humanreadable_number() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_number << 1000 << std::endl;

	std::string expected = "Info    : 1,000\n"; // Assuming the locale uses commas for thousands
	ASSERT_EQUAL("test_humanreadable_number", expected, output.str());
	RETURN_TEST("test_humanreadable_number", 0);
}

// Test enabling human-readable bytes formatting
int test_humanreadable_bytes() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_bytes << 10240 << std::endl;

	std::string expected = "Info    : 10 KiB\n"; // Example: 10240 bytes = 10 KiB
	ASSERT_EQUAL("test_humanreadable_bytes", expected, output.str());
	RETURN_TEST("test_humanreadable_bytes", 0);
}

// Test disabling human-readable formatting
int test_nohumanreadable() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_number << 1000 << " " << nohumanreadable << 1000 << std::endl;

	std::string expected = "Info    : 1,000 1000\n"; // First is formatted, second is raw
	ASSERT_EQUAL("test_nohumanreadable", expected, output.str());
	RETURN_TEST("test_nohumanreadable", 0);
}

int test_humanreadable_enable_and_disable() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	// Enable human-readable number formatting
	log << Level::Info << humanreadable_number << 1000 << std::endl;
	std::string expected_enable = "Info    : 1,000\n"; // Human-readable with thousand separator
	ASSERT_EQUAL("test_humanreadable_enable_and_disable (enable)", expected_enable, output.str());

	// Clear the stream for the next test
	output.str("");
	output.clear();

	// Disable human-readable formatting (Raw output)
	log << Level::Info << nohumanreadable << 1000 << std::endl;
	std::string expected_disable = "Info    : 1000\n"; // Raw format, no thousand separator
	ASSERT_EQUAL("test_humanreadable_enable_and_disable (disable)", expected_disable, output.str());

	RETURN_TEST("test_humanreadable_enable_and_disable", 0);
}

int test_smart_pointer_usage() {
	std::ostringstream output;
	auto log = std::make_shared<StormByte::Logger::Log>(output, Level::Info, "%L:");

	log << Level::Info << "Smart pointer log message" << std::endl;

	std::string expected = "Info    : Smart pointer log message\n";
	ASSERT_EQUAL("test_smart_pointer_usage", expected, output.str());
	RETURN_TEST("test_smart_pointer_usage", 0);
}

int main() {
	int result = 0;

	result += test_basic_logging();
	result += test_log_level_filtering();
	result += test_log_data();
	result += log_to_stdout();
	result += test_log_with_std_endl();
	result += test_humanreadable_number();
	result += test_humanreadable_bytes();
	result += test_nohumanreadable();
	result += test_humanreadable_enable_and_disable();

	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
