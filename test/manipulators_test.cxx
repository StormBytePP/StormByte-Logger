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

#include <StormByte/logger/exception.hxx>
#include <StormByte/logger/log.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/test_handlers.h>

#include <sstream>
#include <string>

using namespace StormByte::Logger;

// ---------------------------------------------------------------------------
// Human-readable number / bytes
// ---------------------------------------------------------------------------

int test_manip_humanreadable_number_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_number << 1000 << std::endl;
	ASSERT_EQUAL("test_manip_humanreadable_number_log", "Info    : 1,000\n", output.str());
	RETURN_TEST("test_manip_humanreadable_number_log", 0);
}

int test_manip_humanreadable_bytes_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_bytes << 10240 << std::endl;
	ASSERT_EQUAL("test_manip_humanreadable_bytes_log", "Info    : 10 KiB\n", output.str());
	RETURN_TEST("test_manip_humanreadable_bytes_log", 0);
}

int test_manip_nohumanreadable_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_number << 1000 << std::endl;
	log << Level::Info << nohumanreadable << 1000 << std::endl;
	ASSERT_EQUAL("test_manip_nohumanreadable_log", "Info    : 1,000\nInfo    : 1000\n", output.str());
	RETURN_TEST("test_manip_nohumanreadable_log", 0);
}

int test_manip_chainable_threadedlog() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	tlog << Level::Info << humanreadable_number << humanreadable_bytes << 10240 << std::endl;
	ASSERT_EQUAL("test_manip_chainable_threadedlog", "Info    : 10 KiB\n", output.str());
	RETURN_TEST("test_manip_chainable_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Redact: keep last N visible; redact / redact(0) masks all.
// Numbers are converted then redacted. redact_first keeps a prefix.
// ---------------------------------------------------------------------------

int test_manip_redact_full_string() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact << "secret" << std::endl;
	ASSERT_EQUAL("test_manip_redact_full_string", "Info    : ******\n", output.str());
	RETURN_TEST("test_manip_redact_full_string", 0);
}

int test_manip_redact_keep_last() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact(4) << "super-secret" << std::endl;
	ASSERT_EQUAL("test_manip_redact_keep_last", "Info    : ********cret\n", output.str());
	RETURN_TEST("test_manip_redact_keep_last", 0);
}

int test_manip_redact_keep_last_zero_same_as_full() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact(0) << "abc" << std::endl;
	ASSERT_EQUAL("test_manip_redact_keep_last_zero_same_as_full", "Info    : ***\n", output.str());
	RETURN_TEST("test_manip_redact_keep_last_zero_same_as_full", 0);
}

int test_manip_redact_keep_last_ge_length() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact(10) << "abc" << std::endl;
	ASSERT_EQUAL("test_manip_redact_keep_last_ge_length", "Info    : abc\n", output.str());
	RETURN_TEST("test_manip_redact_keep_last_ge_length", 0);
}

int test_manip_redact_empty_string() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact << "" << std::endl;
	ASSERT_EQUAL("test_manip_redact_empty_string", "Info    : \n", output.str());
	RETURN_TEST("test_manip_redact_empty_string", 0);
}

int test_manip_redact_const_char_ptr() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	const char* token = "password123";
	log << Level::Info << redact(3) << token << std::endl;
	ASSERT_EQUAL("test_manip_redact_const_char_ptr", "Info    : ********123\n", output.str());
	RETURN_TEST("test_manip_redact_const_char_ptr", 0);
}

int test_manip_noredact_restores_plain() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact << "hidden" << std::endl;
	log << Level::Info << noredact << "visible" << std::endl;
	ASSERT_EQUAL("test_manip_noredact_restores_plain", "Info    : ******\nInfo    : visible\n", output.str());
	RETURN_TEST("test_manip_noredact_restores_plain", 0);
}

int test_manip_redact_stays_active() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact(2) << "one" << " " << "two" << std::endl;
	log << Level::Info << "three" << std::endl;
	log << Level::Info << noredact << "four" << std::endl;
	ASSERT_EQUAL("test_manip_redact_stays_active", "Info    : *ne *wo\nInfo    : ***ee\nInfo    : four\n", output.str());
	RETURN_TEST("test_manip_redact_stays_active", 0);
}

