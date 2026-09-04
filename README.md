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

- **Log** — `operator<<` facade with a minimum print `Level`.
- **Levels** — `LowLevel`, `Debug`, `Warning`, `Notice`, `Info`, `Error`, `Fatal`.
- **Headers** — `%L` level, `%T` timestamp, `%i` thread id, `%%` literal `%`.
- **Human-readable** — `humanreadable_number`, `humanreadable_bytes`, `nohumanreadable` (state sticks until the next one).
- **Redaction** — text **and** numbers: `redact` / `redact(N)` keep last N, `redact_first(N)` keep first N, `no_redact`.
- **ThreadedLog** — one lock per logical line; messages below the print level never take the lock.
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
- [Installation](#installation)
- [Usage](#usage)
  - [Log and ThreadedLog](#log-and-threadedlog)
  - [Human-readable numbers](#human-readable-numbers)
  - [Redaction](#redaction)
- [Contributing](#contributing)
- [License](#license)

## Installation

Needs a C++26 compiler, CMake 3.28 or newer, and [StormByte Base](https://github.com/StormBytePP/StormByte) ≥ 1.0.0.

```sh
git clone --recursive https://github.com/StormBytePP/StormByte-Logger.git
cd StormByte-Logger
cmake -S . -B build
cmake --build build
```

## Usage

Headers are `#include <StormByte/logger/….hxx>`. Namespace root is `StormByte::Logger`.

### Log and ThreadedLog

```cpp
#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/logger/manipulators.hxx>

using namespace StormByte::Logger;

Log log(std::cout, Level::Info, "[%L] %T");
log << Level::Info << "hello" << std::endl;

ThreadedLog tlog(std::cout, Level::Debug, "[%L %i] %T");
```

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

## Contributing

Issues only on this repository. Fork and open a pull request against `master`.

## License

GNU Lesser General Public License version 3 or later. See [LICENSE](LICENSE) and <https://www.gnu.org/licenses/lgpl-3.0.html>.
