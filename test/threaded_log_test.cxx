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

#include <StormByte/base64.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/string.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <chrono>
#include <future>
#include <regex>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>

using namespace StormByte::Logger;

namespace {
	void IsolateLine(Log& log) {
		if (!log.Enabled(Level::LowLevel))
			log << Level::LowLevel << std::endl;
	}
}

// ---------------------------------------------------------------------------
// Basic emit
// ---------------------------------------------------------------------------

int test_threadedlog_basic() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	tlog << Level::Info << "Threaded basic message" << std::endl;
	std::string expected = "Info    : Threaded basic message\n";
	ASSERT_EQUAL("test_threadedlog_basic", expected, output.str());
	RETURN_TEST("test_threadedlog_basic", 0);
}

int test_smart_pointer_usage() {
	std::ostringstream output;
	std::shared_ptr<StormByte::Logger::Log> log = std::make_shared<StormByte::Logger::ThreadedLog>(output, Level::Info, "%L:");
	log << Level::Info << "Smart pointer log message" << std::endl;
	std::string expected = "Info    : Smart pointer log message\n";
	ASSERT_EQUAL("test_smart_pointer_usage", expected, output.str());
	RETURN_TEST("test_smart_pointer_usage", 0);
}

// ---------------------------------------------------------------------------
// Floor / Enabled / views
// ---------------------------------------------------------------------------

int test_threadedlog_critical_levels_are_never_filtered() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Fatal, "%L:");
	tlog << Level::LowLevel << "hidden low level" << std::endl;
	tlog << Level::Debug << "hidden debug" << std::endl;
	tlog << Level::Warning << "visible warning" << std::endl;
	tlog << Level::Notice << "hidden notice" << std::endl;
	tlog << Level::Info << "hidden info" << std::endl;
	tlog << Level::Error << "visible error" << std::endl;
	tlog << Level::Fatal << "visible fatal" << std::endl;
	const std::string expected =
		"Warning : visible warning\n"
		"Error   : visible error\n"
		"Fatal   : visible fatal\n";
	ASSERT_EQUAL("test_threadedlog_critical_levels_are_never_filtered", expected, output.str());
	RETURN_TEST("test_threadedlog_critical_levels_are_never_filtered", 0);
}

int test_threadedlog_enabled_and_views() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	ASSERT_TRUE("test_threadedlog_enabled_and_views (Info)", log.Enabled(Level::Info));
	ASSERT_TRUE("test_threadedlog_enabled_and_views (Warning)", log.Enabled(Level::Warning));
	ASSERT_FALSE("test_threadedlog_enabled_and_views (Debug)", log.Enabled(Level::Debug));

	const std::string owned = "owned";
	const std::wstring wowned = L"wide";
	log << Level::Info << std::string_view{owned} << " " << std::wstring_view{wowned} << std::endl;
	log << Level::Debug << std::string_view{owned} << std::endl;
	ASSERT_EQUAL("test_threadedlog_enabled_and_views", "Info    : owned wide\n", output.str());
	RETURN_TEST("test_threadedlog_enabled_and_views", 0);
}

// ---------------------------------------------------------------------------
// Binary span: encode before the line lock
// ---------------------------------------------------------------------------

int test_threadedlog_span_default_is_base64() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	const std::vector<std::byte> raw{
		std::byte{'H'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'}, std::byte{'o'}
	};
	log << Level::Info << std::span<const std::byte>{raw} << std::endl;
	ASSERT_EQUAL("test_threadedlog_span_default_is_base64",
		"Info    : " + StormByte::Base64Encode(raw) + "\n", output.str());
	RETURN_TEST("test_threadedlog_span_default_is_base64", 0);
}

int test_threadedlog_span_vector_converts() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	const std::vector<std::byte> raw{std::byte{0x01}, std::byte{0x02}};
	log << Level::Info << raw << std::endl;
	ASSERT_EQUAL("test_threadedlog_span_vector_converts",
		"Info    : " + StormByte::Base64Encode(raw) + "\n", output.str());
	RETURN_TEST("test_threadedlog_span_vector_converts", 0);
}

