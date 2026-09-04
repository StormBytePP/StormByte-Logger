# StormByte-Logger

![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte-Logger/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte-Logger/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github)](https://github.com/sponsors/StormBytePP)

Streaming logger for the StormByte C++ suite: level filtering, custom headers, human-readable numbers, redaction, and a thread-safe facade.

This repository is **only** the Logger module. It depends on [StormByte Base](https://github.com/StormBytePP/StormByte). The rest of the suite lives in sibling repos.

## Features

- Stream API (`operator<<`) with `Level` filtering
- Header format: `%L` level, `%T` timestamp, `%i` thread id, `%%` literal `%`
- `humanreadable_number` / `humanreadable_bytes` / `nohumanreadable`
- Redaction of text **and** numbers (`redact`, `redact(N)`, `redact_first(N)`, `no_redact`)
- `ThreadedLog`: one lock per logical line; filtered messages do not take the lock

## Table of Contents

- [Repository](#repository)
- [Installation](#installation)
- [Modules](#modules)
  - [Base](https://dev.stormbyte.org/StormByte)
  - [Buffer](https://dev.stormbyte.org/StormByte-Buffer)
  - [Config](https://dev.stormbyte.org/StormByte-Config)
  - [Crypto](https://dev.stormbyte.org/StormByte-Crypto)
  - [Database](https://dev.stormbyte.org/StormByte-Database)
  - **Logger**
  - [Multimedia](https://dev.stormbyte.org/StormByte-Multimedia)
  - [Network](https://dev.stormbyte.org/StormByte-Network)
  - [System](https://dev.stormbyte.org/StormByte-System)
- [Contributing](#contributing)
- [License](#license)

## Repository

Source: [GitHub](https://github.com/StormBytePP/StormByte-Logger)

## Installation

### Prerequisites

- C++26 compiler
- CMake 3.28 or newer
- [StormByte Base](https://github.com/StormBytePP/StormByte) (submodule / BuildMaster)

### Building

```sh
git clone --recursive https://github.com/StormBytePP/StormByte-Logger.git
cd StormByte-Logger
cmake -S . -B build -G Ninja
cmake --build build
```

## Modules

### Logger

```cpp
#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/logger/manipulators.hxx>

using namespace StormByte::Logger;

Log log(std::cout, Level::Info, "[%L] %T");
log << Level::Info << "hello" << std::endl;

ThreadedLog tlog(std::cout, Level::Debug, "[%L %i] %T");
```

#### Human-readable numbers

State stays until another of these manipulators is applied.

```cpp
log << Level::Info << humanreadable_number << 1000 << std::endl;   // e.g. 1,000
log << Level::Info << humanreadable_bytes << 10240 << std::endl;  // e.g. 10 KiB
log << Level::Info << nohumanreadable << 1000 << std::endl;
```

#### Redaction

Applies to strings **and** numbers (numbers are converted first). Stays on until `no_redact`.

| Manipulator | Effect |
|-------------|--------|
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

Fork and open a pull request. Issues only (no Wiki / Discussions).

## License

LGPLv3 or later. See [LICENSE](LICENSE).
