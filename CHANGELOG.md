# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte Logger is the stream-logging module of the StormByte C++ suite.

It depends on [StormByte Base 1.2.0](https://github.com/StormBytePP/StormByte/releases/tag/1.2.0) or newer. This repository is not Base, Buffer, Config, Crypto, Database, Multimedia, Network or System.

Public headers under `StormByte/logger/` cover `Log`, `ThreadedLog`, header formats (`%L` `%T` `%i` `%c` `%g`), components, groups, ANSI colors, temporary formats, human-readable numbers and bytes, redaction of text and numbers, hex dumps (`hex` / `nohex`), and binary payloads (`std::span<const std::byte>`, default Base64).

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte-Logger/blob/master/README.md)
- License: GNU Lesser General Public License version 3 or later, [LICENSE](https://github.com/StormBytePP/StormByte-Logger/blob/master/LICENSE)

## [Unreleased]

### Added

- `Log::Enabled(Level)`: print-floor query (Warning/Error/Fatal always true). Does not open a line and does not consult throttle.
- `operator<<(std::string_view)` and `operator<<(std::wstring_view)` on `Log` and `ThreadedLog`, with the same filtered early-out as other payloads. `std::string` / `std::wstring` convert to the views.
- `hex` / `hex(N)` / `nohex`: dump subsequent payloads as spaced `0xAA` bytes. `N` is bytes per row (default 16); wrap uses a raw newline without a new header and without ending the logical line. `hex(0)` is `nohex`. Applies to text, wide text (after UTF-8), numbers (`42` → bytes of `"42"`) and binary spans. Hex runs before redaction.
- `operator<<(std::span<const std::byte>)` on `Log` and `ThreadedLog`. Default output is Base64 (`StormByte::Base64Encode`). `std::vector<std::byte>` converts to the span. With `hex` the dump is the raw bytes, not the Base64 text. Empty spans emit an empty payload. `ThreadedLog` formats the payload before taking the line lock (`FormatBinary` + `WritePrepared`).

### Changed

- Dropped the dedicated `operator<<(const std::string&)` / `operator<<(const std::wstring&)` overloads. Call sites that pass `std::string` still compile.
- `ThreadedLog` wide payloads encode with `String::UTF8Encode(std::wstring_view)` before taking the line lock.
- Bundled StormByte Base is [1.2.0](https://github.com/StormBytePP/StormByte/releases/tag/1.2.0). Using `UTF8Encode(std::wstring_view)` and `Base64Encode(std::span<const std::byte>)` requires Base 1.2.0 or newer.
- **Breaking:** `noredact` is now `noredact`, same shape as `nocolor`, `nohex` and `nohumanreadable`. There is no compatibility alias.

### Fixed

- `~Implementation` no longer first-touches thread-local line state (Valgrind still-reachable TLS at exit).

## [1.1.1] - 2026-09-15

### Changed

- Bundled StormByte Base is [1.1.1](https://github.com/StormBytePP/StormByte/releases/tag/1.1.1). The declared requirement stays [1.1.0](https://github.com/StormBytePP/StormByte/releases/tag/1.1.0) or newer.

[1.1.1]: https://github.com/StormBytePP/StormByte-Logger/compare/1.1.0...1.1.1

## [1.1.0] - 2026-09-13

### Added

- `group("name")` and `%g`; the group is cleared by newline and is not rendered in the body when `%g` is absent.
- Sticky per-thread `component("name")`, `reset_component` and `%c`. `component("")` selects the root without throwing, while `reset_component` is the canonical reset. The state belongs to the thread, not the `Log`; two `Log` instances on one thread observe the same component.
- ANSI colors by `Level` and component override, with `color`, `color(Color::X)` and `nocolor`. Color output is disabled by default.
- Persistent general and component formats, plus nested `push_format` / `pop_format`; an empty `pop_format` is idempotent.
- Expanded tests for colors, temporary formats, groups, components, threads, Unicode and lock recovery.
- Added configurable line throttle policies (`Drop`, `Sample`, `Window`) with component/level/group rule precedence and drop summaries.
- Added `FlushThrottle()` and selective `FlushThrottle(spec)` for pending drop summaries at job boundaries.

### Changed

- Requires [StormByte Base 1.1.0](https://github.com/StormBytePP/StormByte/releases/tag/1.1.0) or newer (`Exception` with `Component`). Logger did not use the two-string `Exception` constructor.
- `Warning`, `Error` and `Fatal` are always emitted, even when the configured floor is higher. This is intentional.
- Expanded `ThreadedLog` coverage for the filtered hot path, wide conversion before locking, and recovery when Unicode conversion fails.

### Fixed

- `ThreadedLog::FlushThrottle` preserves ownership of an already-held line lock.
- Format changes now reset the active throttle line snapshot after closing an open line.
- `endl` releases the `ThreadedLog` line lock even if `WillWrite()` changes midway through a line.
- Wide-string logging converts before acquiring the lock; a Unicode conversion error no longer terminates the logger from an internal `noexcept` path.
- **ThreadedLog::Write(Level)** — two threads sharing a `ThreadedLog` raced on `Implementation`’s `optional<Color>` (`m_content_color.reset()` at the start of a line). The level token now takes the line lock like other `Write` overloads; payload and `endl` already did.

[1.1.0]: https://github.com/StormBytePP/StormByte-Logger/releases/tag/1.1.0

## [1.0.0] - 2026-09-05

Initial public release of StormByte Logger.

### Added

- `Log` streaming facade with `operator<<`
- Level filter: `LowLevel`, `Debug`, `Warning`, `Notice`, `Info`, `Error`, `Fatal`
- Header format: `%L`, `%T`, `%i`, `%%`
- Manipulators: `humanreadable_number`, `humanreadable_bytes`, `nohumanreadable`
- Redaction: `redact` / `redact(N)` keep last N; `redact_first(N)` keep first N; `no_redact`; applies to text and numbers
- `ThreadedLog`: one lock per logical line; filtered messages do not take the lock
- Uses [StormByte Base 1.0.0](https://github.com/StormBytePP/StormByte/releases/tag/1.0.0) (`String`, `ThreadLock`, platform)
- Unit tests (filter, threads, redaction)
- Project version read from the `VERSION` file
- CMake 3.28 floor

### Notes

- `Log` is not thread-safe. Use `ThreadedLog` when several threads share one logger.
- Messages below the print level return early.
- Needs a C++26 compiler and [StormByte Base 1.0.0](https://github.com/StormBytePP/StormByte/releases/tag/1.0.0).

[1.0.0]: https://github.com/StormBytePP/StormByte-Logger/releases/tag/1.0.0
