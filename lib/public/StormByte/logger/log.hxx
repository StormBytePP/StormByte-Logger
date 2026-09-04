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

#pragma once

#include <StormByte/logger/manipulators.hxx>
#include <StormByte/logger/typedefs.hxx>

#include <memory>
#include <ostream>
#include <string>

/**
 * @namespace StormByte::Logger
 * @brief Logger module of the StormByte suite.
 */
namespace StormByte::Logger {
	class Implementation;

	/**
	 * @class Log
	 * @brief Public streaming facade for the StormByte logger.
	 *
	 * Owns a shared_ptr to the internal Implementation and exposes operator<<
	 * overloads similar to std::ostream. Filtered levels early-out without I/O.
	 */
	class STORMBYTE_LOGGER_PUBLIC Log {
		friend STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept;
		friend STORMBYTE_LOGGER_PUBLIC Log& no_redact(Log& log) noexcept;

		public:
			/**
			 * @brief Construct a Log writing to @p out.
			 * @param out Output stream (e.g. std::cout).
			 * @param level Minimum Level that will be emitted.
			 * @param format Header format: %L level, %T timestamp, %i thread id, %% literal %.
			 */
			Log(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

			Log(const Log&) = default;
			Log(Log&&) noexcept = default;
			~Log() noexcept = default;
			Log& operator=(const Log&) = default;
			Log& operator=(Log&&) noexcept = default;

			/**
			 * @name Streaming Operators
			 * Data overloads early-out when the current message level is filtered.
			 * Level, stream manipulators, Log manipulators and RedactManip are always
			 * forwarded so logger state stays consistent.
			 */
			//@{
			inline Log& operator<<(bool v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(char v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(signed char v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(unsigned char v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(short v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(unsigned short v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(int v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(unsigned int v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(unsigned long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(long long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(unsigned long long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(float v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(double v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(long double v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(const std::string& v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(const char* v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(const std::wstring& v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(const wchar_t* v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(const Level& level) {
				Write(level);
				return *this;
			}
			inline Log& operator<<(std::ostream& (*manip)(std::ostream&)) {
				Write(manip);
				return *this;
			}
			inline Log& operator<<(Log& (*manip)(Log&) noexcept) {
				Write(manip);
				return *this;
			}
			/**
			 * @brief Apply redaction policy (full or keep-last-N). State remains until no_redact.
			 */
			inline Log& operator<<(RedactManip m) {
				Write(m);
				return *this;
			}
			//@}

		protected:
			std::shared_ptr<Implementation> m_impl;

			/**
			 * @brief Whether messages at the current level will be written.
			 */
			bool WillWrite() const noexcept;

			virtual void Write(bool v);
			virtual void Write(char v);
			virtual void Write(signed char v);
			virtual void Write(unsigned char v);
			virtual void Write(short v);
			virtual void Write(unsigned short v);
			virtual void Write(int v);
			virtual void Write(unsigned int v);
			virtual void Write(long v);
			virtual void Write(unsigned long v);
			virtual void Write(long long v);
			virtual void Write(unsigned long long v);
			virtual void Write(float v);
			virtual void Write(double v);
			virtual void Write(long double v);
			virtual void Write(const std::string& v);
			virtual void Write(const char* v);
			virtual void Write(const std::wstring& v);
			virtual void Write(const wchar_t* v);
			virtual void Write(const Level& level);
			virtual void Write(std::ostream& (*manip)(std::ostream&));
			virtual void Write(Log& (*manip)(Log&) noexcept);
			/**
			 * @brief Forward redaction state to the implementation.
			 */
			virtual void Write(RedactManip m);
	};

	template <typename Ptr, typename T>
	Ptr& operator<<(Ptr& logger, const T& value) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Log>> || std::is_same_v<Ptr, std::unique_ptr<Log>> {
		if (logger)
			*logger << value;
		return logger;
	}

	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, const Level& level) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Log>> || std::is_same_v<Ptr, std::unique_ptr<Log>> {
		if (logger)
			*logger << level;
		return logger;
	}

	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, std::ostream& (*manip)(std::ostream&)) noexcept
		requires std::is_same_v<Ptr, std::shared_ptr<Log>> || std::is_same_v<Ptr, std::unique_ptr<Log>> {
		if (logger)
			*logger << manip;
		return logger;
	}
}
