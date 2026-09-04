# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte Logger is the stream-logging module of the StormByte C++ suite.

It depends on StormByte Base. This repository is not Base, Buffer, Config, Crypto, Database, Multimedia, Network or System.

Public headers under `StormByte/logger/` cover `Log`, `ThreadedLog`, header formats (`%L` `%T` `%i`), human-readable numbers and bytes, and redaction of text and numbers.

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte-Logger/blob/master/README.md)
- License: GNU Lesser General Public License version 3 or later, [LICENSE](https://github.com/StormBytePP/StormByte-Logger/blob/master/LICENSE)

## [1.0.0] - 2026-09-04

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