int test_threadedlog_span_hex() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	const std::vector<std::byte> raw{std::byte{0x01}, std::byte{0xAB}};
	log << Level::Info << hex << raw << std::endl;
	ASSERT_EQUAL("test_threadedlog_span_hex", "Info    : 0x01 0xAB\n", output.str());
	RETURN_TEST("test_threadedlog_span_hex", 0);
}

int test_threadedlog_span_filtered() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	const std::vector<std::byte> raw{std::byte{0xFF}};
	log << Level::Debug << raw << std::endl;
	ASSERT_EQUAL("test_threadedlog_span_filtered", std::string{}, output.str());
	log << Level::Info << "after" << std::endl;
	ASSERT_EQUAL("test_threadedlog_span_filtered (after)", "Info    : after\n", output.str());
	RETURN_TEST("test_threadedlog_span_filtered", 0);
}

// ---------------------------------------------------------------------------
// Line lock / concurrency
// ---------------------------------------------------------------------------

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
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	const int threads = 4;
	const int parts = 10;
	auto worker = [&](int id) {
		for (int i = 0; i < parts; ++i) {
			tlog << Level::Info << "p" << id << ":" << i << " ";
		}

		tlog << std::endl;
	};
	std::vector<std::thread> pool;
	for (int t = 0; t < threads; ++t) pool.emplace_back(worker, t);
	for (auto &th : pool) th.join();
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
			start_futures[i].get();
			tlog << Level::Info << "T" << i << std::endl;
			done_promises[i].set_value();
		});
	}

	for (int i = 0; i < threads; ++i) {
		start_promises[i].set_value();
		done_futures[i].get();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	for (auto &th : pool) th.join();
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

int test_threadedlog_level_switch_flush() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Debug, "%L:");
	tlog << Level::Info << "part1";
	tlog << Level::Debug << "part2" << std::endl;
	std::string out = output.str();
	if (out.find("part1") == std::string::npos || out.find("part2") == std::string::npos) {
		ASSERT_EQUAL("test_threadedlog_level_switch_flush", std::string("contains part1 and part2"), out);
		RETURN_TEST("test_threadedlog_level_switch_flush", 1);
	}

	RETURN_TEST("test_threadedlog_level_switch_flush", 0);
}

// ---------------------------------------------------------------------------
// Filtered path must not leak the lock
// ---------------------------------------------------------------------------

int test_threadedlog_filtered_endl_no_deadlock() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	for (int i = 0; i < 50; ++i) {
		tlog << Level::Debug << "hidden " << i << std::endl;
	}

	tlog << Level::Info << "after filtered" << std::endl;
	std::string expected = "Info    : after filtered\n";
	ASSERT_EQUAL("test_threadedlog_filtered_endl_no_deadlock", expected, output.str());
	RETURN_TEST("test_threadedlog_filtered_endl_no_deadlock", 0);
}

int test_threadedlog_filtered_multithreaded_then_info() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	const int threads = 4;
	const int repeats = 40;
	std::atomic<int> done{0};
	auto worker = [&](int id) {
		for (int i = 0; i < repeats; ++i) {
			tlog << Level::Debug << "d" << id << ":" << i << std::endl;
		}

		done.fetch_add(1);
	};
	std::vector<std::thread> pool;
	for (int t = 0; t < threads; ++t) pool.emplace_back(worker, t);
	for (auto &th : pool) th.join();
	if (done.load() != threads) {
		ASSERT_EQUAL("test_threadedlog_filtered_multithreaded_then_info (workers)", std::to_string(threads), std::to_string(done.load()));
		RETURN_TEST("test_threadedlog_filtered_multithreaded_then_info", 1);
	}

	tlog << Level::Info << "ok" << std::endl;
	std::string expected = "Info    : ok\n";
	ASSERT_EQUAL("test_threadedlog_filtered_multithreaded_then_info", expected, output.str());
	RETURN_TEST("test_threadedlog_filtered_multithreaded_then_info", 0);
}

