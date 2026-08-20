#include <StormByte/logger/log.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <StormByte/logger/implementation.hxx>

namespace StormByte::Logger {
	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_number(Log& log) noexcept {
		humanreadable_number(*log.m_impl);
		return log;
	}

	STORMBYTE_LOGGER_PUBLIC Log& humanreadable_bytes(Log& log) noexcept {
		humanreadable_bytes(*log.m_impl);
		return log;
	}

	STORMBYTE_LOGGER_PUBLIC Log& nohumanreadable(Log& log) noexcept {
		nohumanreadable(*log.m_impl);
		return log;
	}

	STORMBYTE_LOGGER_PUBLIC Log& no_redact(Log& log) noexcept {
		log.m_impl->SetRedact(false, 0, false);
		return log;
	}
}
