# StormByte

![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte-Logger/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte-Logger/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-StormBytePP-ea4aaa?logo=githubsponsors)](https://github.com/sponsors/StormBytePP)

This repository is **StormByte Logger**: stream logging for the StormByte C++ suite.

It depends on [StormByte Base](https://github.com/StormBytePP/StormByte). Public headers live under `StormByte/logger/` and cover `Log`, `ThreadedLog`, header formats, human-readable numbers and redaction.

The suite is split on purpose. Base, Buffer, Config, Crypto, Database, Multimedia, Network and System are **other repositories**. This one does not implement them.

## What this module does

- **Log** — `operator<<` facade with a minimum print `Level`. Copies share the same backend.
- **Levels** — ordered from least to most severe: `LowLevel`, `Debug`, `Warning`, `Notice`, `Info`, `Error`, `Fatal`.
- **Headers** — `%L` level, `%T` timestamp, `%i` thread id, `%%` literal `%`.
- **Human-readable** — `humanreadable_number`, `humanreadable_bytes`, `nohumanreadable` (state sticks until the next one).
- **Redaction** — text **and** numbers: `redact` / `redact(N)` keep last N, `redact_first(N)` keep first N, `no_redact`.
- **ThreadedLog** — one lock per logical line (held until a newline manipulator). Messages below the print floor do not take the lock on payload writes.
- **Not thread-safe** — plain `Log` is single-threaded. Share a logger across threads only via `ThreadedLog`.

## The rest of the suite

| Module | Role | API |
| --- | --- | --- |
| [Base](https://github.com/StormBytePP/StormByte) | Exceptions, Expected, serialization, strings, UUID, concepts | [/StormByte](https://dev.stormbyte.org/StormByte) |
| [Buffer](https://github.com/StormBytePP/StormByte-Buffer) | FIFO, SharedFIFO, Ring, Producer/Consumer and multi-stage pipelines | [/StormByte-Buffer](https://dev.stormbyte.org/StormByte-Buffer) |
| [Config](https://github.com/StormBytePP/StormByte-Config) | Human-readable text and versioned binary documents (groups, lists, raw bytes) | [/StormByte-Config](https://dev.stormbyte.org/StormByte-Config) |
| [Crypto](https://github.com/StormBytePP/StormByte-Crypto) | Hash, compress, encrypt, sign and key agreement — Crypto++ never leaves the private tree | [/StormByte-Crypto](https://dev.stormbyte.org/StormByte-Crypto) |
| [Database](https://github.com/StormBytePP/StormByte-Database) | One API over SQLite, PostgreSQL and MariaDB | [/StormByte-Database](https://dev.stormbyte.org/StormByte-Database) |
| **Logger** | This repository | [/StormByte-Logger](https://dev.stormbyte.org/StormByte-Logger) |
| [Multimedia](https://github.com/StormBytePP/StormByte-Multimedia) | Decode, encode and containers without raw FFmpeg types; codecs enabled only if present | [/StormByte-Multimedia](https://dev.stormbyte.org/StormByte-Multimedia) |
| [Network](https://github.com/StormBytePP/StormByte-Network) | Framed packets, Client/Server, IPv4/IPv6 TCP and Buffer pipelines (compress/encrypt) | [/StormByte-Network](https://dev.stormbyte.org/StormByte-Network) |
| [System](https://github.com/StormBytePP/StormByte-System) | Processes, pipes and environment variables across Linux, Windows and macOS | [/StormByte-System](https://dev.stormbyte.org/StormByte-System) |

## Table of Contents

- [What this module does](#what-this-module-does)
- [The rest of the suite](#the-rest-of-the-suite)
- [Documentation](#documentation)
- [Levels](#levels)
- [Headers](#headers)
- [Installation](#installation)
- [Usage](#usage)
  - [Log and ThreadedLog](#log-and-threadedlog)
  - [A line](#a-line)
  - [Sharing a logger](#sharing-a-logger)
  - [Human-readable numbers](#human-readable-numbers)
  - [Redaction](#redaction)
  - [Use from other suite modules](#use-from-other-suite-modules)
- [ThreadedLog contract](#threadedlog-contract)
- [Contributing](#contributing)
- [License](#license)

## Documentation

- This README: how to build, levels, headers, streaming contract, examples.
- Doxygen class reference (headers under `StormByte/logger/`): [https://dev.stormbyte.org/StormByte-Logger/](https://dev.stormbyte.org/StormByte-Logger/).

Other modules that take a `std::shared_ptr<StormByte::Logger::Log>` (Multimedia pipeline steps, FFmpeg filter writers) should point here rather than re-document the logger. The print floor is chosen by the **application**, not by the library that logs.

## Levels

`StormByte::Logger::Level` is the only severity type. It is used twice:

1. **Print floor** — constructor argument. Nothing *below* that value is written.
2. **Message level** — first `operator<<(Level)` on a line. That sets the level of everything until the next `Level` or the end of the line.

Order is **least severe → most severe** (this is not syslog):

| Level | Value | When to use |
| --- | --- | --- |
| `LowLevel` | 0 | High-volume diagnostics: per-unit PTS/DTS, wait/wake, hopper chatter. Expect slowness if the floor is this low. |
| `Debug` | 1 | Useful but quieter: binds, reserves, work summaries, codec open. |
| `Warning` | 2 | Recoverable problems that did not fail the job. |
| `Notice` | 3 | Significant *normal* events: created, opened path, eof, closed. Keep this quiet. |
| `Info` | 4 | Job-level information (for example Multimedia `Transcoder: done`). |
| `Error` | 5 | Error conditions. |
| `Fatal` | 6 | Unrecoverable errors. |

A message is emitted when `message_level >= print_floor`.

Examples:

- Floor `Info` prints `Info`, `Error`, `Fatal`. It does **not** print `Notice`, `Warning`, `Debug` or `LowLevel`.
- Floor `Debug` prints `Debug` and everything more severe (`Warning` … `Fatal`). It does **not** print `LowLevel`.
- Floor `LowLevel` prints everything.

`LevelToString(Level)` returns the short name used in `%L` (`"LowLevel"`, `"Debug"`, `"Notice"`, …). The header pads the name to 8 characters.

Payload `operator<<` for values returns immediately when the current message is below the floor. Setting a `Level`, applying a manipulator, or writing `std::endl` is still forwarded so logger state stays consistent.

## Headers

Third constructor argument. Specifiers:

| Token | Meaning |
| --- | --- |
| `%L` | Current message level, padded to 8 characters |
| `%T` | Local timestamp `dd/mm/YYYY HH:MM:SS` |
| `%i` | `std::this_thread::get_id()` |
| `%%` | A literal `%` |

Default format is `"[%L] %T"`. A typical multi-thread format is `"[%L] %T"` or `"[%L %i] %T"`.

Example with floor `Debug` and format `"[%L] %T"`:

```
[Notice  ] 12/09/2026 06:48:47 STMM Remuxer: created
[Debug   ] 12/09/2026 06:48:47 STMM Demuxer: bind remuxer t=1
```

The logger writes the header once per line, then the payload, then the newline manipulator.

## Installation

Needs a C++26 compiler, CMake 3.28 or newer, and [StormByte Base](https://github.com/StormBytePP/StormByte) ≥ 1.0.0.

```sh
git clone --recursive https://github.com/StormBytePP/StormByte-Logger.git
cd StormByte-Logger
cmake -S . -B build
cmake --build build
```

Link `StormByte-Logger` (and Base). Include path: the public install prefix, headers as `#include <StormByte/logger/….hxx>`.

## Usage

Headers are `#include <StormByte/logger/….hxx>`. Namespace root is `StormByte::Logger`.

### Log and ThreadedLog

```cpp
#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/logger/manipulators.hxx>
#include <iostream>
#include <memory>

using namespace StormByte::Logger;

Log log(std::cout, Level::Info, "[%L] %T");
log << Level::Info << "hello" << std::endl;

auto tlog = std::make_shared<ThreadedLog>(std::cout, Level::Debug, "[%L] %T");
*tlog << Level::Notice << "opened source /tmp/in.mkv" << std::endl;
*tlog << Level::Debug  << "mapped Video 0 -> order 0" << std::endl;
```

`Log` and `ThreadedLog` accept any `std::ostream` (`std::cout`, a file stream, a string stream).

Streamed payload types: `bool`, the standard integer and floating types, `char` / `unsigned char` / `wchar_t`, `const char*`, `const wchar_t*`, `std::string`, `std::wstring`. There is no `std::format` overload on the logger itself; format first, then stream the string.

### A line

A line is:

```cpp
log << Level::Notice << "opened source " << path << std::endl;
```

1. `<< Level` selects the message level and starts (or restarts) the line.
2. Payload writes append to that line.
3. `std::endl` (or any stream manipulator that writes a newline) ends the line, prints the header if needed, and is the point at which `ThreadedLog` drops the line lock.

Do not start a line without a `Level` if you care about the filter. Do not omit the newline: `ThreadedLog` holds the line lock until one is seen.

### Sharing a logger

Copy and copy-assignment of `Log` / `ThreadedLog` share the same `Implementation` (`shared_ptr`). That is the intended way to hand one logger to several objects on **one** thread.

Across threads, construct a `ThreadedLog` (or `std::make_shared<ThreadedLog>`) and pass that pointer. `Log` has no line lock; concurrent `operator<<` will interleave characters.

```cpp
auto log = std::make_shared<ThreadedLog>(std::cout, Level::Notice, "[%L] %T");
Demuxer demux(log);
Muxer   mux(log, container);
```

The objects store `std::shared_ptr<Log>`. `ThreadedLog` *is-a* `Log`, so the same pointer type works.

### Human-readable numbers

State stays until another of these manipulators is applied.

```cpp
log << Level::Info << humanreadable_number << 1000 << std::endl;   // e.g. 1,000
log << Level::Info << humanreadable_bytes << 10240 << std::endl;  // e.g. 10 KiB
log << Level::Info << nohumanreadable << 1000 << std::endl;
```

### Redaction

Applies to strings **and** numbers (numbers are converted first). Stays on until `no_redact`.

| Manipulator | Effect |
| --- | --- |
| `redact` / `redact(0)` | Every character becomes `*` |
| `redact(N)` | Keep the **last** N characters |
| `redact_first(N)` | Keep the **first** N characters |
| `no_redact` | Disable |

```cpp
log << Level::Info << redact << "super-secret" << std::endl;
// ************

log << Level::Info << redact(4) << "super-secret" << std::endl;
// ********cret

log << Level::Info << redact_first(4) << "super-secret" << std::endl;
// supe********

log << Level::Info << no_redact << "visible again" << std::endl;
```

Same contract on `ThreadedLog`.

### Use from other suite modules

Multimedia (and any FFmpeg filter linked against it) logs through this module. Convention used there:

- Prefix the payload with a module tag, then the producer: `STMM Encoder(libx265): finish`.
- `LowLevel` — per-packet / per-frame / wait-wake. Sparse-sample if the volume would drown the log.
- `Debug` — binds, reserves, work `n/min/max`.
- `Notice` — created, open path, eof, closed. Must stay low-noise.
- `Info` — job close only (`Transcoder: done`).

The application chooses the floor. A user who sets `LowLevel` is asking for noise and the cost that comes with it.

## ThreadedLog contract

`ThreadedLog` serializes **logical lines**, not individual `<<` tokens from one thread.

- The line lock is taken when a write that will be printed starts (or when `<< Level` starts a line).
- The lock is dropped when a stream manipulator that writes a newline is applied (`std::endl`).
- Filtered payload writes do not take the lock.
- `<< Level` always updates the current message level. If that level is below the floor, the lock is released immediately after the update.

`endl` must drop the lock even if another thread just changed the current level. That is required so a filtered `LowLevel` line cannot leave the lock held and stall every other writer.

`Implementation` current-level / enabled flags are still process-wide, not `thread_local`. Do not interleave two unfinished lines on the same logger from two threads without finishing each line with a newline. The supported pattern is: one thread writes a complete line (`Level` … `endl`) at a time; `ThreadedLog` only prevents those complete lines from mixing characters.

## Contributing

Issues only on this repository. Fork and open a pull request against `master`.

## License

GNU Lesser General Public License version 3 or later. See [LICENSE](LICENSE) and <https://www.gnu.org/licenses/lgpl-3.0.html>.