int test_threadedlog_filtered_hot_path() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	constexpr int threads = 8;
	constexpr int repeats = 5000;
	std::atomic<int> completed{0};
	auto worker = [&](int id) {
		for (int i = 0; i < repeats; ++i) {
			tlog << Level::Debug << "discarded-" << id << ':' << i << std::endl;
		}

		completed.fetch_add(1, std::memory_order_release);
	};
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) pool.emplace_back(worker, id);
	for (auto& thread : pool) thread.join();
	ASSERT_EQUAL("test_threadedlog_filtered_hot_path (workers)", threads, completed.load(std::memory_order_acquire));
	ASSERT_EQUAL("test_threadedlog_filtered_hot_path (no output)", std::string{}, output.str());
	tlog << Level::Info << "after filtered hot path" << std::endl;
	ASSERT_EQUAL("test_threadedlog_filtered_hot_path", "Info    : after filtered hot path\n", output.str());
	RETURN_TEST("test_threadedlog_filtered_hot_path", 0);
}

// ---------------------------------------------------------------------------
// Wide / UTF-8
// ---------------------------------------------------------------------------

int test_threadedlog_invalid_wide_releases_line_lock() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	bool threw = false;
	try {
		tlog << Level::Info << std::wstring(1, static_cast<wchar_t>(0xD800));
	} catch (const StormByte::UTF8Error&) {
		threw = true;
	}

	ASSERT_TRUE("test_threadedlog_invalid_wide_releases_line_lock (throws)", threw);
	tlog << Level::Info << "after invalid input" << std::endl;
	ASSERT_EQUAL("test_threadedlog_invalid_wide_releases_line_lock", "Info    : after invalid input\n", output.str());
	RETURN_TEST("test_threadedlog_invalid_wide_releases_line_lock", 0);
}

int test_threadedlog_filtered_wide_skips_conversion() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	bool threw = false;
	try {
		tlog << Level::Debug << std::wstring(1, static_cast<wchar_t>(0xD800)) << std::endl;
	} catch (const StormByte::UTF8Error&) {
		threw = true;
	}

	ASSERT_TRUE("test_threadedlog_filtered_wide_skips_conversion (no throw)", !threw);
	ASSERT_EQUAL("test_threadedlog_filtered_wide_skips_conversion (no output)", std::string{}, output.str());
	tlog << Level::Info << "after filtered invalid input" << std::endl;
	ASSERT_EQUAL("test_threadedlog_filtered_wide_skips_conversion", "Info    : after filtered invalid input\n", output.str());
	RETURN_TEST("test_threadedlog_filtered_wide_skips_conversion", 0);
}

// ---------------------------------------------------------------------------
// Color / format / group
// ---------------------------------------------------------------------------

int test_threadedlog_colored_lines_do_not_mix() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%L:");
	tlog.Color(Level::Info, Color::Cyan);
	constexpr int threads = 8;
	constexpr int repeats = 250;
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) {
		pool.emplace_back([&, id] {
			for (int i = 0; i < repeats; ++i)
				tlog << Level::Info << "T" << id << ':' << i << std::endl;
		});
	}

	for (auto& thread : pool) thread.join();
	std::istringstream input(output.str());
	std::string line;
	int count = 0;
	while (std::getline(input, line)) {
		ASSERT_TRUE("test_threadedlog_colored_lines_do_not_mix (prefix)", line.starts_with("\033[36mInfo    : T"));
		ASSERT_TRUE("test_threadedlog_colored_lines_do_not_mix (reset)", line.ends_with("\033[0m"));
		ASSERT_EQUAL("test_threadedlog_colored_lines_do_not_mix (single color)", std::size_t{1},
			static_cast<std::size_t>(line.find("\033[36m") != std::string::npos));
		++count;
	}

	ASSERT_EQUAL("test_threadedlog_colored_lines_do_not_mix (count)", threads * repeats, count);
	RETURN_TEST("test_threadedlog_colored_lines_do_not_mix", 0);
}

