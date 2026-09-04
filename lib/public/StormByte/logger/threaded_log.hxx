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

#include <StormByte/logger/log.hxx>
#include <StormByte/thread_lock.hxx>

#include <memory>

/**
 * @namespace StormByte::Logger
 * @brief Logger module of the StormByte suite.
 */
namespace StormByte::Logger {
	/**
	 * @class ThreadedLog
	 * @brief Thread-safe logging facade.
	 *
	 * Serializes logical lines (until a newline manipulator) so concurrent
	 * writers do not interleave. Filtered messages do not hold the line lock.
	 */
	class STORMBYTE_LOGGER_PUBLIC ThreadedLog : public Log {
		public:
			/**
			 * @brief Construct a ThreadedLog writing to @p out.
			 * @param out Output stream.
			 * @param level Minimum Level that will be emitted.
			 * @param format Header format string (%L, %T, %i).
			 */
			ThreadedLog(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

			/** @brief Copy constructor. Shares the line lock. */
			ThreadedLog(const ThreadedLog&) = default;
			/** @brief Move constructor. */
			ThreadedLog(ThreadedLog&&) noexcept = default;
			/** @brief Destructor. */
			~ThreadedLog() noexcept = default;
			/** @brief Copy assignment. Shares the line lock. */
			ThreadedLog& operator=(const ThreadedLog&) = default;
			/** @brief Move assignment. */
			ThreadedLog& operator=(ThreadedLog&&) noexcept = default;

			/**
			 * @name Streaming Operators
			 * Same contract as Log; data overloads early-out when filtered.
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
			inline Log& operator<<(RedactManip m) {
				Write(m);
				return *this;
			}
			//@}

		private:
			std::shared_ptr<ThreadLock> m_lock;	///< Shared line lock (copy shares it)

			void Write(bool v) override;								///< @brief Locked emit.
			void Write(char v) override;								///< @brief Locked emit.
			void Write(signed char v) override;							///< @brief Locked emit.
			void Write(unsigned char v) override;						///< @brief Locked emit.
			void Write(short v) override;								///< @brief Locked emit.
			void Write(unsigned short v) override;						///< @brief Locked emit.
			void Write(int v) override;									///< @brief Locked emit.
			void Write(unsigned int v) override;						///< @brief Locked emit.
			void Write(long v) override;								///< @brief Locked emit.
			void Write(unsigned long v) override;						///< @brief Locked emit.
			void Write(long long v) override;							///< @brief Locked emit.
			void Write(unsigned long long v) override;					///< @brief Locked emit.
			void Write(float v) override;								///< @brief Locked emit.
			void Write(double v) override;								///< @brief Locked emit.
			void Write(long double v) override;							///< @brief Locked emit.
			void Write(const std::string& v) override;					///< @brief Locked emit.
			void Write(const char* v) override;							///< @brief Locked emit.
			void Write(const std::wstring& v) override;					///< @brief Locked emit.
			void Write(const wchar_t* v) override;						///< @brief Locked emit.
			void Write(const Level& level) override;					///< @brief Set level (lock as needed).
			void Write(std::ostream& (*manip)(std::ostream&)) override;	///< @brief Stream manipulator; newline drops the lock.
			void Write(Log& (*manip)(Log&) noexcept) override;			///< @brief Logger manipulator.
			void Write(RedactManip m) override;							///< @brief Enable redaction policy.
	};
}
