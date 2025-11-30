#include <StormByte/logger/log.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <regex>

using namespace StormByte::Logger;

int test_format_mask_literals() {
    std::ostringstream output;
    // Format includes a literal 'T' inside brackets followed by %i (thread id)
    Log log(output, Level::Info, "[%L] [T%i] %T: ");

    log << Level::Info << "hello" << std::endl;

    std::string out = output.str();

    // Ensure no placeholders remain
    if (out.find("%L") != std::string::npos) {
        ASSERT_EQUAL("test_format_mask_literals (leftover %L)", "none", "%L");
        RETURN_TEST("test_format_mask_literals", 1);
    }
    if (out.find("%i") != std::string::npos) {
        ASSERT_EQUAL("test_format_mask_literals (leftover %i)", "none", "%i");
        RETURN_TEST("test_format_mask_literals", 1);
    }
    if (out.find("%T") != std::string::npos) {
        ASSERT_EQUAL("test_format_mask_literals (leftover %T)", "none", "%T");
        RETURN_TEST("test_format_mask_literals", 1);
    }

    // Literal 'T' followed by thread id should appear as "[T"
    if (out.find("[T") == std::string::npos) {
        ASSERT_EQUAL("test_format_mask_literals (missing [T)", "found", out);
        RETURN_TEST("test_format_mask_literals", 1);
    }

    // Message should be present
    if (out.find("hello") == std::string::npos) {
        ASSERT_EQUAL("test_format_mask_literals (missing message)", "hello", out);
        RETURN_TEST("test_format_mask_literals", 1);
    }

    RETURN_TEST("test_format_mask_literals", 0);
}

int main() {
    int result = 0;
    result += test_format_mask_literals();

    if (result == 0) std::cout << "All tests passed!" << std::endl;
    else std::cout << result << " tests failed." << std::endl;
    return result;
}
