cmake_minimum_required(VERSION 3.20)
project(seam CXX)

# C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++26 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Compiler settings
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Versioning
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT GIT_VERSION)
    set(GIT_VERSION "dev")
endif()

string(TIMESTAMP BUILD_TIME "%a %b %d %Y %H:%M:%S UTC" UTC)

set(GENERATED_VERSION_IXX ${CMAKE_CURRENT_BINARY_DIR}/src/version.ixx)
configure_file(
    src/version.ixx.in
    ${GENERATED_VERSION_IXX}
)

# Dependencies
find_package(magic_enum CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(replxx CONFIG REQUIRED)

# Seam executable
file(GLOB_RECURSE SEAM_SOURCES "src/*.cxx")
list(FILTER SEAM_SOURCES EXCLUDE REGEX "_test\\.cxx$")
file(GLOB_RECURSE SEAM_MODULES "src/*.ixx")
list(APPEND SEAM_MODULES ${GENERATED_VERSION_IXX})
add_executable(seam ${SEAM_SOURCES})
target_sources(seam PRIVATE FILE_SET CXX_MODULES FILES ${SEAM_MODULES})
target_link_libraries(seam PRIVATE magic_enum::magic_enum CLI11::CLI11 replxx::replxx)

# Testing
enable_testing()
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES "tests/*.cxx")
file(GLOB_RECURSE TEST_SOURCES_MODULES "tests/*.ixx")
set(TEST_MODULES ${SEAM_MODULES})
add_executable(seam_tests ${TEST_SOURCES})
target_sources(seam_tests PRIVATE FILE_SET CXX_MODULES FILES ${TEST_MODULES} ${TEST_SOURCES_MODULES})
target_link_libraries(seam_tests PRIVATE GTest::gtest GTest::gtest_main replxx::replxx)

gtest_discover_tests(seam_tests)
