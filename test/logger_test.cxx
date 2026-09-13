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
#include <StormByte/logger/exception.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/string.hxx>
#include <StormByte/test_handlers.h>
#include <clocale>
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
	std::string expected = "Warning : Warning message\nError   : Error message\n";
	ASSERT_EQUAL("test_log_level_filtering", expected, output.str());
	RETURN_TEST("test_log_level_filtering", 0);
}
int test_log_critical_levels_are_never_filtered() {
	std::ostringstream output;
	Log log(output, Level::Fatal, "%L:");
	log << Level::LowLevel << "hidden low level" << std::endl;
	log << Level::Debug << "hidden debug" << std::endl;
	log << Level::Warning << "visible warning" << std::endl;
	log << Level::Notice << "hidden notice" << std::endl;
	log << Level::Info << "hidden info" << std::endl;
	log << Level::Error << "visible error" << std::endl;
	log << Level::Fatal << "visible fatal" << std::endl;
	const std::string expected =
		"Warning : visible warning\n"
		"Error   : visible error\n"
		"Fatal   : visible fatal\n";
	ASSERT_EQUAL("test_log_critical_levels_are_never_filtered", expected, output.str());
	RETURN_TEST("test_log_critical_levels_are_never_filtered", 0);
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
int test_color_manipulators_and_line_reset() {
	std::ostringstream output;
	Log log(output, Level::LowLevel, "%L:");
	log.Color(Level::Notice, Color::Yellow);
	log << Level::Notice << nocolor << "plain " << color(Color::Green) << "green " << color << "configured" << std::endl;
	log << Level::Notice << "next" << std::endl;
	const std::string expected =
		"\033[33mNotice  : \033[0mplain \033[32mgreen \033[0m\033[33mconfigured\033[0m\n"
		"\033[33mNotice  : next\033[0m\n";
	ASSERT_EQUAL("test_color_manipulators_and_line_reset", expected, output.str());
	RETURN_TEST("test_color_manipulators_and_line_reset", 0);
}
int test_default_color_emits_no_ansi() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << color << "plain" << nocolor << " text" << std::endl;
	ASSERT_EQUAL("test_default_color_emits_no_ansi", "Info    : plain text\n", output.str());
	RETURN_TEST("test_default_color_emits_no_ansi", 0);
}
int test_all_configured_colors_emit_expected_ansi() {
	const std::vector<std::pair<Color, std::string>> colors = {
		{Color::Default, ""},
		{Color::Black, "\033[30m"},
		{Color::Red, "\033[31m"},
		{Color::Green, "\033[32m"},
		{Color::Yellow, "\033[33m"},
		{Color::Blue, "\033[34m"},
		{Color::Magenta, "\033[35m"},
		{Color::Cyan, "\033[36m"},
		{Color::Gray, "\033[90m"},
		{Color::White, "\033[37m"},
		{Color::BrightBlack, "\033[90m"},
		{Color::BrightRed, "\033[91m"},
		{Color::BrightGreen, "\033[92m"},
		{Color::BrightYellow, "\033[93m"},
		{Color::BrightBlue, "\033[94m"},
		{Color::BrightMagenta, "\033[95m"},
		{Color::BrightCyan, "\033[96m"},
		{Color::BrightWhite, "\033[97m"}
	};
	for (const auto& [configured, ansi] : colors) {
		std::ostringstream output;
		Log log(output, Level::Info, "%L:");
		log.Color(Level::Info, configured);
		ASSERT_EQUAL("test_all_configured_colors_emit_expected_ansi (getter)", configured, log.Color(Level::Info));
		log << Level::Info << "value" << std::endl;
		const std::string expected = ansi + "Info    : value" + (ansi.empty() ? "" : "\033[0m") + "\n";
		ASSERT_EQUAL("test_all_configured_colors_emit_expected_ansi", expected, output.str());
	}
	RETURN_TEST("test_all_configured_colors_emit_expected_ansi", 0);
}
int test_filtered_color_has_no_side_effects() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log.Color(Level::Debug, Color::Red);
	log << Level::Debug << color(Color::Green) << "hidden" << std::endl;
	log << Level::Info << "visible" << std::endl;
	ASSERT_EQUAL("test_filtered_color_has_no_side_effects", "Info    : visible\n", output.str());
	RETURN_TEST("test_filtered_color_has_no_side_effects", 0);
}
int test_color_and_temporary_format_interoperate() {
	std::ostringstream output;
	Log log(output, Level::Info, "BASE[%L]");
	log.Color(Level::Info, Color::Blue);
	log << push_format("TEMP[%L]") << Level::Info << nocolor << "plain" << color << " blue" << std::endl;
	log << pop_format << Level::Info << "base" << std::endl;
	const std::string expected =
		"\033[34mTEMP[Info    ] \033[0mplain\033[34m blue\033[0m\n"
		"\033[34mBASE[Info    ] base\033[0m\n";
	ASSERT_EQUAL("test_color_and_temporary_format_interoperate", expected, output.str());
	RETURN_TEST("test_color_and_temporary_format_interoperate", 0);
}
int test_colored_logger_destructor_resets_stream() {
	std::ostringstream output;
	{
		Log log(output, Level::Info, "%L:");
		log.Color(Level::Info, Color::Red);
		log << Level::Info << "unterminated";
	}
	ASSERT_EQUAL("test_colored_logger_destructor_resets_stream", "\033[31mInfo    : unterminated\033[0m", output.str());
	RETURN_TEST("test_colored_logger_destructor_resets_stream", 0);
}
int test_push_pop_format_stack() {
	std::ostringstream output;
	Log log(output, Level::Info, "BASE[%L]");
	log << Level::Info << "base" << std::endl;
	log << push_format("TEMP[%L]") << Level::Info << "temp" << std::endl;
	log << push_format("INNER[%L]") << Level::Info << "inner" << std::endl;
	log << pop_format << Level::Info << "temp again" << std::endl;
	log << pop_format << Level::Info << "base again" << std::endl;
	log << pop_format << Level::Info << "still base" << std::endl;
	const std::string expected =
		"BASE[Info    ] base\n"
		"TEMP[Info    ] temp\n"
		"INNER[Info    ] inner\n"
		"TEMP[Info    ] temp again\n"
		"BASE[Info    ] base again\n"
		"BASE[Info    ] still base\n";
	ASSERT_EQUAL("test_push_pop_format_stack", expected, output.str());
	RETURN_TEST("test_push_pop_format_stack", 0);
}
int test_push_format_empty_and_partial_line_reset() {
	std::ostringstream output;
	Log log(output, Level::Info, "BASE[%L]");
	log << Level::Info << "before" << push_format("NEXT[%L]") << "after" << std::endl;
	log << push_format("") << Level::Info << "empty" << std::endl;
	log << pop_format << Level::Info << "next" << std::endl;
	const std::string expected =
		"BASE[Info    ] before\n"
		"NEXT[Info    ] after\n"
		" empty\n"
		"NEXT[Info    ] next\n";
	ASSERT_EQUAL("test_push_format_empty_and_partial_line_reset", expected, output.str());
	RETURN_TEST("test_push_format_empty_and_partial_line_reset", 0);
}
int test_group_header_and_line_reset() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:%g");
	log << group("Decoder") << Level::Info << "open" << std::endl;
	log << Level::Info << "plain" << std::endl;
	ASSERT_EQUAL("test_group_header_and_line_reset", "Info    :Decoder open\nInfo    : plain\n", output.str());
	RETURN_TEST("test_group_header_and_line_reset", 0);
}
int test_group_without_token_and_empty_group() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << group("Decoder") << Level::Info << "open" << std::endl;
	log << group("") << Level::Info << "plain" << std::endl;
	ASSERT_EQUAL("test_group_without_token_and_empty_group", "Info    : open\nInfo    : plain\n", output.str());
	RETURN_TEST("test_group_without_token_and_empty_group", 0);
}
int test_group_change_closes_partial_line() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:%g");
	log << group("Decoder") << Level::Info << "before";
	log << group("Encoder") << "after" << std::endl;
	ASSERT_EQUAL("test_group_change_closes_partial_line", "Info    :Decoder before\nInfo    :Encoder after\n", output.str());
	RETURN_TEST("test_group_change_closes_partial_line", 0);
}
int test_filtered_group_has_no_side_effects() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:%g");
	log << group("Hidden") << Level::Debug << "hidden" << std::endl;
	log << Level::Info << "visible" << std::endl;
	ASSERT_EQUAL("test_filtered_group_has_no_side_effects", "Info    : visible\n", output.str());
	RETURN_TEST("test_filtered_group_has_no_side_effects", 0);
}
int test_group_and_color_share_the_header() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:%g");
	log.Color(Level::Info, Color::Yellow);
	log << group("Decoder") << Level::Info << "open" << std::endl;
	ASSERT_EQUAL("test_group_and_color_share_the_header", "\033[33mInfo    :Decoder open\033[0m\n", output.str());
	RETURN_TEST("test_group_and_color_share_the_header", 0);
}
int test_component_header_is_sticky_and_resettable() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]");
	log << component("Multimedia") << Level::Info << "first" << std::endl;
	log << Level::Info << "second" << std::endl;
	log << reset_component << Level::Info << "third" << std::endl;
	ASSERT_EQUAL("test_component_header_is_sticky_and_resettable",
		"Multimedia[Info    ] first\nMultimedia[Info    ] second\n[Info    ] third\n", output.str());
	RETURN_TEST("test_component_header_is_sticky_and_resettable", 0);
}
int test_component_without_token_preserves_legacy_output() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << component("Hidden") << Level::Info << "message" << std::endl;
	ASSERT_EQUAL("test_component_without_token_preserves_legacy_output", "Info    : message\n", output.str());
	RETURN_TEST("test_component_without_token_preserves_legacy_output", 0);
}
int test_component_color_override_has_priority() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]");
	log.Color(Level::Info, Color::Blue);
	log.Color("Multimedia", Level::Info, Color::Red);
	log << component("Multimedia") << Level::Info << "red" << std::endl;
	log << reset_component << Level::Info << "blue" << std::endl;
	ASSERT_EQUAL("test_component_color_override_has_priority",
		"\033[31mMultimedia[Info    ] red\033[0m\n\033[34m[Info    ] blue\033[0m\n", output.str());
	RETURN_TEST("test_component_color_override_has_priority", 0);
}
int test_empty_component_selects_root() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]");
	log << component("Multimedia") << Level::Info << "named" << std::endl;
	log << component("") << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_empty_component_selects_root", "Multimedia[Info    ] named\n[Info    ] root\n", output.str());
	RETURN_TEST("test_empty_component_selects_root", 0);
}
int test_component_format_priority_and_fallback() {
	std::ostringstream output;
	Log log(output, Level::Info, "GENERAL[%L]");
	log.Format("Media", "MEDIA[%L]");
	log << component("Media") << Level::Info << "media" << std::endl;
	log << component("Other") << Level::Info << "fallback" << std::endl;
	log.Format("Media", "");
	log << component("Media") << Level::Info << "general again" << std::endl;
	const std::string expected =
		"MEDIA[Info    ] media\n"
		"GENERAL[Info    ] fallback\n"
		"GENERAL[Info    ] general again\n";
	ASSERT_EQUAL("test_component_format_priority_and_fallback", expected, output.str());
	RETURN_TEST("test_component_format_priority_and_fallback", 0);
}
int test_push_format_overrides_component_and_restores_resolution() {
	std::ostringstream output;
	Log log(output, Level::Info, "GENERAL[%L]");
	log.Format("Media", "MEDIA[%L]");
	log << component("Media") << push_format("TEMP[%L]") << Level::Info << "temporary" << std::endl;
	log << pop_format << Level::Info << "component again" << std::endl;
	log << component("Other") << push_format("TEMP[%L]") << Level::Info << "other temporary" << std::endl;
	log << pop_format;
	log << component("Media") << Level::Info << "media after component switch" << std::endl;
	const std::string expected =
		"TEMP[Info    ] temporary\n"
		"MEDIA[Info    ] component again\n"
		"TEMP[Info    ] other temporary\n"
		"MEDIA[Info    ] media after component switch\n";
	ASSERT_EQUAL("test_push_format_overrides_component_and_restores_resolution", expected, output.str());
	RETURN_TEST("test_push_format_overrides_component_and_restores_resolution", 0);
}
int test_format_change_redecides_throttle_line() {
	std::ostringstream output;
	Log log(output, Level::Info, "A[%L]");
	log.Throttle(0.0, 1);
	log << Level::Info << "first";
	log.Format("B[%L]");
	log << Level::Info << "second" << std::endl;
	ASSERT_EQUAL("test_format_change_redecides_throttle_line", "A[Info    ] first\n", output.str());
	RETURN_TEST("test_format_change_redecides_throttle_line", 0);
}
int test_wide_string_logging_is_locale_independent() {
	int result = 0;
	const char* current_locale = std::setlocale(LC_ALL, nullptr);
	const std::string saved_locale = current_locale == nullptr ? "C" : current_locale;
	std::setlocale(LC_ALL, "C");
	try {
		std::ostringstream output;
		Log log(output, Level::Info, "%L:");
		log << Level::Info << std::wstring{L"caf\u00e9 \U0001F600"} << std::endl;
		ASSERT_EQUAL("test_wide_string_logging_is_locale_independent", "Info    : caf\xC3\xA9 \xF0\x9F\x98\x80\n", output.str());
	} catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		result++;
	}
	std::setlocale(LC_ALL, saved_locale.c_str());
	RETURN_TEST("test_wide_string_logging_is_locale_independent", result);
}
int test_invalid_wide_string_propagates_without_termination() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	bool threw = false;
	try {
		log << Level::Info << std::wstring(1, static_cast<wchar_t>(0xD800));
	} catch (const StormByte::UTF8Error&) {
		threw = true;
	}
	ASSERT_TRUE("test_invalid_wide_string_propagates_without_termination", threw);
	log << Level::Info << "after invalid input" << std::endl;
	ASSERT_EQUAL("test_invalid_wide_string_propagates_without_termination", "Info    : after invalid input\n", output.str());
	RETURN_TEST("test_invalid_wide_string_propagates_without_termination", 0);
}
int test_throttle_off_preserves_output() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log << Level::Info << "one" << std::endl;
	log << Level::Warning << "two" << std::endl;
	ASSERT_EQUAL("test_throttle_off_preserves_output", "Info    : one\nWarning : two\n", output.str());
	RETURN_TEST("test_throttle_off_preserves_output", 0);
}
int test_throttle_drop_burst() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	log.Throttle(0.0, 2);
	for (int index = 0; index < 5; ++index)
		log << Level::Info << index << std::endl;
	ASSERT_EQUAL("test_throttle_drop_burst", "Info    : 0\nInfo    : 1\n", output.str());
	RETURN_TEST("test_throttle_drop_burst", 0);
}
int test_throttle_sample_and_window() {
	std::ostringstream sample_output;
	Log sample(sample_output, Level::Info, "%L:");
	ThrottleSpec sample_spec;
	sample_spec.Policy = ThrottlePolicy::Sample;
	sample_spec.SampleN = 10;
	sample.Throttle(sample_spec);
	for (int index = 0; index < 20; ++index)
		sample << Level::Info << index << std::endl;
	ASSERT_EQUAL("test_throttle_sample_and_window (sample)",
		"Info    : 0\nInfo    : dropped 9 messages\nInfo    : 10\n", sample_output.str());

	std::ostringstream window_output;
	Log window(window_output, Level::Info, "%L:");
	ThrottleSpec window_spec;
	window_spec.Policy = ThrottlePolicy::Window;
	window_spec.WindowKeep = 2;
	window_spec.WindowPeriod = 5;
	window.Throttle(window_spec);
	for (int index = 0; index < 6; ++index)
		window << Level::Info << index << std::endl;
	ASSERT_EQUAL("test_throttle_sample_and_window (window)",
		"Info    : 0\nInfo    : 1\nInfo    : dropped 3 messages\nInfo    : 5\n", window_output.str());
	RETURN_TEST("test_throttle_sample_and_window", 0);
}
int test_throttle_precedence_and_no_throttle() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]%g:");
	ThrottleSpec global;
	global.Burst = 10;
	log.Throttle(global);
	ThrottleSpec component_rule = global;
	component_rule.Component = "A";
	component_rule.Burst = 2;
	log.Throttle(component_rule);
	ThrottleSpec other_rule = global;
	other_rule.Component = "B";
	other_rule.Burst = 1;
	log.Throttle(other_rule);
	log << component("A") << group("x") << Level::Info << "a0" << std::endl;
	log << Level::Info << "a1" << std::endl;
	log << Level::Info << "a2" << std::endl;
	log << component("B") << group("x") << Level::Info << "b0" << std::endl;
	log << Level::Info << "b1" << std::endl;
	log.NoThrottle(component("B"));
	log << Level::Info << "b2" << std::endl;
	const std::string out = output.str();
	ASSERT_TRUE("test_throttle_precedence_and_no_throttle (component)", out.find("A[Info    ]x: a0") != std::string::npos);
	ASSERT_TRUE("test_throttle_precedence_and_no_throttle (root)", out.find("B[Info    ]x: b0") != std::string::npos);
	ASSERT_EQUAL("test_throttle_precedence_and_no_throttle (reset)", std::string("B[Info    ]: b2\n"), out.substr(out.rfind("B[Info    ]:")));
	RETURN_TEST("test_throttle_precedence_and_no_throttle", 0);
}
int test_throttle_warning_but_not_error_or_fatal() {
	std::ostringstream output;
	Log log(output, Level::Fatal, "%L:");
	ThrottleSpec spec;
	spec.Burst = 1;
	log.Throttle(spec);
	log << Level::Warning << "warning 1" << std::endl;
	log << Level::Warning << "warning 2" << std::endl;
	log << Level::Error << "error" << std::endl;
	log << Level::Fatal << "fatal" << std::endl;
	ASSERT_EQUAL("test_throttle_warning_but_not_error_or_fatal",
		"Warning : warning 1\nError   : error\nFatal   : fatal\n", output.str());
	RETURN_TEST("test_throttle_warning_but_not_error_or_fatal", 0);
}
int test_throttle_summary_preserves_context() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]%g ");
	ThrottleSpec spec;
	spec.Component = "A";
	spec.Level = Level::Info;
	spec.Group = "g";
	spec.Policy = ThrottlePolicy::Window;
	spec.WindowKeep = 1;
	spec.WindowPeriod = 2;
	log.Throttle(spec);
	log << component("A") << group("g") << Level::Info << "first" << std::endl;
	log << group("g") << Level::Info << "dropped" << std::endl;
	log << group("g") << Level::Info << "third" << std::endl;
	ASSERT_EQUAL("test_throttle_summary_preserves_context",
		"A[Info    ]g  first\nA[Info    ]g  dropped 1 messages\nA[Info    ]g  third\n", output.str());
	RETURN_TEST("test_throttle_summary_preserves_context", 0);
}
int test_throttle_component_and_group_isolation() {
	std::ostringstream output;
	Log log(output, Level::Info, "%c[%L]%g ");
	ThrottleSpec component_rule;
	component_rule.Component = "A";
	component_rule.Burst = 1;
	log.Throttle(component_rule);
	log << component("A") << Level::Info << "a0" << std::endl;
	log << component("A") << Level::Info << "a1" << std::endl;
	log << component("B") << Level::Info << "b0" << std::endl;
	log << Level::Info << "b1" << std::endl;
	ThrottleSpec group_rule;
	group_rule.Group = "x";
	group_rule.Burst = 1;
	log.Throttle(group_rule);
	log << group("x") << Level::Info << "x0" << std::endl;
	log << group("x") << Level::Info << "x1" << std::endl;
	log << group("y") << Level::Info << "y0" << std::endl;
	ASSERT_EQUAL("test_throttle_component_and_group_isolation", std::string("A[Info    ]  a0\nB[Info    ]  b0\nB[Info    ]  b1\nB[Info    ]x  x0\nB[Info    ]y  y0\n"), output.str());
	RETURN_TEST("test_throttle_component_and_group_isolation", 0);
}
int test_throttle_empty_lines_are_counted() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	ThrottleSpec spec;
	spec.Policy = ThrottlePolicy::Window;
	spec.WindowKeep = 1;
	spec.WindowPeriod = 2;
	log.Throttle(spec);
	log << Level::Info << std::endl;
	log << Level::Info << std::endl;
	log << Level::Info << std::endl;
	ASSERT_EQUAL("test_throttle_empty_lines_are_counted",
		"\nInfo    : dropped 1 messages\n\n", output.str());
	RETURN_TEST("test_throttle_empty_lines_are_counted", 0);
}
int test_throttle_level_change_redecides_line() {
	std::ostringstream output;
	Log log(output, Level::Info, "%L:");
	ThrottleSpec spec;
	spec.Level = Level::Info;
	spec.Burst = 1;
	log.Throttle(spec);
	log << Level::Info << "first";
	log << Level::Warning << "second" << std::endl;
	ASSERT_EQUAL("test_throttle_level_change_redecides_line", "Info    : first\nWarning : second\n", output.str());
	RETURN_TEST("test_throttle_level_change_redecides_line", 0);
}
int test_throttle_rejects_invalid_specs() {
	Log log(std::cout, Level::Info);
	ThrottleSpec spec;
	spec.Rate = -1.0;
	bool threw = false;
	try { log.Throttle(spec); } catch (const StormByte::Logger::ThrottleError& ex) {
		threw = true;
		ASSERT_TRUE("test_throttle_rejects_invalid_specs (component)", std::string(ex.what()).find("StormByte::Logger:") == 0);
	}
	ASSERT_TRUE("test_throttle_rejects_invalid_specs (negative rate)", threw);
	spec = {};
	spec.Rate = 1.0;
	try { log.Throttle(spec); } catch (const StormByte::Logger::ThrottleError& ex) {
		threw = true;
		ASSERT_TRUE("test_throttle_rejects_invalid_specs (component)", std::string(ex.what()).find("StormByte::Logger:") == 0);
	}
	ASSERT_TRUE("test_throttle_rejects_invalid_specs (zero burst)", threw);
	spec = {};
	spec.Rate = 100.0;
	spec.Burst = 1;
	threw = false;
	try { log.Throttle(spec); } catch (const StormByte::Exception&) { threw = true; }
	ASSERT_TRUE("test_throttle_rejects_invalid_specs (valid rate)", !threw);
	spec = {};
	spec.Policy = ThrottlePolicy::Sample;
	spec.SampleN = 1;
	threw = false;
	try { log.Throttle(spec); } catch (const StormByte::Exception&) { threw = true; }
	ASSERT_TRUE("test_throttle_rejects_invalid_specs (sample)", threw);
	spec = {};
	spec.Policy = ThrottlePolicy::Window;
	spec.WindowKeep = 2;
	spec.WindowPeriod = 1;
	threw = false;
	try { log.Throttle(spec); } catch (const StormByte::Exception&) { threw = true; }
	ASSERT_TRUE("test_throttle_rejects_invalid_specs (window)", threw);
	return 0;
}
int main() {
	int result = 0;
	result += test_basic_logging();
	result += test_log_level_filtering();
	result += test_log_critical_levels_are_never_filtered();
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
	result += test_color_manipulators_and_line_reset();
	result += test_default_color_emits_no_ansi();
	result += test_all_configured_colors_emit_expected_ansi();
	result += test_filtered_color_has_no_side_effects();
	result += test_color_and_temporary_format_interoperate();
	result += test_colored_logger_destructor_resets_stream();
	result += test_push_pop_format_stack();
	result += test_push_format_empty_and_partial_line_reset();
	result += test_group_header_and_line_reset();
	result += test_group_without_token_and_empty_group();
	result += test_group_change_closes_partial_line();
	result += test_filtered_group_has_no_side_effects();
	result += test_group_and_color_share_the_header();
	result += test_component_header_is_sticky_and_resettable();
	result += test_component_without_token_preserves_legacy_output();
	result += test_component_color_override_has_priority();
	result += test_empty_component_selects_root();
	result += test_component_format_priority_and_fallback();
	result += test_push_format_overrides_component_and_restores_resolution();
	result += test_format_change_redecides_throttle_line();
	result += test_wide_string_logging_is_locale_independent();
	result += test_invalid_wide_string_propagates_without_termination();
	result += test_throttle_off_preserves_output();
	result += test_throttle_drop_burst();
	result += test_throttle_sample_and_window();
	result += test_throttle_precedence_and_no_throttle();
	result += test_throttle_warning_but_not_error_or_fatal();
	result += test_throttle_summary_preserves_context();
	result += test_throttle_component_and_group_isolation();
	result += test_throttle_empty_lines_are_counted();
	result += test_throttle_level_change_redecides_line();
	result += test_throttle_rejects_invalid_specs();
	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}
	return result;
}
