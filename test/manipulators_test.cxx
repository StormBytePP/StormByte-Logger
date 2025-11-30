#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>

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
    // now raw
    log << Level::Info << nohumanreadable << 1000 << std::endl;

    std::string expected = "Info    : 1,000\nInfo    : 1000\n";
    ASSERT_EQUAL("test_manip_nohumanreadable_log", expected, output.str());
    RETURN_TEST("test_manip_nohumanreadable_log", 0);
}

int test_manip_chainable_threadedlog() {
    std::ostringstream output;
    ThreadedLog tlog(output, Level::Info, "%L:");

    // chain manipulators - number then bytes: last should take effect for numeric output
    tlog << Level::Info << humanreadable_number << humanreadable_bytes << 10240 << std::endl;

    // Expect bytes formatting to apply (humanreadable_bytes applied last)
    std::string expected = "Info    : 10 KiB\n";
    ASSERT_EQUAL("test_manip_chainable_threadedlog", expected, output.str());
    RETURN_TEST("test_manip_chainable_threadedlog", 0);
}

int main() {
    int result = 0;
    result += test_manip_humanreadable_number_log();
    result += test_manip_humanreadable_bytes_log();
    result += test_manip_nohumanreadable_log();
    result += test_manip_chainable_threadedlog();

    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " tests failed." << std::endl;
    }
    return result;
}