int test_threadedlog_push_pop_format_is_line_safe() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "BASE[%L]");
	constexpr int threads = 4;
	constexpr int repeats = 100;
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) {
		pool.emplace_back([&, id] {
			for (int i = 0; i < repeats; ++i) {
				tlog << push_format("T" + std::to_string(id) + "[%L]")
					<< Level::Info << "message" << std::endl
					<< pop_format;
			}
		});
	}

	for (auto& thread : pool) thread.join();
	std::istringstream input(output.str());
	std::string line;
	int count = 0;
	while (std::getline(input, line)) {
		ASSERT_TRUE("test_threadedlog_push_pop_format_is_line_safe (format)",
			std::regex_match(line, std::regex("^T[0-3]\\[Info    \\] message$")));
		++count;
	}

	ASSERT_EQUAL("test_threadedlog_push_pop_format_is_line_safe (count)", threads * repeats, count);
	tlog << pop_format;
	tlog << Level::Info << "after empty pop" << std::endl;
	ASSERT_TRUE("test_threadedlog_push_pop_format_is_line_safe (restored base)",
		output.str().ends_with("BASE[Info    ] after empty pop\n"));
	RETURN_TEST("test_threadedlog_push_pop_format_is_line_safe", 0);
}

int test_threadedlog_groups_do_not_mix() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%g[%L]");
	constexpr int threads = 4;
	constexpr int repeats = 200;
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) {
		pool.emplace_back([&, id] {
			for (int i = 0; i < repeats; ++i)
				tlog << group("G" + std::to_string(id)) << Level::Info << "message" << std::endl;
		});
	}

	for (auto& thread : pool) thread.join();
	std::istringstream input(output.str());
	std::string line;
	int count = 0;
	while (std::getline(input, line)) {
		ASSERT_TRUE("test_threadedlog_groups_do_not_mix (line)",
			std::regex_match(line, std::regex("^G[0-3]\\[Info    \\] message$")));
		++count;
	}

	ASSERT_EQUAL("test_threadedlog_groups_do_not_mix (count)", threads * repeats, count);
	RETURN_TEST("test_threadedlog_groups_do_not_mix", 0);
}

int test_threadedlog_filtered_group_releases_lock() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%g[%L]");
	tlog << group("Hidden") << Level::Debug << "hidden" << std::endl;
	tlog << Level::Info << "visible" << std::endl;
	ASSERT_EQUAL("test_threadedlog_filtered_group_releases_lock", "[Info    ] visible\n", output.str());
	RETURN_TEST("test_threadedlog_filtered_group_releases_lock", 0);
}

// ---------------------------------------------------------------------------
// Components
// ---------------------------------------------------------------------------

int test_threadedlog_component_header_is_sticky_and_resettable() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c[%L]");
	log << reset_component;
	IsolateLine(log);
	log << component("Multimedia") << Level::Info << "first" << std::endl;
	log << Level::Info << "second" << std::endl;
	log << reset_component << Level::Info << "third" << std::endl;
	ASSERT_EQUAL("test_threadedlog_component_header_is_sticky_and_resettable",
		"Multimedia[Info    ] first\nMultimedia[Info    ] second\n[Info    ] third\n", output.str());
	RETURN_TEST("test_threadedlog_component_header_is_sticky_and_resettable", 0);
}

int test_threadedlog_component_without_token_preserves_legacy_output() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	log << reset_component;
	IsolateLine(log);
	log << component("Hidden") << Level::Info << "message" << std::endl;
	ASSERT_EQUAL("test_threadedlog_component_without_token_preserves_legacy_output", "Info    : message\n", output.str());
	RETURN_TEST("test_threadedlog_component_without_token_preserves_legacy_output", 0);
}

int test_threadedlog_component_color_override_has_priority() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c[%L]");
	log << reset_component;
	IsolateLine(log);
	log.Color(Level::Info, Color::Blue);
	log.Color("Multimedia", Level::Info, Color::Red);
	log << component("Multimedia") << Level::Info << "red" << std::endl;
	log << reset_component << Level::Info << "blue" << std::endl;
	ASSERT_EQUAL("test_threadedlog_component_color_override_has_priority",
		"\033[31mMultimedia[Info    ] red\033[0m\n\033[34m[Info    ] blue\033[0m\n", output.str());
	RETURN_TEST("test_threadedlog_component_color_override_has_priority", 0);
}

