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
#include <StormByte/type_traits.hxx>

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
	 * The configured print level does not suppress Warning, Error or Fatal.
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
			 * @param format Header format: %L level, %T timestamp, %i thread id, %c component, %g group, %% literal %.
			 */
			Log(std::ostream& out, const Level& level = Level::Info, const std::string& format = "[%L] %T");

			/** @brief Copy constructor. Shares the Implementation. */
			Log(const Log&) = default;
			/** @brief Move constructor. */
			Log(Log&&) noexcept = default;
			/** @brief Destructor. */
			~Log() noexcept = default;
			/** @brief Copy assignment. Shares the Implementation. */
			Log& operator=(const Log&) = default;
			/** @brief Move assignment. */
			Log& operator=(Log&&) noexcept = default;

			/**
			 * @brief Set the configured color for a logging level.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that level.
			 * @return Reference to this logger.
			 */
			virtual Log& Color(const Level& level, const StormByte::Logger::Color& color);

			/**
			 * @brief Get the configured color for a logging level.
			 * @param level Level whose color is requested.
			 * @return Configured color.
			 */
			virtual StormByte::Logger::Color Color(const Level& level) const;

			/**
			 * @brief Set a color override for a component and level.
			 * @param component Component name.
			 * @param level Level whose color is changed.
			 * @param color Color to use for that component and level.
			 * @return Reference to this logger.
			 */
			virtual Log& Color(const std::string& component, const Level& level, const StormByte::Logger::Color& color);

			/**
			 * @brief Get a component color, falling back to the general color.
			 * @param component Component name.
			 * @param level Level whose color is requested.
			 * @return Component override or general color.
			 */
			virtual StormByte::Logger::Color Color(const std::string& component, const Level& level) const;

			/**
			 * @brief Set the general header format.
			 * @param format Format used by default.
			 * @return Reference to this logger.
			 */
			virtual Log& Format(const std::string& format);

			/**
			 * @brief Get the effective current header format.
			 * @return Temporary, component-specific or general format.
			 */
			virtual const std::string& Format() const;

			/**
			 * @brief Set or remove a component-specific header format.
			 * @param component Component name; empty selects the general format.
			 * @param format Format, or empty to remove the override.
			 * @return Reference to this logger.
			 */
			virtual Log& Format(const std::string& component, const std::string& format);

			/**
			 * @brief Get a component-specific format, falling back to the general format.
			 * @param component Component name.
			 * @return Component format or general format.
			 */
			virtual const std::string& Format(const std::string& component) const;

			/**
			 * @brief Install a throttle rule.
			 * @param spec Rule to install.
			 * @return Reference to this logger.
			 */
			virtual Log& Throttle(const ThrottleSpec& spec);
			/**
			 * @brief Remove a throttle rule with the same selectors.
			 * @param spec Selectors of the rule to remove.
			 * @return Reference to this logger.
			 */
			virtual Log& NoThrottle(const ThrottleSpec& spec);
			/** @brief Install a global Drop rule. */
			virtual Log& Throttle(double rate, std::size_t burst);
			/** @brief Install a global Sample or Window rule. */
			virtual Log& Throttle(double rate, std::size_t burst, ThrottlePolicy policy, std::size_t value, std::size_t period = 0);
			/** @brief Install a level-scoped Drop rule. */
			virtual Log& Throttle(const Level& level, double rate, std::size_t burst);
			/** @brief Install a group-scoped Drop rule. */
			virtual Log& Throttle(GroupManip group, double rate, std::size_t burst);
			/** @brief Install a component-scoped Drop rule. */
			virtual Log& Throttle(ComponentManip component, double rate, std::size_t burst);
			/** @brief Install an exact component/level/group rule. */
			virtual Log& Throttle(ComponentManip component, const Level& level, GroupManip group, double rate, std::size_t burst, ThrottlePolicy policy = ThrottlePolicy::Drop, std::size_t value = 0, std::size_t period = 0);
			/** @brief Remove all throttle rules. */
			virtual Log& NoThrottle();
			/** @brief Remove a level-scoped rule. */
			virtual Log& NoThrottle(const Level& level);
			/** @brief Remove a group-scoped rule. */
			virtual Log& NoThrottle(GroupManip group);
			/** @brief Remove a component-scoped rule. */
			virtual Log& NoThrottle(ComponentManip component);
			/** @brief Remove an exact component/level/group rule. */
			virtual Log& NoThrottle(ComponentManip component, const Level& level, GroupManip group);

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
			 * @brief Save the current format and activate a temporary format.
			 * @param manip Format manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(FormatManip manip) {
				Write(manip);
				return *this;
			}
			/**
			 * @brief Restore the most recently saved format.
			 * @param manip Pop-format manipulator.
			 * @return Reference to this logger.
			 */
			inline Log& operator<<(PopFormatManip manip) {
				Write(manip);
				return *this;
			}
			/**
			 * @brief Set the producer group for the current line.
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

		protected:
			std::shared_ptr<Implementation> m_impl; ///< Shared backend (copies of Log share it)

			/**
			 * @brief Whether messages at the current level will be written.
			 * @return true if the current level is at or above the print floor.
			 */
			bool WillWrite() const noexcept;

			/**
			 * @brief Decide throttle admission before a payload claims a line lock.
			 * @return true when the current line may emit.
			 */
			bool PrepareLine();

			/**
			 * @brief Whether output has already started for the current line.
			 * @return true when the line header has been emitted.
			 */
			bool HasOpenOutputLine() const noexcept;

			/**
			 * @name Write
			 * Forward a value or state change to Implementation.
			 */
			//@{
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
			/**
			 * @brief Forward a color manipulator.
			 * @param manip Color manipulator.
			 */
			virtual void Write(ColorManip manip);
			/**
			 * @brief Forward a no-color manipulator.
			 * @param manip No-color manipulator.
			 */
			virtual void Write(NoColorManip manip);
			/**
			 * @brief Forward a push-format manipulator.
			 * @param manip Format manipulator.
			 */
			virtual void Write(FormatManip manip);
			/**
			 * @brief Forward a pop-format manipulator.
			 * @param manip Pop-format manipulator.
			 */
			virtual void Write(PopFormatManip manip);
			/**
			 * @brief Forward a group manipulator.
			 * @param manip Group manipulator.
			 */
			virtual void Write(GroupManip manip);
			/**
			 * @brief Forward a component manipulator.
			 * @param manip Component manipulator.
			 */
			virtual void Write(ComponentManip manip);
			/**
			 * @brief Forward a reset-component manipulator.
			 * @param manip Reset-component manipulator.
			 */
			virtual void Write(ResetComponentManip manip);
			//@}
	};

	/**
	 * @brief Stream a value into a smart pointer to Log.
	 * @tparam Ptr std::shared_ptr<Log> or std::unique_ptr<Log>.
	 * @tparam T Value type.
	 * @param logger Smart pointer to Log.
	 * @param value Value to stream.
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr, typename T>
	Ptr& operator<<(Ptr& logger, const T& value) noexcept
		requires StormByte::Type::SameAs<Ptr, std::shared_ptr<Log>> || StormByte::Type::SameAs<Ptr, std::unique_ptr<Log>> {
		if (logger)
			*logger << value;
		return logger;
	}

	/**
	 * @brief Stream a Level into a smart pointer to Log.
	 * @tparam Ptr std::shared_ptr<Log> or std::unique_ptr<Log>.
	 * @param logger Smart pointer to Log.
	 * @param level Level to set.
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, const Level& level) noexcept
		requires StormByte::Type::SameAs<Ptr, std::shared_ptr<Log>> || StormByte::Type::SameAs<Ptr, std::unique_ptr<Log>> {
		if (logger)
			*logger << level;
		return logger;
	}

	/**
	 * @brief Stream a stream manipulator into a smart pointer to Log.
	 * @tparam Ptr std::shared_ptr<Log> or std::unique_ptr<Log>.
	 * @param logger Smart pointer to Log.
	 * @param manip Stream manipulator (e.g. std::endl).
	 * @return Reference to the smart pointer.
	 */
	template <typename Ptr>
	Ptr& operator<<(Ptr& logger, std::ostream& (*manip)(std::ostream&)) noexcept
		requires StormByte::Type::SameAs<Ptr, std::shared_ptr<Log>> || StormByte::Type::SameAs<Ptr, std::unique_ptr<Log>> {
		if (logger)
			*logger << manip;
		return logger;
	}
}
