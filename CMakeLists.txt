CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
project(rvsim)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -fsanitize=address")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")
set (CMAKE_CXX_FLAGS "-fdiagnostics-color=always")

find_package(Boost COMPONENTS program_options REQUIRED)
find_library(GTEST_LIBRARY NAMES gtest gtest_main)
include_directories( ${Boost_INCLUDE_DIR} )

set (CMAKE_CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/ELFIO)

add_library(segment STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/segment.cc)

add_library(memory STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/memory.cc)
target_link_libraries(memory segment)

add_library(registers STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/register_file.cc)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
target_link_libraries(${PROJECT_NAME} segment memory registers)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} Boost::program_options)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test.cc)
target_link_libraries(test gtest segment memory registers)
