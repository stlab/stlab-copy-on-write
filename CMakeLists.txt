cmake_minimum_required(VERSION 3.24)

include(cmake/CPM.cmake)

# Fetch cpp-library before project()
# Check https://github.com/stlab/cpp-library/releases for the latest version
CPMAddPackage("gh:stlab/cpp-library@5.2.0")
include(${cpp-library_SOURCE_DIR}/cpp-library.cmake)

# Enable dependency tracking before project()
cpp_library_enable_dependency_tracking()

# Now declare project
project(copy-on-write)

# Enable testing infrastructure (required for TESTS and EXAMPLES)
include(CTest)

# Let cpp-library handle the project declaration and version detection
cpp_library_setup(
    DESCRIPTION "Copy-on-write wrapper for any type"
    NAMESPACE stlab
    HEADERS copy_on_write.hpp
    EXAMPLES basic_usage_test.cpp
    TESTS copy_on_write_tests.cpp
)