int test_manip_redact_affects_numbers() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact << 42 << " secret" << std::endl;
	ASSERT_EQUAL("test_manip_redact_affects_numbers", "Info    : *********\n", output.str());
	RETURN_TEST("test_manip_redact_affects_numbers", 0);
}

int test_manip_redact_then_change_keep() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact << "abcdef" << std::endl;
	log << Level::Info << redact(2) << "abcdef" << std::endl;
	log << Level::Info << redact << "abcdef" << std::endl;
	ASSERT_EQUAL("test_manip_redact_then_change_keep", "Info    : ******\nInfo    : ****ef\nInfo    : ******\n", output.str());
	RETURN_TEST("test_manip_redact_then_change_keep", 0);
}

int test_manip_redact_threadedlog() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	tlog << Level::Info << redact(4) << "super-secret" << std::endl;
	tlog << Level::Info << noredact << "ok" << std::endl;
	ASSERT_EQUAL("test_manip_redact_threadedlog", "Info    : ********cret\nInfo    : ok\n", output.str());
	RETURN_TEST("test_manip_redact_threadedlog", 0);
}

int test_manip_redact_with_humanreadable_independent() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << humanreadable_number << redact << 1000 << " token" << std::endl;
	log << Level::Info << noredact << nohumanreadable << 1000 << std::endl;
	ASSERT_EQUAL("test_manip_redact_with_humanreadable_independent", "Info    : ***********\nInfo    : 1000\n", output.str());
	RETURN_TEST("test_manip_redact_with_humanreadable_independent", 0);
}

int test_manip_redact_wstring() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	std::wstring wide = L"secret";
	log << Level::Info << redact << wide << std::endl;
	ASSERT_EQUAL("test_manip_redact_wstring", "Info    : ******\n", output.str());
	RETURN_TEST("test_manip_redact_wstring", 0);
}

int test_manip_redact_first() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact_first(4) << "super-secret" << std::endl;
	ASSERT_EQUAL("test_manip_redact_first", "Info    : supe********\n", output.str());
	RETURN_TEST("test_manip_redact_first", 0);
}

int test_manip_redact_first_zero_same_as_full() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact_first(0) << "abc" << std::endl;
	ASSERT_EQUAL("test_manip_redact_first_zero_same_as_full", "Info    : ***\n", output.str());
	RETURN_TEST("test_manip_redact_first_zero_same_as_full", 0);
}

int test_manip_redact_first_ge_length() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << redact_first(10) << "abc" << std::endl;
	ASSERT_EQUAL("test_manip_redact_first_ge_length", "Info    : abc\n", output.str());
	RETURN_TEST("test_manip_redact_first_ge_length", 0);
}

int test_manip_redact_first_const_char_ptr() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	const char* token = "password123";
	log << Level::Info << redact_first(3) << token << std::endl;
	ASSERT_EQUAL("test_manip_redact_first_const_char_ptr", "Info    : pas********\n", output.str());
	RETURN_TEST("test_manip_redact_first_const_char_ptr", 0);
}

int test_manip_redact_first_threadedlog() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	tlog << Level::Info << redact_first(4) << "super-secret" << std::endl;
	tlog << Level::Info << noredact << "ok" << std::endl;
	ASSERT_EQUAL("test_manip_redact_first_threadedlog", "Info    : supe********\nInfo    : ok\n", output.str());
	RETURN_TEST("test_manip_redact_first_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Hex: dump payload bytes. hex(N) wraps every N bytes without a new header.
// hex(0) is nohex. Redact runs after hex.
// ---------------------------------------------------------------------------

int test_manip_hex_string() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << hex << "AB" << std::endl;
	ASSERT_EQUAL("test_manip_hex_string", "Info    : 0x41 0x42\n", output.str());
	RETURN_TEST("test_manip_hex_string", 0);
}