int test_threadedlog_empty_component_does_not_push() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c[%L]");
	log << reset_component;
	IsolateLine(log);
	log << component("Multimedia") << Level::Info << "named" << std::endl;
	log << component("") << Level::Info << "still named" << std::endl;
	log << reset_component << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_threadedlog_empty_component_does_not_push",
		"Multimedia[Info    ] named\nMultimedia[Info    ] still named\n[Info    ] root\n", output.str());
	RETURN_TEST("test_threadedlog_empty_component_does_not_push", 0);
}

int test_threadedlog_component_format_priority_and_fallback() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "GENERAL[%L]");
	log << reset_component;
	IsolateLine(log);
	log.Format("Media", "MEDIA[%L]");
	log << component("Media") << Level::Info << "media" << std::endl;
	log << reset_component << component("Other") << Level::Info << "fallback" << std::endl;
	log.Format("Media", "");
	log << reset_component << component("Media") << Level::Info << "general again" << std::endl;
	const std::string expected =
		"MEDIA[Info    ] media\n"
		"GENERAL[Info    ] fallback\n"
		"GENERAL[Info    ] general again\n";
	ASSERT_EQUAL("test_threadedlog_component_format_priority_and_fallback", expected, output.str());
	RETURN_TEST("test_threadedlog_component_format_priority_and_fallback", 0);
}

int test_threadedlog_push_format_overrides_component_and_restores_resolution() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "GENERAL[%L]");
	log << reset_component;
	IsolateLine(log);
	log.Format("Media", "MEDIA[%L]");
	log << component("Media") << push_format("TEMP[%L]") << Level::Info << "temporary" << std::endl;
	log << pop_format << Level::Info << "component again" << std::endl;
	log << reset_component << component("Other") << push_format("TEMP[%L]") << Level::Info << "other temporary" << std::endl;
	log << pop_format;
	log << reset_component << component("Media") << Level::Info << "media after component switch" << std::endl;
	const std::string expected =
		"TEMP[Info    ] temporary\n"
		"MEDIA[Info    ] component again\n"
		"TEMP[Info    ] other temporary\n"
		"MEDIA[Info    ] media after component switch\n";
	ASSERT_EQUAL("test_threadedlog_push_format_overrides_component_and_restores_resolution", expected, output.str());
	RETURN_TEST("test_threadedlog_push_format_overrides_component_and_restores_resolution", 0);
}

int test_threadedlog_format_change_redecides_throttle_line() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "A[%L]");
	log << reset_component;
	IsolateLine(log);
	log.Throttle(0.0, 1);
	log << Level::Info << "first";
	log.Format("B[%L]");
	log << Level::Info << "second" << std::endl;
	ASSERT_EQUAL("test_threadedlog_format_change_redecides_throttle_line", "A[Info    ] first\n", output.str());
	RETURN_TEST("test_threadedlog_format_change_redecides_throttle_line", 0);
}

int test_threadedlog_components_are_thread_local() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%c[%L]");
	constexpr int threads = 4;
	constexpr int repeats = 100;
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) {
		pool.emplace_back([&, id] {
			tlog << reset_component << component("C" + std::to_string(id));
			for (int i = 0; i < repeats; ++i)
				tlog << Level::Info << "message" << std::endl;
		});
	}

	for (auto& thread : pool) thread.join();
	std::istringstream input(output.str());
	std::string line;
	int count = 0;
	while (std::getline(input, line)) {
		ASSERT_TRUE("test_threadedlog_components_are_thread_local (line)",
			std::regex_match(line, std::regex("^C[0-3]\\[Info    \\] message$")));
		++count;
	}

	ASSERT_EQUAL("test_threadedlog_components_are_thread_local (count)", threads * repeats, count);
	RETURN_TEST("test_threadedlog_components_are_thread_local", 0);
}

int test_threadedlog_component_does_not_hold_line_lock() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "%c[%L]");
	tlog << reset_component;
	tlog << component("Filtered") << Level::Debug << "hidden" << std::endl;
	tlog << Level::Info << "visible" << std::endl;
	ASSERT_EQUAL("test_threadedlog_component_does_not_hold_line_lock", "Filtered[Info    ] visible\n", output.str());
	RETURN_TEST("test_threadedlog_component_does_not_hold_line_lock", 0);
}

