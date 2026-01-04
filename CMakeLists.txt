cmake_minimum_required(VERSION 3.20)
project(seam CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++26 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

find_package(magic_enum CONFIG REQUIRED)

file(GLOB_RECURSE SEAM_SOURCES "src/*.cxx")
list(FILTER SEAM_SOURCES EXCLUDE REGEX "_test\\.cxx$")
file(GLOB_RECURSE SEAM_MODULES "src/*.ixx")
add_executable(seam ${SEAM_SOURCES})
target_sources(seam PRIVATE FILE_SET CXX_MODULES FILES ${SEAM_MODULES})
target_link_libraries(seam PRIVATE magic_enum::magic_enum)
target_include_directories(seam PRIVATE ${magic_enum_DIR}/include)

# Testing
enable_testing()
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES "src/*_test.cxx")
add_executable(seam_tests ${TEST_SOURCES})
target_sources(seam_tests PRIVATE FILE_SET CXX_MODULES FILES ${SEAM_MODULES})
target_link_libraries(seam_tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  magic_enum::magic_enum
)
target_include_directories(seam_tests PRIVATE ${magic_enum_DIR}/include)

gtest_discover_tests(seam_tests)
