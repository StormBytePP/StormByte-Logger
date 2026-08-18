#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <string>

using namespace StormByte::Logger;

int test_manip_humanreadable_number_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_number << 1000 << std::endl;

	std::string expected = "Info    : 1,000\n";
	ASSERT_EQUAL("test_manip_humanreadable_number_log", expected, output.str());
	RETURN_TEST("test_manip_humanreadable_number_log", 0);
}

int test_manip_humanreadable_bytes_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_bytes << 10240 << std::endl;

	std::string expected = "Info    : 10 KiB\n";
	ASSERT_EQUAL("test_manip_humanreadable_bytes_log", expected, output.str());
	RETURN_TEST("test_manip_humanreadable_bytes_log", 0);
}

int test_manip_nohumanreadable_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_number << 1000 << std::endl;
	log << Level::Info << nohumanreadable << 1000 << std::endl;

	std::string expected = "Info    : 1,000\nInfo    : 1000\n";
	ASSERT_EQUAL("test_manip_nohumanreadable_log", expected, output.str());
	RETURN_TEST("test_manip_nohumanreadable_log", 0);
}

int test_manip_chainable_threadedlog() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	tlog << Level::Info << humanreadable_number << humanreadable_bytes << 10240 << std::endl;

	std::string expected = "Info    : 10 KiB\n";
	ASSERT_EQUAL("test_manip_chainable_threadedlog", expected, output.str());
	RETURN_TEST("test_manip_chainable_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Redact: keep first N characters visible; rest become '*'.
// redact / redact(0) → mask all.
// ---------------------------------------------------------------------------

int test_manip_redact_full_string() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact << "secret" << std::endl;

	std::string expected = "Info    : ******\n";
	ASSERT_EQUAL("test_manip_redact_full_string", expected, output.str());
	RETURN_TEST("test_manip_redact_full_string", 0);
}

int test_manip_redact_keep_first() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact(4) << "super-secret" << std::endl;

	// "super-secret" (12) → supe********
	std::string expected = "Info    : supe********\n";
	ASSERT_EQUAL("test_manip_redact_keep_first", expected, output.str());
	RETURN_TEST("test_manip_redact_keep_first", 0);
}

int test_manip_redact_keep_first_zero_same_as_full() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact(0) << "abc" << std::endl;

	std::string expected = "Info    : ***\n";
	ASSERT_EQUAL("test_manip_redact_keep_first_zero_same_as_full", expected, output.str());
	RETURN_TEST("test_manip_redact_keep_first_zero_same_as_full", 0);
}

int test_manip_redact_keep_first_ge_length() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	// N >= length → nothing masked
	log << Level::Info << redact(10) << "abc" << std::endl;

	std::string expected = "Info    : abc\n";
	ASSERT_EQUAL("test_manip_redact_keep_first_ge_length", expected, output.str());
	RETURN_TEST("test_manip_redact_keep_first_ge_length", 0);
}

int test_manip_redact_empty_string() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact << "" << std::endl;

	std::string expected = "Info    : \n";
	ASSERT_EQUAL("test_manip_redact_empty_string", expected, output.str());
	RETURN_TEST("test_manip_redact_empty_string", 0);
}

int test_manip_redact_const_char_ptr() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	const char* token = "password123";
	log << Level::Info << redact(3) << token << std::endl;

	// "password123" (11) → pas********
	std::string expected = "Info    : pas********\n";
	ASSERT_EQUAL("test_manip_redact_const_char_ptr", expected, output.str());
	RETURN_TEST("test_manip_redact_const_char_ptr", 0);
}

int test_manip_no_redact_restores_plain() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact << "hidden" << std::endl;
	log << Level::Info << no_redact << "visible" << std::endl;

	std::string expected = "Info    : ******\nInfo    : visible\n";
	ASSERT_EQUAL("test_manip_no_redact_restores_plain", expected, output.str());
	RETURN_TEST("test_manip_no_redact_restores_plain", 0);
}

int test_manip_redact_stays_active() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact(2) << "one" << " " << "two" << std::endl;
	log << Level::Info << "three" << std::endl;
	log << Level::Info << no_redact << "four" << std::endl;

	// "one" → on*, " " → *, "two" → tw*, "three" → th***
	std::string expected = "Info    : on* tw*\nInfo    : th***\nInfo    : four\n";
	ASSERT_EQUAL("test_manip_redact_stays_active", expected, output.str());
	RETURN_TEST("test_manip_redact_stays_active", 0);
}

int test_manip_redact_does_not_affect_numbers() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	// Whole " secret" (leading space included) is redacted
	log << Level::Info << redact << 42 << " secret" << std::endl;

	std::string expected = "Info    : 42*******\n";
	ASSERT_EQUAL("test_manip_redact_does_not_affect_numbers", expected, output.str());
	RETURN_TEST("test_manip_redact_does_not_affect_numbers", 0);
}

int test_manip_redact_then_change_keep() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << redact << "abcdef" << std::endl;
	log << Level::Info << redact(2) << "abcdef" << std::endl;
	log << Level::Info << redact << "abcdef" << std::endl;

	std::string expected = "Info    : ******\nInfo    : ab****\nInfo    : ******\n";
	ASSERT_EQUAL("test_manip_redact_then_change_keep", expected, output.str());
	RETURN_TEST("test_manip_redact_then_change_keep", 0);
}

int test_manip_redact_threadedlog() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");

	tlog << Level::Info << redact(4) << "super-secret" << std::endl;
	tlog << Level::Info << no_redact << "ok" << std::endl;

	std::string expected = "Info    : supe********\nInfo    : ok\n";
	ASSERT_EQUAL("test_manip_redact_threadedlog", expected, output.str());
	RETURN_TEST("test_manip_redact_threadedlog", 0);
}

int test_manip_redact_with_humanreadable_independent() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	log << Level::Info << humanreadable_number << redact << 1000 << " token" << std::endl;
	log << Level::Info << no_redact << nohumanreadable << 1000 << std::endl;

	// " token" (6 chars) → ******
	std::string expected = "Info    : 1,000******\nInfo    : 1000\n";
	ASSERT_EQUAL("test_manip_redact_with_humanreadable_independent", expected, output.str());
	RETURN_TEST("test_manip_redact_with_humanreadable_independent", 0);
}

int test_manip_redact_wstring() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");

	std::wstring wide = L"secret";
	log << Level::Info << redact << wide << std::endl;

	std::string expected = "Info    : ******\n";
	ASSERT_EQUAL("test_manip_redact_wstring", expected, output.str());
	RETURN_TEST("test_manip_redact_wstring", 0);
}

int main() {
	int result = 0;

	result += test_manip_humanreadable_number_log();
	result += test_manip_humanreadable_bytes_log();
	result += test_manip_nohumanreadable_log();
	result += test_manip_chainable_threadedlog();

	result += test_manip_redact_full_string();
	result += test_manip_redact_keep_first();
	result += test_manip_redact_keep_first_zero_same_as_full();
	result += test_manip_redact_keep_first_ge_length();
	result += test_manip_redact_empty_string();
	result += test_manip_redact_const_char_ptr();
	result += test_manip_no_redact_restores_plain();
	result += test_manip_redact_stays_active();
	result += test_manip_redact_does_not_affect_numbers();
	result += test_manip_redact_then_change_keep();
	result += test_manip_redact_threadedlog();
	result += test_manip_redact_with_humanreadable_independent();
	result += test_manip_redact_wstring();

	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