int test_threadedlog_component_formats_do_not_mix() {
	std::ostringstream output;
	ThreadedLog tlog(output, Level::Info, "GENERAL[%L]");
	tlog.Format("A", "A[%L]");
	tlog.Format("B", "B[%L]");
	constexpr int threads = 2;
	constexpr int repeats = 100;
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) {
		pool.emplace_back([&, id] {
			const std::string name(1, static_cast<char>('A' + id));
			tlog << reset_component << component(name);
			for (int i = 0; i < repeats; ++i)
				tlog << Level::Info << "message" << std::endl;
		});
	}

	for (auto& thread : pool) thread.join();
	std::istringstream input(output.str());
	std::string line;
	int count = 0;
	while (std::getline(input, line)) {
		ASSERT_TRUE("test_threadedlog_component_formats_do_not_mix (line)",
			std::regex_match(line, std::regex("^[AB]\\[Info    \\] message$")));
		++count;
	}

	ASSERT_EQUAL("test_threadedlog_component_formats_do_not_mix (count)", threads * repeats, count);
	RETURN_TEST("test_threadedlog_component_formats_do_not_mix", 0);
}

// ---------------------------------------------------------------------------
// Scope / component stack
// ---------------------------------------------------------------------------

int test_threadedlog_component_stack_push_pop_and_join() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c %L:");
	log << reset_component;
	IsolateLine(log);
	log << component("Multimedia") << component("Decoder") << Level::Info << "nested" << std::endl;
	log << pop_component << Level::Info << "parent" << std::endl;
	log << reset_component << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_threadedlog_component_stack_push_pop_and_join",
		"Multimedia/Decoder Info    : nested\nMultimedia Info    : parent\n Info    : root\n",
		output.str());
	RETURN_TEST("test_threadedlog_component_stack_push_pop_and_join", 0);
}

int test_threadedlog_scope_path_and_nested_scope() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c %L:");
	log << reset_component;
	IsolateLine(log);
	auto mm = log.Scope("Multimedia");
	auto dec = mm->Scope("Decoder");
	auto abs = log.Scope("Multimedia/Encoder");
	log << Level::Info << "root" << std::endl;
	mm << Level::Info << "mm" << std::endl;
	dec << Level::Info << "dec" << std::endl;
	abs << Level::Info << "enc" << std::endl;
	ASSERT_EQUAL("test_threadedlog_scope_path_and_nested_scope",
		" Info    : root\nMultimedia Info    : mm\nMultimedia/Decoder Info    : dec\nMultimedia/Encoder Info    : enc\n",
		output.str());
	RETURN_TEST("test_threadedlog_scope_path_and_nested_scope", 0);
}

int test_threadedlog_scope_does_not_use_tls_stack() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%c %L:");
	log << reset_component;
	IsolateLine(log);
	log << component("TLS");
	auto scoped = log.Scope("Multimedia");
	scoped << Level::Info << "scoped" << std::endl;
	log << Level::Info << "stack" << std::endl;
	ASSERT_EQUAL("test_threadedlog_scope_does_not_use_tls_stack",
		"Multimedia Info    : scoped\nTLS Info    : stack\n", output.str());
	log << reset_component;
	RETURN_TEST("test_threadedlog_scope_does_not_use_tls_stack", 0);
}

int test_threadedlog_scope_format_inherits_parent_and_leaf_wins() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "ROOT %L:");
	log << reset_component;
	IsolateLine(log);
	log.Format("Multimedia", "MM %c %L:");
	auto dec = log.Scope("Multimedia/Decoder");
	dec << Level::Info << "inherited" << std::endl;
	dec->Format("DEC %c %L:");
	dec << Level::Info << "leaf" << std::endl;
	log << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_threadedlog_scope_format_inherits_parent_and_leaf_wins",
		"MM Multimedia/Decoder Info    : inherited\nDEC Multimedia/Decoder Info    : leaf\nROOT Info    : root\n",
		output.str());
	RETURN_TEST("test_threadedlog_scope_format_inherits_parent_and_leaf_wins", 0);
}

