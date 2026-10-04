cmake_minimum_required(VERSION 3.14)
project(stripe_bug_bash CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GoogleTest automatically
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Library target
add_library(ledger_lib src/ledger.cpp)
target_include_directories(ledger_lib PUBLIC include)

# Test target
enable_testing()
add_executable(ledger_tests tests/ledger_test.cpp)
target_link_libraries(ledger_tests ledger_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ledger_tests)
