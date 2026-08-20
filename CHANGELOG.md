# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-20

Initial public release of **StormByte-Logger**: a modern, stream-style C++23 logging library with level filtering, custom headers, human-readable formatting, redaction and optional thread safety.

### Added

- `Log` streaming facade with familiar `operator<<` syntax
- Level-based filtering (`LowLevel`, `Debug`, `Warning`, `Notice`, `Info`, `Error`, `Fatal`)
- Customizable header format (`%L` level, `%T` timestamp, `%i` thread id, `%%` literal percent)
- Human-readable number and byte-size formatting manipulators (`humanreadable_number`, `humanreadable_bytes`, `nohumanreadable`)
- Redaction support:
  - `redact` / `redact(n)` → keep last N characters
  - `redact_first(n)` → keep first N characters
  - Applies to both text and numbers
  - `no_redact` to disable
- `ThreadedLog` – thread-safe variant that serializes logical lines (until newline)
- Smart-pointer overloads (`std::shared_ptr` / `std::unique_ptr`)
- Integration with StormByte Base (String utilities, ThreadLock, platform detection)
- Comprehensive unit tests (including high-volume filtered logging, multi-threaded scenarios and redaction)

### Notes

- `Log` instances are not thread-safe. Use `ThreadedLog` when multiple threads write to the same logger.
- Filtered messages (below the configured level) early-out with near-zero overhead.
- Requires a C++23 compliant compiler and StormByte Base ≥ 1.0.0.

[1.0.0]: https://github.com/StormBytePP/StormByte-Logger/releases/tag/1.0.0
