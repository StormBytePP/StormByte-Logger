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
		ASSERT_EQUAL("test_format_mask_literals (leftover %L)", std::string("none"), std::string("%L"));
		RETURN_TEST("test_format_mask_literals", 1);
	}
	if (out.find("%i") != std::string::npos) {
		ASSERT_EQUAL("test_format_mask_literals (leftover %i)", std::string("none"), std::string("%i"));
		RETURN_TEST("test_format_mask_literals", 1);
	}
	if (out.find("%T") != std::string::npos) {
		ASSERT_EQUAL("test_format_mask_literals (leftover %T)", std::string("none"), std::string("%T"));
		RETURN_TEST("test_format_mask_literals", 1);
	}
	// Literal 'T' followed by thread id should appear as "[T"
	if (out.find("[T") == std::string::npos) {
		ASSERT_EQUAL("test_format_mask_literals (missing [T)", std::string("found"), out);
		RETURN_TEST("test_format_mask_literals", 1);
	}
	// Message should be present
	if (out.find("hello") == std::string::npos) {
		ASSERT_EQUAL("test_format_mask_literals (missing message)", std::string("hello"), out);
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
