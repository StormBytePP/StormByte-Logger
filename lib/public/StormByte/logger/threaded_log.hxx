/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Logger.
 *
 * StormByte-Logger original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-Logger source in this
 * repository. They do not cover other StormByte modules or any third-party
 * material shipped with this repository (including everything under
 * thirdparty/, and in particular the bundled StormByte-String tree and
 * the StormByte Base tree it vendors), which remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-Logger. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#pragma once

#include <StormByte/logger/log.hxx>
#include <StormByte/thread_lock.hxx>

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

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
	 * Text payloads use @c std::string_view / @c std::wstring_view like @c Log.
	 * Owned suite text and @c StormByte::Size use the same overloads as @c Log.
	 * Binary payloads use @c std::span<const std::byte> (Base64 by default).
	 *
	 * @c Scope clones this type and shares both the Implementation and the
	 * line lock. The sticky component path is copied onto the clone.
	 */
	class STORMBYTE_LOGGER_PUBLIC ThreadedLog : public Log {
		public:
			/**
			 * @brief Construct a ThreadedLog writing to @p out.
			 * @param out Output stream.
			 * @param level Minimum Level that will be emitted.
			 * @param format Header format string (%L, %T, %i, %c, %g).
			 */
			ThreadedLog(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

			/**
			 * @brief Copy constructor.
			 * @note Shares the Implementation and the line lock. Copies the sticky path.
			 */
			ThreadedLog(const ThreadedLog&) = default;

			/**
			 * @brief Move constructor.
			 */
			ThreadedLog(ThreadedLog&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~ThreadedLog() noexcept override = default;

			/**
			 * @brief Copy assignment.
			 * @return Reference to this logger.
			 * @note Shares the Implementation and the line lock.
			 */
			ThreadedLog& operator=(const ThreadedLog&) = default;

			/**
			 * @brief Move assignment.
			 * @return Reference to this logger.
			 */
			ThreadedLog& operator=(ThreadedLog&&) noexcept = default;

			/**
			 * @brief Set a level color while holding the line lock.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that level.
			 * @return Reference to this logger.
			 * @note On a scoped facade this stores a component override for the sticky path.
			 */
			Log& Color(const Level& level, const StormByte::Logger::Color& color) override;

			/**
			 * @brief Get the configured color for a logging level.
			 * @param level Level whose color is requested.
			 * @return Configured color.
			 */
			StormByte::Logger::Color Color(const Level& level) const override;

			/**
			 * @brief Set a component color override under the line lock.
			 * @param component Component path.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that component and level.
			 * @return Reference to this logger.
			 */
			Log& Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color) override;

			/**
			 * @brief Get a component color, falling back along the path then to the general color.
			 * @param component Component path.
			 * @param level Level whose color is requested.
			 * @return Component override or general color.
			 */
			StormByte::Logger::Color Color(const std::string& component, const Level& level) const override;

			/**
			 * @brief Set the header format under the line lock.
			 * @param format Format used by default, or for the sticky path on a scoped facade.
			 * @return Reference to this logger.
			 */
			Log& Format(const std::string& format) override;

			/**
			 * @brief Get the effective current header format.
			 * @return Temporary, component-specific or general format.
			 */
			const std::string& Format() const override;

			/**
			 * @brief Set or remove a component-specific header format under the line lock.
			 * @param component Component path.
			 * @param format Format, or empty to remove the override.
			 * @return Reference to this logger.
			 */
			Log& Format(const std::string& component, const std::string& format) override;

			/**
			 * @brief Get a component-specific format, falling back along the path then to general.
			 * @param component Component path.
			 * @return Component format or general format.
			 */
			const std::string& Format(const std::string& component) const override;

			/**
			 * @brief Install a throttle rule under the line lock.
			 * @param spec Rule to install. An absent Component uses the sticky path when scoped.
			 * @return Reference to this logger.
			 */
			Log& Throttle(const ThrottleSpec& spec) override;

			/**
			 * @brief Remove a throttle rule under the line lock.
			 * @param spec Selectors of the rule to remove.
			 * @return Reference to this logger.
			 */
			Log& NoThrottle(const ThrottleSpec& spec) override;

			/**
			 * @brief Install a Drop rule (global, or sticky path if scoped).
			 * @param rate Lines per second.
			 * @param burst Token capacity.
			 * @return Reference to this logger.
			 */
			Log& Throttle(double rate, std::size_t burst) override;

			/**
			 * @brief Install a Sample or Window rule (global, or sticky path if scoped).
			 * @param rate Lines per second.
			 * @param burst Token capacity.
			 * @param policy Sample or Window.
			 * @param value SampleN or WindowKeep.
			 * @param period WindowPeriod when @p policy is Window.
			 * @return Reference to this logger.
			 */
			Log& Throttle(double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period = 0) override;

			/**
			 * @brief Install a level-scoped Drop rule.
			 * @param level Level selector.
			 * @param rate Lines per second.
			 * @param burst Token capacity.
			 * @return Reference to this logger.
			 */
			Log& Throttle(const Level& level, double rate, std::size_t burst) override;

			/**
			 * @brief Install a group-scoped Drop rule.
			 * @param group Group selector.
			 * @param rate Lines per second.
			 * @param burst Token capacity.
			 * @return Reference to this logger.
			 */
			Log& Throttle(GroupManip group, double rate, std::size_t burst) override;

			/**
			 * @brief Install a component-scoped Drop rule. The manipulator path is used as-is.
			 * @param component Component path.
			 * @param rate Lines per second.
			 * @param burst Token capacity.
			 * @return Reference to this logger.
			 */
			Log& Throttle(ComponentManip component, double rate, std::size_t burst) override;

			/**
			 * @brief Install an exact component/level/group rule.
			 * @param component Component path used as-is.
			 * @param level Level selector.
			 * @param group Group selector.
			 * @param rate Lines per second.
			 * @param burst Token capacity.
			 * @param policy Count policy.
			 * @param value SampleN or WindowKeep.
			 * @param period WindowPeriod when @p policy is Window.
			 * @return Reference to this logger.
			 */
			Log& Throttle(ComponentManip component, const Level& level, GroupManip group, double rate, std::size_t burst, ThrottlePolicy policy = ThrottlePolicy::Drop, std::size_t value = 0, std::size_t period = 0) override;

			/**
			 * @brief Remove all throttle rules.
			 * @return Reference to this logger.
			 */
			Log& NoThrottle() override;

			/**
			 * @brief Remove a level-scoped rule.
			 * @param level Level selector.
			 * @return Reference to this logger.
			 */
			Log& NoThrottle(const Level& level) override;

			/**
			 * @brief Remove a group-scoped rule.
			 * @param group Group selector.
			 * @return Reference to this logger.
			 */
			Log& NoThrottle(GroupManip group) override;

			/**
			 * @brief Remove a component-scoped rule. The manipulator path is used as-is.
			 * @param component Component path.
			 * @return Reference to this logger.
			 */
			Log& NoThrottle(ComponentManip component) override;

			/**
			 * @brief Remove an exact component/level/group rule.
			 * @param component Component path used as-is.
			 * @param level Level selector.
			 * @param group Group selector.
			 * @return Reference to this logger.
			 */
			Log& NoThrottle(ComponentManip component, const Level& level, GroupManip group) override;

			/**
			 * @brief Flush all dropped summaries under the line lock.
			 * @return Reference to this logger.
			 */
			Log& FlushThrottle() override;

			/**
			 * @brief Flush matching dropped summaries under the line lock.
			 * @param spec Selectors of the rules to flush.
			 * @return Reference to this logger.
			 */
			Log& FlushThrottle(const ThrottleSpec& spec) override;

			/**
			 * @name Streaming Operators
			 * Same contract as Log; data overloads early-out when filtered.
			 */
			//@{

			/**
			 * @brief Stream a boolean.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(bool v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a character.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(char v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a signed character.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(signed char v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream an unsigned character.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(unsigned char v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a short.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(short v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream an unsigned short.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(unsigned short v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream an int.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(int v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream an unsigned int.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(unsigned int v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a long.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream an unsigned long.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(unsigned long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a long long.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(long long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream an unsigned long long.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(unsigned long long v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a float.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(float v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a double.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(double v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a long double.
			 * @param v Value to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(long double v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream UTF-8 text. @c std::string converts to this view.
			 * @param v Text to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(std::string_view v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a C string.
			 * @param v Text to write; may be null.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const char* v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream owned UTF-8 bytes.
			 * @param v Buffer owned by Base. Copied into the line buffer.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const StormByte::CString& v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(static_cast<std::string_view>(v));
				return *this;
			}

			/**
			 * @brief Stream owned UTF-8 text.
			 * @param v Text owned by String. Copied into the line buffer.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const StormByte::String::String& v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(static_cast<std::string_view>(v));
				return *this;
			}

			/**
			 * @brief Stream wide text. @c std::wstring converts to this view.
			 * @param v Wide text to write.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(std::wstring_view v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a wide C string.
			 * @param v Text to write; may be null.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const wchar_t* v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream owned wide bytes.
			 * @param v Buffer owned by Base. Copied into the line buffer.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const StormByte::WCString& v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(static_cast<std::wstring_view>(v));
				return *this;
			}

			/**
			 * @brief Stream owned wide text.
			 * @param v Text owned by String. Copied into the line buffer.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const StormByte::String::WString& v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(static_cast<std::wstring_view>(v));
				return *this;
			}

			/**
			 * @brief Stream raw bytes. Default output is Base64; @c hex dumps @c 0xHH.
			 * @param v Contiguous bytes. @c std::vector<std::byte> converts to this span.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(std::span<const std::byte> v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}

			/**
			 * @brief Stream a byte count.
			 * @param v Size. Uses @c Size::operator std::string (IEC text).
			 * @return Reference to this logger.
			 * @note The @c std::string is built only after @c WillWrite.
			 */
			inline Log& operator<<(const StormByte::Size& v) {
				if (!WillWrite()) [[likely]] return *this;
				const std::string text = static_cast<std::string>(v);
				Write(std::string_view{text});
				return *this;
			}

			/**
			 * @brief Set the level of the current line.
			 * @param level Level to emit.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(const Level& level) {
				Write(level);
				return *this;
			}

			/**
			 * @brief Apply a stream manipulator (e.g. @c std::endl).
			 * @param manip Stream manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(std::ostream& (*manip)(std::ostream&)) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Apply a Log manipulator.
			 * @param manip Logger manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(Log& (*manip)(Log&) noexcept) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Apply redaction policy. State remains until noredact.
			 * @param m Redaction manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(RedactManip m) {
				Write(m);
				return *this;
			}

			/**
			 * @brief Dump subsequent payloads as hex bytes until nohex.
			 * @param m Hex manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(HexManip m) {
				Write(m);
				return *this;
			}

			/**
			 * @brief Disable hex dumps and restore default payload formatting.
			 * @param m No-hex manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(NoHexManip m) {
				Write(m);
				return *this;
			}

			/**
			 * @brief Apply a configured or explicit content color.
			 * @param manip Color manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(ColorManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Disable color for subsequent content.
			 * @param manip No-color manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(NoColorManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Save and activate a temporary format under the line lock.
			 * @param manip Format manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(FormatManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Restore a saved format under the line lock.
			 * @param manip Pop-format manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(PopFormatManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Set the producer group for the current line under the line lock.
			 * @param manip Group manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(GroupManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Push a component segment onto the current thread's stack.
			 * @param manip Component manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(ComponentManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Pop one component segment from the current thread's stack.
			 * @param manip Pop-component manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(PopComponentManip manip) {
				Write(manip);
				return *this;
			}

			/**
			 * @brief Clear the current thread's component stack.
			 * @param manip Reset-component manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(ResetComponentManip manip) {
				Write(manip);
				return *this;
			}

			//@}

		protected:
			/**
			 * @brief Deep-copy this facade into a shared_ptr.
			 * @return Pointer to a ThreadedLog that shares Implementation and line lock.
			 */
			PointerType Clone() const override;

			/**
			 * @brief Move this facade into a shared_ptr.
			 * @return Pointer to a ThreadedLog that shares Implementation and line lock.
			 * @note This object is not emptied; the Implementation stays shared.
			 */
			PointerType Move() override;

		private:
			std::shared_ptr<ThreadLock> m_lock;	///< Shared line lock (copy and Scope share it)

			/**
			 * @name Write
			 * Locked emit. Newline stream manipulators drop the line lock.
			 */
			//@{

			/**
			 * @brief Forward a boolean under the line lock.
			 * @param v Value to write.
			 */
			void Write(bool v) override;

			/**
			 * @brief Forward a character under the line lock.
			 * @param v Value to write.
			 */
			void Write(char v) override;

			/**
			 * @brief Forward a signed character under the line lock.
			 * @param v Value to write.
			 */
			void Write(signed char v) override;

			/**
			 * @brief Forward an unsigned character under the line lock.
			 * @param v Value to write.
			 */
			void Write(unsigned char v) override;

			/**
			 * @brief Forward a short under the line lock.
			 * @param v Value to write.
			 */
			void Write(short v) override;

			/**
			 * @brief Forward an unsigned short under the line lock.
			 * @param v Value to write.
			 */
			void Write(unsigned short v) override;

			/**
			 * @brief Forward an int under the line lock.
			 * @param v Value to write.
			 */
			void Write(int v) override;

			/**
			 * @brief Forward an unsigned int under the line lock.
			 * @param v Value to write.
			 */
			void Write(unsigned int v) override;

			/**
			 * @brief Forward a long under the line lock.
			 * @param v Value to write.
			 */
			void Write(long v) override;

			/**
			 * @brief Forward an unsigned long under the line lock.
			 * @param v Value to write.
			 */
			void Write(unsigned long v) override;

			/**
			 * @brief Forward a long long under the line lock.
			 * @param v Value to write.
			 */
			void Write(long long v) override;

			/**
			 * @brief Forward an unsigned long long under the line lock.
			 * @param v Value to write.
			 */
			void Write(unsigned long long v) override;

			/**
			 * @brief Forward a float under the line lock.
			 * @param v Value to write.
			 */
			void Write(float v) override;

			/**
			 * @brief Forward a double under the line lock.
			 * @param v Value to write.
			 */
			void Write(double v) override;

			/**
			 * @brief Forward a long double under the line lock.
			 * @param v Value to write.
			 */
			void Write(long double v) override;

			/**
			 * @brief Forward UTF-8 text under the line lock.
			 * @param v Text to write.
			 */
			void Write(std::string_view v) override;

			/**
			 * @brief Forward a C string under the line lock.
			 * @param v Text to write; may be null.
			 */
			void Write(const char* v) override;

			/**
			 * @brief Forward wide text under the line lock.
			 * @param v Text to write.
			 */
			void Write(std::wstring_view v) override;

			/**
			 * @brief Forward a wide C string under the line lock.
			 * @param v Text to write; may be null.
			 */
			void Write(const wchar_t* v) override;

			/**
			 * @brief Forward raw bytes under the line lock after formatting.
			 * @param v Contiguous bytes to format as Base64 or hex.
			 */
			void Write(std::span<const std::byte> v) override;

			/**
			 * @brief Forward a level change under the line lock.
			 * @param level Level of the current line.
			 */
			void Write(const Level& level) override;

			/**
			 * @brief Forward a stream manipulator under the line lock.
			 * @param manip Stream manipulator.
			 */
			void Write(std::ostream& (*manip)(std::ostream&)) override;

			/**
			 * @brief Forward a Log manipulator under the line lock.
			 * @param manip Logger manipulator.
			 */
			void Write(Log& (*manip)(Log&) noexcept) override;

			/**
			 * @brief Apply redaction state under the line lock.
			 * @param m Redaction manipulator.
			 */
			void Write(RedactManip m) override;

			/**
			 * @brief Apply hex-dump state under the line lock.
			 * @param m Hex manipulator.
			 */
			void Write(HexManip m) override;

			/**
			 * @brief Disable hex dumps under the line lock.
			 * @param m No-hex manipulator.
			 */
			void Write(NoHexManip m) override;

			/**
			 * @brief Apply a color manipulator under the line lock.
			 * @param manip Color manipulator.
			 */
			void Write(ColorManip manip) override;

			/**
			 * @brief Apply a no-color manipulator under the line lock.
			 * @param manip No-color manipulator.
			 */
			void Write(NoColorManip manip) override;

			/**
			 * @brief Apply a push-format manipulator under the line lock.
			 * @param manip Format manipulator.
			 */
			void Write(FormatManip manip) override;

			/**
			 * @brief Apply a pop-format manipulator under the line lock.
			 * @param manip Pop-format manipulator.
			 */
			void Write(PopFormatManip manip) override;

			/**
			 * @brief Apply a group manipulator under the line lock.
			 * @param manip Group manipulator.
			 */
			void Write(GroupManip manip) override;

			/**
			 * @brief Apply a component push manipulator.
			 * @param manip Component manipulator.
			 */
			void Write(ComponentManip manip) override;

			/**
			 * @brief Apply a component pop manipulator.
			 * @param manip Pop-component manipulator.
			 */
			void Write(PopComponentManip manip) override;

			/**
			 * @brief Apply a reset-component manipulator.
			 * @param manip Reset-component manipulator.
			 */
			void Write(ResetComponentManip manip) override;

			//@}
	};
}
