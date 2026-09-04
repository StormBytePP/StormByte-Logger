/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Logger.
 *
 * StormByte-Logger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * or later, as published by the Free Software Foundation.
 *
 * StormByte-Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte-Logger. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 */

#include <StormByte/logger/log.hxx>
#include <StormByte/test_handlers.h>
#include <sstream>
#include <thread>
#include <vector>
#include <cstdio>
using namespace StormByte::Logger;
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
	std::string expected = "Info    : 1,000\n";
	ASSERT_EQUAL("test_humanreadable_number", expected, output.str());
	RETURN_TEST("test_humanreadable_number", 0);
}
int test_humanreadable_bytes() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_bytes << 10240 << std::endl;
	std::string expected = "Info    : 10 KiB\n";
	ASSERT_EQUAL("test_humanreadable_bytes", expected, output.str());
	RETURN_TEST("test_humanreadable_bytes", 0);
}
int test_nohumanreadable() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_number << 1000 << " " << nohumanreadable << 1000 << std::endl;
	std::string expected = "Info    : 1,000 1000\n";
	ASSERT_EQUAL("test_nohumanreadable", expected, output.str());
	RETURN_TEST("test_nohumanreadable", 0);
}
int test_humanreadable_enable_and_disable() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_number << 1000 << std::endl;
	std::string expected_enable = "Info    : 1,000\n";
	ASSERT_EQUAL("test_humanreadable_enable_and_disable (enable)", expected_enable, output.str());
	output.str("");
	output.clear();
	log << Level::Info << nohumanreadable << 1000 << std::endl;
	std::string expected_disable = "Info    : 1000\n";
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
// --- New: filtered fast-path ---
int test_filtered_produces_empty_output() {
	std::ostringstream output;
	Log log(output, Level::Error, "%L:");
	for (int i = 0; i < 100; ++i) {
		log << Level::Debug << "debug " << i << " " << true << " " << 3.14 << std::endl;
		log << Level::Info << "info " << i << std::endl;
		log << Level::Warning << "warn " << i << std::endl;
	}
	ASSERT_EQUAL("test_filtered_produces_empty_output", std::string(""), output.str());
	RETURN_TEST("test_filtered_produces_empty_output", 0);
}
int test_filtered_then_enabled_message() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Debug << "should not appear " << 123 << std::endl;
	log << Level::Info << "visible" << std::endl;
	log << Level::Debug << "still hidden" << std::endl;
	log << Level::Error << "error visible" << std::endl;
	std::string expected = "Info    : visible\nError   : error visible\n";
	ASSERT_EQUAL("test_filtered_then_enabled_message", expected, output.str());
	RETURN_TEST("test_filtered_then_enabled_message", 0);
}
int test_escaped_percent_in_format() {
	std::ostringstream output;
	Log log(output, Level::Info, "[%%] [%L]:");
	log << Level::Info << "ok" << std::endl;
	std::string out = output.str();
	if (out.find("%L") != std::string::npos) {
		ASSERT_EQUAL("test_escaped_percent_in_format (leftover %L)", std::string("none"), std::string("%L"));
		RETURN_TEST("test_escaped_percent_in_format", 1);
	}
	if (out.find("[%]") == std::string::npos && out.find("[% ]") == std::string::npos) {
		// Expect literal % then space-padded level region; at least "[%]" or "[% Info..."
		if (out.find("[%") == std::string::npos) {
			ASSERT_EQUAL("test_escaped_percent_in_format (missing literal %)", std::string("found"), out);
			RETURN_TEST("test_escaped_percent_in_format", 1);
		}
	}
	if (out.find("ok") == std::string::npos) {
		ASSERT_EQUAL("test_escaped_percent_in_format (missing message)", std::string("ok"), out);
		RETURN_TEST("test_escaped_percent_in_format", 1);
	}
	// Must contain a literal percent sign from %%
	if (out.find('%') == std::string::npos) {
		ASSERT_EQUAL("test_escaped_percent_in_format (no percent char)", std::string("has %"), out);
		RETURN_TEST("test_escaped_percent_in_format", 1);
	}
	RETURN_TEST("test_escaped_percent_in_format", 0);
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
	result += test_smart_pointer_usage();
	result += test_filtered_produces_empty_output();
	result += test_filtered_then_enabled_message();
	result += test_escaped_percent_in_format();
	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