int test_threadedlog_scope_shares_lock_and_path() {
	std::ostringstream output;
	auto log = std::make_shared<ThreadedLog>(output, Level::Info, "%c %L:");
	*log << reset_component;
	IsolateLine(*log);
	auto dec = log->Scope("Multimedia/Decoder");
	dec << Level::Info << "dec" << std::endl;
	log << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_threadedlog_scope_shares_lock_and_path",
		"Multimedia/Decoder Info    : dec\n Info    : root\n", output.str());
	RETURN_TEST("test_threadedlog_scope_shares_lock_and_path", 0);
}

int test_threadedlog_scope_is_threadedlog_and_concurrent() {
	std::ostringstream output;
	auto root = std::make_shared<ThreadedLog>(output, Level::Info, "%c %L:");
	auto scoped = root->Scope("Buffer/Pipeline");
	ASSERT_TRUE("Scope preserves ThreadedLog",
		std::dynamic_pointer_cast<ThreadedLog>(scoped) != nullptr);

	constexpr int kThreads = 12;
	constexpr int kLines = 64;
	std::vector<std::thread> workers;
	workers.reserve(static_cast<std::size_t>(kThreads));
	for (int t = 0; t < kThreads; ++t) {
		workers.emplace_back([scoped]() {
			for (int i = 0; i < kLines; ++i)
				scoped << Level::Info << "n" << std::endl;
		});
	}
	for (auto& worker : workers)
		worker.join();

	const auto text = output.str();
	std::size_t lines = 0;
	for (char c : text) {
		if (c == '\n')
			++lines;
	}
	ASSERT_EQUAL("test_threadedlog_scope_is_threadedlog_and_concurrent",
		static_cast<std::size_t>(kThreads * kLines), lines);
	RETURN_TEST("test_threadedlog_scope_is_threadedlog_and_concurrent", 0);
}

int test_threadedlog_scope_throttle_is_leaf() {
	std::ostringstream output;
	auto log = std::make_shared<ThreadedLog>(output, Level::Info, "%c %L:");
	*log << reset_component;
	IsolateLine(*log);
	auto dec = log->Scope("Multimedia/Decoder");
	dec->Throttle(0.0, 1);
	dec << Level::Info << "one" << std::endl;
	dec << Level::Info << "two" << std::endl;
	log << Level::Info << "root" << std::endl;
	ASSERT_EQUAL("test_threadedlog_scope_throttle_is_leaf",
		"Multimedia/Decoder Info    : one\n Info    : root\n", output.str());
	RETURN_TEST("test_threadedlog_scope_throttle_is_leaf", 0);
}

// ---------------------------------------------------------------------------
// Throttle
// ---------------------------------------------------------------------------

int test_threadedlog_throttle_drops_without_deadlock() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	log.Throttle(0.0, 10);
	constexpr int threads = 8;
	constexpr int repeats = 100;
	std::vector<std::thread> pool;
	pool.reserve(threads);
	for (int id = 0; id < threads; ++id) {
		pool.emplace_back([&, id] {
			for (int index = 0; index < repeats; ++index)
				log << Level::Info << id << ':' << index << std::endl;
		});
	}

	for (auto& thread : pool) thread.join();
	log << Level::Fatal << "fatal survives" << std::endl;
	ASSERT_TRUE("test_threadedlog_throttle_drops_without_deadlock", output.str().find("Fatal   : fatal survives\n") != std::string::npos);
	RETURN_TEST("test_threadedlog_throttle_drops_without_deadlock", 0);
}

int test_threadedlog_flush_throttle_releases_lock() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	ThrottleSpec spec;
	spec.Policy = ThrottlePolicy::Window;
	spec.WindowKeep = 1;
	spec.WindowPeriod = 2;
	log.Throttle(spec);
	log << Level::Info << "first" << std::endl;
	log << Level::Info << "dropped" << std::endl;
	log.FlushThrottle();
	log << Level::Fatal << "fatal after flush" << std::endl;
	ASSERT_TRUE("test_threadedlog_flush_throttle_releases_lock",
		output.str().find("Fatal   : fatal after flush\n") != std::string::npos);
	RETURN_TEST("test_threadedlog_flush_throttle_releases_lock", 0);
}

