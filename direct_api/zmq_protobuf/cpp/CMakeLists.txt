cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0091 NEW)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

project(sonasoft_direct_zmq_cpp LANGUAGES CXX)

include(FetchContent)

# Older third-party dependencies may still declare very old policy ranges.
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

set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZMQ_BUILD_PERFORMANCE_TESTS OFF CACHE BOOL "" FORCE)
set(ZMQ_BUILD_FRAMEWORK OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(WITH_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_CURVE OFF CACHE BOOL "" FORCE)

set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CPPZMQ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  protobuf
  URL https://github.com/protocolbuffers/protobuf/archive/refs/tags/v33.5.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(
  libzmq
  URL https://github.com/zeromq/libzmq/archive/refs/tags/v4.3.5.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(
  cppzmq
  URL https://github.com/zeromq/cppzmq/archive/refs/tags/v4.11.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(protobuf libzmq cppzmq)

include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")

get_filename_component(proto_root_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
file(GLOB proto_files "${proto_root_dir}/proto/*.proto")
list(SORT proto_files)

if(proto_files STREQUAL "")
  message(FATAL_ERROR "Could not find any .proto files under ${proto_root_dir}/proto")
endif()

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${generated_dir}/proto")

add_executable(sonasoft_direct_cpp src/main.cpp)
target_sources(sonasoft_direct_cpp PRIVATE ${proto_files})

protobuf_generate(
  TARGET sonasoft_direct_cpp
  LANGUAGE cpp
  PROTOC_OUT_DIR "${generated_dir}"
  IMPORT_DIRS "${proto_root_dir}"
)

target_include_directories(
  sonasoft_direct_cpp
  PRIVATE
    "${generated_dir}"
    "${cppzmq_SOURCE_DIR}"
)

target_link_libraries(
  sonasoft_direct_cpp
  PRIVATE
    protobuf::libprotobuf
    libzmq-static
)

set_target_properties(
  sonasoft_direct_cpp
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)