int test_manip_hex_columns_wrap_without_header() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << hex(2) << "ABCD" << std::endl;
	ASSERT_EQUAL("test_manip_hex_columns_wrap_without_header", "Info    : 0x41 0x42\n0x43 0x44\n", output.str());
	RETURN_TEST("test_manip_hex_columns_wrap_without_header", 0);
}

int test_manip_hex_zero_is_nohex() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << hex(0) << "AB" << std::endl;
	ASSERT_EQUAL("test_manip_hex_zero_is_nohex", "Info    : AB\n", output.str());
	RETURN_TEST("test_manip_hex_zero_is_nohex", 0);
}

int test_manip_nohex_restores_plain() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << hex << "A" << std::endl;
	log << Level::Info << nohex << "A" << std::endl;
	ASSERT_EQUAL("test_manip_nohex_restores_plain", "Info    : 0x41\nInfo    : A\n", output.str());
	RETURN_TEST("test_manip_nohex_restores_plain", 0);
}

int test_manip_hex_number_uses_text_bytes() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << hex << 42 << std::endl;
	ASSERT_EQUAL("test_manip_hex_number_uses_text_bytes", "Info    : 0x34 0x32\n", output.str());
	RETURN_TEST("test_manip_hex_number_uses_text_bytes", 0);
}

int test_manip_hex_then_redact() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << hex << redact << "A" << std::endl;
	ASSERT_EQUAL("test_manip_hex_then_redact", "Info    : ****\n", output.str());
	RETURN_TEST("test_manip_hex_then_redact", 0);
}

int test_manip_hex_threadedlog() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	log << Level::Info << hex(2) << "AB" << std::endl;
	ASSERT_EQUAL("test_manip_hex_threadedlog", "Info    : 0x41 0x42\n", output.str());
	RETURN_TEST("test_manip_hex_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Color
// ---------------------------------------------------------------------------

int test_manip_color_and_nocolor_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]%g");
	log.Color(Level::Info, Color::Red);
	log << component("Core") << group("work") << Level::Info
		<< nocolor << "plain " << color(Color::Green) << "green" << std::endl;
	ASSERT_EQUAL("test_manip_color_and_nocolor_log",
		"\033[31mCore[Info    ]work \033[0mplain \033[32mgreen\033[0m\n", output.str());
	RETURN_TEST("test_manip_color_and_nocolor_log", 0);
}

int test_manip_color_threadedlog() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	log.Color(Level::Info, Color::Cyan);
	log << Level::Info << color << "cyan" << std::endl;
	ASSERT_EQUAL("test_manip_color_threadedlog", "\033[36mInfo    : cyan\033[0m\n", output.str());
	RETURN_TEST("test_manip_color_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Group / component
// ---------------------------------------------------------------------------

int test_manip_group_component_reset() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]%g");
	log << reset_component;
	log << component("Module") << group("line") << Level::Info << "first" << std::endl;
	log << Level::Info << "second" << std::endl;
	log << reset_component << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_manip_group_component_reset",
		"Module[Info    ]line first\nModule[Info    ] second\n[Info    ] root\n", output.str());
	RETURN_TEST("test_manip_group_component_reset", 0);
}

int test_manip_group_component_threadedlog() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c[%L]%g");
	log << component("Module") << group("line") << Level::Info << "first" << std::endl;
	ASSERT_EQUAL("test_manip_group_component_threadedlog", "Module[Info    ]line first\n", output.str());
	RETURN_TEST("test_manip_group_component_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Temporary and component formats
// ---------------------------------------------------------------------------

int test_manip_push_pop_and_component_format() {
	std::ostringstream output;
	Log log(output, Level::Info, "GENERAL[%L]");
	log.Format("Module", "COMPONENT[%L]");
	log << component("Module") << push_format("TEMP[%L]") << Level::Info << "temp" << std::endl;
	log << pop_format << Level::Info << "component" << std::endl;
	log << reset_component << Level::Info << "general" << std::endl;
	ASSERT_EQUAL("test_manip_push_pop_and_component_format",
		"TEMP[Info    ] temp\nCOMPONENT[Info    ] component\nGENERAL[Info    ] general\n", output.str());
	RETURN_TEST("test_manip_push_pop_and_component_format", 0);
}

int test_manip_push_pop_threadedlog() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "BASE[%L]");
	log << push_format("TEMP[%L]") << Level::Info << "temp" << std::endl;
	log << pop_format << Level::Info << "base" << std::endl;
	ASSERT_EQUAL("test_manip_push_pop_threadedlog", "TEMP[Info    ] temp\nBASE[Info    ] base\n", output.str());
	RETURN_TEST("test_manip_push_pop_threadedlog", 0);
}

