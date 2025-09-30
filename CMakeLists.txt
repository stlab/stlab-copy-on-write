cmake_minimum_required(VERSION 3.20)

# Project declaration - cpp_library_setup will use this name and detect version from git tags
project(stlab-copy-on-write)

set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/.cache/cpm CACHE PATH "CPM cache")
include(cmake/CPM.cmake)

# Fetch cpp-library via CPM

# https://github.com/stlab/cpp-library
# CPMAddPackage(
#     NAME cpp-library
#     URL "${CMAKE_SOURCE_DIR}/../cpp-library"
# )
CPMAddPackage("gh:stlab/cpp-library@4.0.1")
include(${cpp-library_SOURCE_DIR}/cpp-library.cmake)

# Let cpp-library handle the project declaration and version detection
cpp_library_setup(
    DESCRIPTION "Copy-on-write wrapper for any type"
    NAMESPACE stlab
    HEADERS copy_on_write.hpp
    EXAMPLES basic_usage_test.cpp
    TESTS copy_on_write_tests.cpp
)
