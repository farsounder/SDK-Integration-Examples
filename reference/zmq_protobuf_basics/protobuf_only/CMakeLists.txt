cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0091 NEW)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

project(protobuf_only LANGUAGES CXX)

include(FetchContent)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOBUF_BINARIES ON CACHE BOOL "" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(protobuf_ABSL_PROVIDER "module" CACHE STRING "" FORCE)

set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
set(ABSL_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  protobuf
  URL https://github.com/protocolbuffers/protobuf/archive/refs/tags/v33.5.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(protobuf)

include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")

set(proto_file "${CMAKE_CURRENT_LIST_DIR}/tutorial.proto")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${generated_dir}")

add_library(tutorial_proto STATIC)
target_sources(tutorial_proto PRIVATE "${proto_file}")

protobuf_generate(
  TARGET tutorial_proto
  LANGUAGE cpp
  PROTOC_OUT_DIR "${generated_dir}"
  IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}"
)

target_include_directories(
  tutorial_proto
  PUBLIC
    "${generated_dir}"
)

target_link_libraries(
  tutorial_proto
  PUBLIC
    protobuf::libprotobuf
)

add_executable(write_addresses write_addresses.cpp)
target_link_libraries(write_addresses PRIVATE tutorial_proto)

add_executable(read_addresses read_addresses.cpp)
target_link_libraries(read_addresses PRIVATE tutorial_proto)