// ---------------------------------------------------------------------------
// Throttle
// ---------------------------------------------------------------------------

int test_manip_throttle_policies_log() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	ThrottleSpec spec;
	spec.Policy = ThrottlePolicy::Window;
	spec.WindowKeep = 1;
	spec.WindowPeriod = 2;
	log.Throttle(spec);
	log << Level::Info << "one" << std::endl;
	log << Level::Info << "two" << std::endl;
	log << Level::Info << "three" << std::endl;
	ASSERT_EQUAL("test_manip_throttle_policies_log",
		"Info    : one\nInfo    : dropped 1 messages\nInfo    : three\n", output.str());
	RETURN_TEST("test_manip_throttle_policies_log", 0);
}

int test_manip_throttle_threadedlog() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	log.Throttle(0.0, 1);
	log << Level::Info << "one" << std::endl;
	log << Level::Info << "two" << std::endl;
	log << Level::Fatal << "fatal" << std::endl;
	ASSERT_EQUAL("test_manip_throttle_threadedlog",
		"Info    : one\nFatal   : fatal\n", output.str());
	RETURN_TEST("test_manip_throttle_threadedlog", 0);
}

int test_manip_throttle_invalid_configuration() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	ThrottleSpec spec;
	spec.Rate = 1.0;
	bool threw = false;
	try {
		log.Throttle(spec);
	} catch (const StormByte::Logger::ThrottleError&) {
		threw = true;
	}

	ASSERT_TRUE("test_manip_throttle_invalid_configuration", threw);
	ASSERT_EQUAL("test_manip_throttle_invalid_configuration (output)", std::string{}, output.str());
	RETURN_TEST("test_manip_throttle_invalid_configuration", 0);
}

int main() {
	int result = 0;
	result += test_manip_humanreadable_number_log();
	result += test_manip_humanreadable_bytes_log();
	result += test_manip_nohumanreadable_log();
	result += test_manip_chainable_threadedlog();
	result += test_manip_redact_full_string();
	result += test_manip_redact_keep_last();
	result += test_manip_redact_keep_last_zero_same_as_full();
	result += test_manip_redact_keep_last_ge_length();
	result += test_manip_redact_empty_string();
	result += test_manip_redact_const_char_ptr();
	result += test_manip_noredact_restores_plain();
	result += test_manip_redact_stays_active();
	result += test_manip_redact_affects_numbers();
	result += test_manip_redact_then_change_keep();
	result += test_manip_redact_threadedlog();
	result += test_manip_redact_with_humanreadable_independent();
	result += test_manip_redact_wstring();
	result += test_manip_redact_first();
	result += test_manip_redact_first_zero_same_as_full();
	result += test_manip_redact_first_ge_length();
	result += test_manip_redact_first_const_char_ptr();
	result += test_manip_redact_first_threadedlog();
	result += test_manip_hex_string();
	result += test_manip_hex_columns_wrap_without_header();
	result += test_manip_hex_zero_is_nohex();
	result += test_manip_nohex_restores_plain();
	result += test_manip_hex_number_uses_text_bytes();
	result += test_manip_hex_then_redact();
	result += test_manip_hex_threadedlog();
	result += test_manip_color_and_nocolor_log();
	result += test_manip_color_threadedlog();
	result += test_manip_group_component_reset();
	result += test_manip_group_component_threadedlog();
	result += test_manip_push_pop_and_component_format();
	result += test_manip_push_pop_threadedlog();
	result += test_manip_throttle_policies_log();
	result += test_manip_throttle_threadedlog();
	result += test_manip_throttle_invalid_configuration();
	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}

	return result;
}