int test_threadedlog_flush_mid_line_preserves_lock_owner() {
	std::ostringstream output;
	ThreadedLog log(output, Level::Info, "%L:");
	ThrottleSpec spec;
	spec.Policy = ThrottlePolicy::Window;
	spec.WindowKeep = 1;
	spec.WindowPeriod = 2;
	log.Throttle(spec);
	log << Level::Info << "seed" << std::endl;
	log << Level::Info << "dropped" << std::endl;
	log << Level::Info << "first";
	log.FlushThrottle();
	log << Level::Fatal << "after" << std::endl;
	std::thread other([&] { log << Level::Fatal << "other" << std::endl; });
	other.join();
	ASSERT_TRUE("test_threadedlog_flush_mid_line_preserves_lock_owner",
		output.str().find("Info    : first\n") != std::string::npos &&
		output.str().find("Info    : dropped 1 messages\n") != std::string::npos &&
		output.str().find("Fatal   : after\n") != std::string::npos);
	ASSERT_TRUE("test_threadedlog_flush_mid_line_preserves_lock_owner (other)",
		output.str().find("Fatal   : other\n") != std::string::npos);
	RETURN_TEST("test_threadedlog_flush_mid_line_preserves_lock_owner", 0);
}

int main() {
	int result = 0;

	// Basic emit
	result += test_threadedlog_basic();
	result += test_smart_pointer_usage();

	// Floor / Enabled / views
	result += test_threadedlog_critical_levels_are_never_filtered();
	result += test_threadedlog_enabled_and_views();

	// Binary span
	result += test_threadedlog_span_default_is_base64();
	result += test_threadedlog_span_vector_converts();
	result += test_threadedlog_span_hex();
	result += test_threadedlog_span_filtered();

	// Line lock / concurrency
	result += test_threadedlog_multithreaded_ordering();
	result += test_threadedlog_no_endl_sharing();
	result += test_threadedlog_deterministic_ordering();
	result += test_threadedlog_level_switch_flush();

	// Filtered path must not leak the lock
	result += test_threadedlog_filtered_endl_no_deadlock();
	result += test_threadedlog_filtered_multithreaded_then_info();
	result += test_threadedlog_filtered_hot_path();

	// Wide / UTF-8
	result += test_threadedlog_invalid_wide_releases_line_lock();
	result += test_threadedlog_filtered_wide_skips_conversion();

	// Color / format / group
	result += test_threadedlog_colored_lines_do_not_mix();
	result += test_threadedlog_push_pop_format_is_line_safe();
	result += test_threadedlog_groups_do_not_mix();
	result += test_threadedlog_filtered_group_releases_lock();

	// Components
	result += test_threadedlog_component_header_is_sticky_and_resettable();
	result += test_threadedlog_component_without_token_preserves_legacy_output();
	result += test_threadedlog_component_color_override_has_priority();
	result += test_threadedlog_empty_component_does_not_push();
	result += test_threadedlog_component_format_priority_and_fallback();
	result += test_threadedlog_push_format_overrides_component_and_restores_resolution();
	result += test_threadedlog_format_change_redecides_throttle_line();
	result += test_threadedlog_components_are_thread_local();
	result += test_threadedlog_component_does_not_hold_line_lock();
	result += test_threadedlog_component_formats_do_not_mix();

	// Scope / component stack
	result += test_threadedlog_component_stack_push_pop_and_join();
	result += test_threadedlog_scope_path_and_nested_scope();
	result += test_threadedlog_scope_does_not_use_tls_stack();
	result += test_threadedlog_scope_format_inherits_parent_and_leaf_wins();
	result += test_threadedlog_scope_shares_lock_and_path();
	result += test_threadedlog_scope_is_threadedlog_and_concurrent();
	result += test_threadedlog_scope_throttle_is_leaf();

	// Throttle
	result += test_threadedlog_throttle_drops_without_deadlock();
	result += test_threadedlog_flush_throttle_releases_lock();
	result += test_threadedlog_flush_mid_line_preserves_lock_owner();

	if (result == 0) {
		std::cout << "All tests passed!" << std::endl;
	} else {
		std::cout << result << " tests failed." << std::endl;
	}

	return result;
}
