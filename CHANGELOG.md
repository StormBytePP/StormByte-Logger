# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte Logger is the stream-logging module of the StormByte C++ suite.

It depends on StormByte Base. This repository is not Base, Buffer, Config, Crypto, Database, Multimedia, Network or System.

Public headers under `StormByte/logger/` cover `Log`, `ThreadedLog`, header formats (`%L` `%T` `%i` `%c` `%g`), components, groups, ANSI colors, temporary formats, human-readable numbers and bytes, and redaction of text and numbers.

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte-Logger/blob/master/README.md)
- License: GNU Lesser General Public License version 3 or later, [LICENSE](https://github.com/StormBytePP/StormByte-Logger/blob/master/LICENSE)

## [Unreleased]

[Unreleased]: https://github.com/StormBytePP/StormByte-Logger/compare/1.1.0...HEAD

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

- Requires StormByte Base ≥ 1.1.0 (`Exception` with `Component`). Logger did not use the two-string `Exception` constructor.
- `Warning`, `Error` and `Fatal` are always emitted, even when the configured floor is higher. This is intentional.
- Expanded `ThreadedLog` coverage for the filtered hot path, wide conversion before locking, and recovery when Unicode conversion fails.

### Fixed

- `ThreadedLog::FlushThrottle` preserves ownership of an already-held line lock.
- Format changes now reset the active throttle line snapshot after closing an open line.
- `endl` releases the `ThreadedLog` line lock even if `WillWrite()` changes midway through a line.
- Wide-string logging converts before acquiring the lock; a Unicode conversion error no longer terminates the logger from an internal `noexcept` path.

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
- Uses StormByte Base (`String`, `ThreadLock`, platform)
- Unit tests (filter, threads, redaction)
- Project version read from the `VERSION` file
- CMake 3.28 floor

### Notes

- `Log` is not thread-safe. Use `ThreadedLog` when several threads share one logger.
- Messages below the print level return early.
- Needs a C++26 compiler and StormByte Base ≥ 1.0.0.

[1.0.0]: https://github.com/StormBytePP/StormByte-Logger/releases/tag/1.0.0
