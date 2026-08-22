cmake_minimum_required(VERSION 3.21)
project(matching_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings live on an INTERFACE target: the targets link it, but third-party code never sees it
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Werror)

# Sanitizers: opt in with -DENABLE_SANITIZERS=ON (the correctness preset sets it).
option(ENABLE_SANITIZERS "Build with AddressSanitizer + UBSan" OFF)
add_library(project_sanitizers INTERFACE)
if(ENABLE_SANITIZERS)
  target_compile_options(project_sanitizers INTERFACE
      -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(project_sanitizers INTERFACE
      -fsanitize=address,undefined)
endif()

# --- The engine library ---
add_library(orderbook
    src/orderbook.cpp
    src/types.cpp
)
target_include_directories(orderbook PUBLIC include)
target_link_libraries(orderbook
    PRIVATE project_warnings
    PUBLIC  project_sanitizers)

# --- Demo ---
add_executable(orderbook_demo src/main.cpp)
target_link_libraries(orderbook_demo PRIVATE orderbook project_warnings)

# --- Tests ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(orderbook_tests tests/test_orderbook.cpp tests/test_types.cpp tests/test_reference_book.cpp tests/test_differential.cpp)
target_link_libraries(orderbook_tests
    PRIVATE orderbook project_warnings GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(orderbook_tests)
