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
			 * @brief Set a level color while holding the line lock.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that level.
			 * @return Reference to this logger.
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
			 * @param component Component name.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that component and level.
			 * @return Reference to this logger.
			 */
			Log& Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color) override;
			/**
			 * @brief Get a component color, falling back to the general color.
			 * @param component Component name.
			 * @param level Level whose color is requested.
			 * @return Component override or general color.
			 */
			StormByte::Logger::Color Color(const std::string& component, const Level& level) const override;
			/**
			 * @brief Set the general header format under the line lock.
			 * @param format Format used by default.
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
			 * @param component Component name.
			 * @param format Format, or empty to remove the override.
			 * @return Reference to this logger.
			 */
			Log& Format(const std::string& component, const std::string& format) override;
			/**
			 * @brief Get a component-specific format, falling back to the general format.
			 * @param component Component name.
			 * @return Component format or general format.
			 */
			const std::string& Format(const std::string& component) const override;
			/** @brief Install a throttle rule under the line lock. */
			Log& Throttle(const ThrottleSpec& spec) override;
			/** @brief Remove a throttle rule under the line lock. */
			Log& NoThrottle(const ThrottleSpec& spec) override;
			/** @brief Install a global Drop rule. */
			Log& Throttle(double rate, std::size_t burst) override;
			/** @brief Install a global policy rule. */
			Log& Throttle(double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period = 0) override;
			/** @brief Install a level-scoped rule. */
			Log& Throttle(const Level& level, double rate, std::size_t burst) override;
			/** @brief Install a group-scoped rule. */
			Log& Throttle(GroupManip group, double rate, std::size_t burst) override;
			/** @brief Install a component-scoped rule. */
			Log& Throttle(ComponentManip component, double rate, std::size_t burst) override;
			/** @brief Install an exact component/level/group rule. */
			Log& Throttle(ComponentManip component, const Level& level, GroupManip group, double rate, std::size_t burst, ThrottlePolicy policy = ThrottlePolicy::Drop, std::size_t value = 0, std::size_t period = 0) override;
			/** @brief Remove all throttle rules. */
			Log& NoThrottle() override;
			/** @brief Remove a level-scoped rule. */
			Log& NoThrottle(const Level& level) override;
			/** @brief Remove a group-scoped rule. */
			Log& NoThrottle(GroupManip group) override;
			/** @brief Remove a component-scoped rule. */
			Log& NoThrottle(ComponentManip component) override;
			/** @brief Remove an exact component/level/group rule. */
			Log& NoThrottle(ComponentManip component, const Level& level, GroupManip group) override;
			/** @brief Flush all dropped summaries under the line lock. */
			Log& FlushThrottle() override;
			/** @brief Flush matching dropped summaries under the line lock. */
			Log& FlushThrottle(const ThrottleSpec& spec) override;

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
			/**
			 * @brief Stream UTF-8 text. @c std::string converts to this view.
			 */
			inline Log& operator<<(std::string_view v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			inline Log& operator<<(const char* v) {
				if (!WillWrite()) [[likely]] return *this;
				Write(v);
				return *this;
			}
			/**
			 * @brief Stream wide text. @c std::wstring converts to this view.
			 */
			inline Log& operator<<(std::wstring_view v) {
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
			inline Log& operator<<(HexManip m) {
				Write(m);
				return *this;
			}
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
			 * @brief Select the sticky component for the current thread.
			 * @param manip Component manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(ComponentManip manip) {
				Write(manip);
				return *this;
			}
			/**
			 * @brief Clear the sticky component for the current thread.
			 * @param manip Reset-component manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(ResetComponentManip manip) {
				Write(manip);
				return *this;
			}
			//@}

		private:
			std::shared_ptr<ThreadLock> m_lock;	///< Shared line lock (copy shares it)

			/**
			 * @name Write
			 * Locked emit. Newline stream manipulators drop the line lock.
			 */
			//@{
			void Write(bool v) override;
			void Write(char v) override;
			void Write(signed char v) override;
			void Write(unsigned char v) override;
			void Write(short v) override;
			void Write(unsigned short v) override;
			void Write(int v) override;
			void Write(unsigned int v) override;
			void Write(long v) override;
			void Write(unsigned long v) override;
			void Write(long long v) override;
			void Write(unsigned long long v) override;
			void Write(float v) override;
			void Write(double v) override;
			void Write(long double v) override;
			void Write(std::string_view v) override;
			void Write(const char* v) override;
			void Write(std::wstring_view v) override;
			void Write(const wchar_t* v) override;
			void Write(const Level& level) override;
			void Write(std::ostream& (*manip)(std::ostream&)) override;
			void Write(Log& (*manip)(Log&) noexcept) override;
			void Write(RedactManip m) override;
			void Write(HexManip m) override;
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
			 * @brief Apply a component manipulator under the line lock.
			 * @param manip Component manipulator.
			 */
			void Write(ComponentManip manip) override;
			/**
			 * @brief Apply a reset-component manipulator under the line lock.
			 * @param manip Reset-component manipulator.
			 */
			void Write(ResetComponentManip manip) override;
			//@}
	};
}
