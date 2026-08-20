# StormByte
![Linux](https://img.shields.io/badge/Linux-Supported-1793D1?logo=linux&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-Supported-0078D6?logo=windows&logoColor=white)
![macOS](https://img.shields.io/badge/macOS-Supported-000000?logo=apple&logoColor=white)
![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.12+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte-Logger/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte-Logger/actions/workflows/ci.yml)

StormByte is a comprehensive, cross-platform C++ library aimed at easing system programming, configuration management, logging, and database handling tasks. This library provides a unified API that abstracts away the complexities and inconsistencies of different platforms (Windows, Linux).

## Features

- **Buffer Operations**: FIFO buffers, thread-safe shared buffers, producer-consumer interfaces, and pipelines

## Table of Contents

- [Repository](#Repository)
- [Installation](#Installation)
- [Modules](#Modules)
	- [Base](https://dev.stormbyte.org/StormByte)
	- **Buffer**
	- [Config](https://dev.stormbyte.org/StormByte-Config)
	- [Crypto](https://dev.stormbyte.org/StormByte-Crypto)
	- [Database](https://dev.stormbyte.org/StormByte-Database)
	- **Logger**
	- [Multimedia](https://dev.stormbyte.org/StormByte-Multimedia)
	- [Network](https://dev.stormbyte.org/StormByte-Network)
	- [System](https://dev.stormbyte.org/StormByte-System)
- [Contributing](#Contributing)
- [License](#License)

## Repository

You can visit the code repository at [GitHub](https://github.com/StormBytePP/StormByte-Logger)

## Installation

### Prerequisites

Ensure you have the following installed:

- C++23 compatible compiler
- CMake 3.12 or higher

### Building

To build the library, follow these steps:

```sh
git clone https://github.com/StormBytePP/StormByte-Logger.git
cd StormByte-Logger
mkdir build
cd build
cmake ..
make
```

## Modules

### Logger

Streaming logger with level filtering, customizable headers, human-readable numeric formatting, optional redaction of sensitive text, and a thread-safe variant.

#### Basics

```cpp
#include <StormByte/logger/log.hxx>
#include <StormByte/logger/threaded_log.hxx>
#include <StormByte/logger/manipulators.hxx>

using namespace StormByte::Logger;

Log log(std::cout, Level::Info, "[%L] %T");
log << Level::Info << "hello" << std::endl;

// Header placeholders: %L level, %T timestamp, %i thread id, %% literal %
ThreadedLog tlog(std::cout, Level::Debug, "[%L %i] %T");
```

#### Human-readable numbers

```cpp
log << Level::Info << humanreadable_number << 1000 << std::endl;   // e.g. 1,000
log << Level::Info << humanreadable_bytes << 10240 << std::endl;  // e.g. 10 KiB
log << Level::Info << nohumanreadable << 1000 << std::endl;       // raw again
```

State persists until another human-readable manipulator (or `nohumanreadable`) is applied.

#### Redaction

Mask string-like values (`std::string`, `const char*`, wide strings) while logging. Numbers and booleans are not redacted. Policy stays active until `no_redact`.

| Manipulator | Effect |
|-------------|--------|
| `redact` / `redact(0)` | Replace every character with `*` (same length) |
| `redact(N)` | Keep the **first N** characters; mask the rest with `*` |
| `no_redact` | Disable redaction |

```cpp
log << Level::Info << redact << "super-secret" << std::endl;
// ... ************

log << Level::Info << redact(4) << "super-secret" << std::endl;
// ... supe********

log << Level::Info << no_redact << "visible again" << std::endl;
```

Works the same on `ThreadedLog`. Safe for tokens, passwords, and other sensitive text in log lines without changing call sites beyond the manipulator.

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests for any enhancements or bug fixes.

## License

This project is licensed under LGPL v3 License - see the [LICENSE](LICENSE) file for details.
