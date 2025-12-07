CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
project(rvsim LANGUAGES CXX C)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O0 -fsanitize=address")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")
set (CMAKE_CXX_FLAGS "-fdiagnostics-color=always")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LLVM REQUIRED CONFIG)
find_library(GTEST_LIBRARY NAMES gtest gtest_main)

set (CMAKE_CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set (CMAKE_DECODER_DIR        ${CMAKE_BINARY_DIR}/decoder)
set (GEN_DECODER              ${CMAKE_DECODER_DIR}/decoder.inc)
set (CXXOPTS_DIR              ${CMAKE_SOURCE_DIR}/cxxopts/include)

## create generated decoder dir
#file(MAKE_DIRECTORY "${CMAKE_DECODER_DIR}")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_DECODER_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ELFIO)
include_directories(${CMAKE_SOURCE_DIR}/spdlog/include)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(gendecode ${CMAKE_CURRENT_SOURCE_DIR}/decoder_emitter.cc)
target_link_libraries(gendecode PRIVATE LLVMTableGen)

add_custom_command(
  OUTPUT ${GEN_DECODER}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_DECODER_DIR}
  COMMAND $<TARGET_FILE:gendecode>
          -I ${LLVM_INCLUDE_DIRS} -I ${CMAKE_CURRENT_SOURCE_DIR}
          -gen-decoder
          ${CMAKE_CURRENT_SOURCE_DIR}/RV.td
          -o ${GEN_DECODER}
  DEPENDS gendecode
          ${CMAKE_CURRENT_SOURCE_DIR}/decoder_emitter.cc
          ${CMAKE_CURRENT_SOURCE_DIR}/RV.td
          ${CMAKE_CURRENT_SOURCE_DIR}/RVInsn.td
          ${CMAKE_CURRENT_SOURCE_DIR}/RVInsnTypes.td
          ${CMAKE_CURRENT_SOURCE_DIR}/RVEncoding.td
          ${CMAKE_CURRENT_SOURCE_DIR}/RVOpcodes.td
  COMMENT "Generating decoder.inc from RV.td"
  VERBATIM
)

add_library(decoder_helpers STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/decoder_helpers.cc)

add_custom_target(decoder_inc DEPENDS ${GEN_DECODER})
target_link_libraries(decoder_helpers)

add_library(segment STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/segment.cc)

add_library(memory STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/memory.cc)
target_include_directories(memory PRIVATE ${CMAKE_SOURCE_DIR}/ELFIO)
target_link_libraries(memory segment)

add_library(registers STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/register_file.cc)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cc ${GEN_DECODER})
target_link_libraries(${PROJECT_NAME} segment memory registers decoder_helpers)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_DECODER_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES DEPENDS ${GEN_DECODER})
add_dependencies(${PROJECT_NAME} decoder_inc)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE ${CXXOPTS_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test.cc)
target_link_libraries(test gtest segment memory registers decoder_helpers)
target_include_directories(test PUBLIC ${CMAKE_DECODER_DIR})
set_target_properties(test PROPERTIES DEPENDS ${GEN_DECODER})
add_dependencies(test decoder_inc)
