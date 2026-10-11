# Minimum CMake version required for C++20 features
cmake_minimum_required(VERSION 3.20)
# Define project name and C++ language
project(misra_check CXX)
# Enable C++20 language standard
set(CMAKE_CXX_STANDARD 20)
# Require C++20 without fallback
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable compiler-specific extensions for ISO compliance
set(CMAKE_CXX_EXTENSIONS OFF)
# Check if compiler is Microsoft Visual C++
if(MSVC)
    # Enable strict warning level 4, warnings as errors, and standard conforming mode
    add_compile_options(/W4 /WX /permissive-)
# Else handle GCC and Clang compilers
else()
    # Enable all warnings, extra warnings, warnings as errors, and strict pedantic mode
    add_compile_options(-Wall -Wextra -Werror -pedantic)
# End compiler options block
endif()
# Define static library containing the analysis pipeline
add_library(misra_core src/Lexer.cpp src/Parser.cpp src/Rules.cpp src/Reporter.cpp)
# Export include directory for consumer targets
target_include_directories(misra_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# Define CLI executable target
add_executable(misra_check src/main.cpp)
# Link core library to CLI executable
target_link_libraries(misra_check PRIVATE misra_core)
# Enable CTest integration
include(CTest)
# Enable FetchContent to download GoogleTest
include(FetchContent)
# Declare GoogleTest source archive
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
# Enforce shared runtime for GoogleTest on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Fetch and configure GoogleTest targets
FetchContent_MakeAvailable(googletest)
# Define test executable for all unit tests
add_executable(unit_tests tests/tests.cpp)
# Link unit test executable with core library and GoogleTest
target_link_libraries(unit_tests PRIVATE misra_core GTest::gtest_main)
# Register test suite runner with CTest
add_test(NAME AllTests COMMAND unit_tests)
