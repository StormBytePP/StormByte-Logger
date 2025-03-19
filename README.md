# StormByte

StormByte is a comprehensive, cross-platform C++ library aimed at easing system programming, configuration management, logging, and database handling tasks. This library provides a unified API that abstracts away the complexities and inconsistencies of different platforms (Windows, Linux).

## Features

- **Logging**: Supports various logging levels and outputs, including file-based logging.

## Table of Contents

- [Repository](#Repository)
- [Installation](#Installation)
- [Modules](#Modules)
	- [Base](https://dev.stormbyte.org/StormByte)
	- [Config](https://dev.stormbyte.org/StormByte-Config)
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

StormByte Library is composed by several modules:

### Logger

The `Logger` module provides a comprehensive logging framework with support for different logging levels and outputs.

#### Example: Log

```cpp
#include <logger/logger.hxx>
#include <iostream>

using namespace StormByte::Logger;

// Example usage
int main() {
	// Simple logger outputing only errors to stdout
	Logger logger(std::cout, Level::Error);
	logger << Level::Info << "This is an info message"; // Will not be displayed
	logger << Level::Error << "This is an error message"; // Will be displayed
	return 0;
}
```

## Contributing

Contributions are welcome! Please fork the repository and submit pull requests for any enhancements or bug fixes.

## License

This project is licensed under GPL v3 License - see the [LICENSE](LICENSE) file for details.
