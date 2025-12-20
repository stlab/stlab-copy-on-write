# stlab::copy_on_write

[![CI][ci-badge]][ci-link]
[![Documentation][docs-badge]][docs-link]
[![License][license-badge]][license-link]

Copy-on-write wrapper for any type.

[ci-badge]: https://github.com/stlab/stlab-copy-on-write/workflows/CI/badge.svg
[ci-link]: https://github.com/stlab/stlab-copy-on-write/actions/workflows/ci.yml
[docs-badge]: https://img.shields.io/badge/docs-github%20pages-blue
[docs-link]: https://stlab.github.io/stlab-copy-on-write/
[license-badge]: https://img.shields.io/badge/license-BSL%201.0-blue.svg
[license-link]: https://github.com/stlab/stlab-copy-on-write/blob/main/LICENSE

## Overview

**Online Documentation:**
The latest documentation is automatically built and deployed to [GitHub Pages](https://stlab.github.io/stlab-copy-on-write/) on every push to the main branch.

**Version Management:**
The project version is maintained in a single location in the `project()` command at the top of
`CMakeLists.txt`. When creating a new release:

1. Update the version in `project(stlab-copy-on-write VERSION X.Y.Z)`
2. The version will automatically propagate to:
   - CMake package configuration
   - Doxygen documentation
   - GitHub release

`stlab::copy_on_write<T>` is a smart pointer that implements copy-on-write semantics. It allows multiple instances to share the same underlying data until one of them needs to modify it, at which point a copy is made. This can provide significant performance benefits when dealing with expensive-to-copy objects that are frequently copied but rarely modified.

## Features

- **Thread-safe**: Uses atomic reference counting for safe concurrent access
- **Header-only**: No compilation required, just include the header
- **C++17**: Leverages modern C++ features for clean, efficient implementation

## Examples

The project includes executable examples that demonstrate the library's functionality:

- **`example/basic_usage.cpp`**: Comprehensive demonstration of copy-on-write semantics, identity checking, and swap operations

Examples are automatically built and run as part of the test suite to ensure they remain up-to-date and functional. They are also included in the generated documentation.

## Building and Testing

This library uses CMake with CPM for dependency management:

```bash
# Configure and build
cmake --preset=test
cmake --build --preset=test

# Run tests
ctest --preset=test
```

### Including in Your Project

To include this library in your project using CPM:

```cmake
CPMAddPackage("gh:stlab/stlab-copy-on-write@X.Y.Z")

target_link_libraries(your_target PRIVATE stlab::copy-on-write)
```

The library's headers will be available under the `stlab/` directory:

```cpp
#include <stlab/copy_on_write.hpp>
```

### Building Documentation

Documentation is generated using Doxygen with the modern [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) theme:

```bash
# Configure and build documentation
cmake --preset=docs
cmake --build --preset=docs

# View documentation (opens in browser)
open build/docs/html/index.html
```

**Requirements for documentation:**

- Doxygen 1.9.1 or later

To update the `Doxyfile.in` with a newer version of Doxygen use the following command:

```bash
doxygen -u docs/Doxyfile.in
```

## Continuous Integration

This project uses GitHub Actions for continuous integration, automatically:

- **Testing**: Builds and tests on Ubuntu (GCC/Clang), macOS (Clang), and Windows (MSVC)
- **Documentation**: Builds and deploys documentation to GitHub Pages on main branch pushes

All pull requests must pass CI checks before merging.

## Requirements

- C++17 compatible compiler
- CMake 3.20 or later (for building tests)

## Dependencies

This project uses the following external dependencies:

| Dependency              | Repository                                                                      | File Location     | Update Instructions                            |
| ----------------------- | ------------------------------------------------------------------------------- | ----------------- | ---------------------------------------------- |
| **CMake**               | [Kitware/CMake](https://github.com/Kitware/CMake)                               | `CMakeLists.txt`  | Update `VERSION` in `cmake_minimum_required()` |
| **CPM.cmake**           | [cpm-cmake/CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)                   | `CMakeLists.txt`  | Update version in download URL and SHA256 hash |
| **doctest**             | [doctest/doctest](https://github.com/doctest/doctest)                           | `CMakeLists.txt`  | Update `GIT_TAG` in `CPMAddPackage` call       |
| **Doxygen**             | [doxygen/doxygen](https://github.com/doxygen/doxygen)                           | Optional for docs | Install via package manager or from source     |
| **doxygen-awesome-css** | [jothepro/doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) | `CMakeLists.txt`  | Update `GIT_TAG` in `CPMAddPackage` call       |

### Updating Dependencies

#### CMake

To install or update CMake:

##### Option 1: Official Installer

1. Visit the [CMake download page](https://cmake.org/download/)
2. Download the installer for your platform
3. Follow the installation instructions

##### Option 2: Package Manager

- **macOS (Homebrew)**: `brew install cmake`
- **Ubuntu/Debian**: `sudo apt update && sudo apt install cmake`
- **Windows (Chocolatey)**: `choco install cmake`
- **Windows (Scoop)**: `scoop install cmake`

**Verify Installation**: `cmake --version`

#### CPM.cmake

To update CPM.cmake:

1. Visit the [CPM.cmake releases page](https://github.com/cpm-cmake/CPM.cmake/releases)
2. Find the desired version and copy the download URL
3. Update the version in the URL on line 11 of `CMakeLists.txt`
4. Update the SHA256 hash on line 13 (found in the release assets)

**CPM Caching**: This project enables CPM's caching feature to avoid re-downloading dependencies.
The cache is stored in `.cpm-cache/` and is automatically ignored by git. To customize the cache
location, set the `CPM_SOURCE_CACHE` environment variable or CMake variable.

#### doctest

To update doctest:

1. Visit the [doctest releases page](https://github.com/doctest/doctest/releases)
2. Find the desired version tag (e.g., `v2.4.12`)
3. Update the `GIT_TAG` value on line 21 of `CMakeLists.txt`

#### Doxygen

To install or update Doxygen:

##### Option 1: Package Manager

- **macOS (Homebrew)**: `brew install doxygen`
- **Ubuntu/Debian**: `sudo apt update && sudo apt install doxygen`
- **Windows (Chocolatey)**: `choco install doxygen.install`
- **Windows (Scoop)**: `scoop install doxygen`

##### Option 2: Official Installer

1. Visit the [Doxygen download page](https://www.doxygen.nl/download.html)
2. Download the installer for your platform
3. Follow the installation instructions

**Verify Installation**: `doxygen --version`

#### doxygen-awesome-css

This dependency is automatically downloaded via CPM when building documentation.
To update the theme version, update the `GIT_TAG` value in the `CPMAddPackage` call in `CMakeLists.txt`.

## License

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE.txt or copy at <http://www.boost.org/LICENSE_1_0.txt>
