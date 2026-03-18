cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0091 NEW)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

project(hello_world_zmq LANGUAGES CXX)

include(FetchContent)

set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(cppzmq CONFIG QUIET)
find_package(libzmq CONFIG QUIET)

if(NOT cppzmq_FOUND OR NOT libzmq_FOUND)
  set(CPPZMQ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(WITH_DOCS OFF CACHE BOOL "" FORCE)
  set(ENABLE_CURVE OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    libzmq
    GIT_REPOSITORY https://github.com/zeromq/libzmq.git
    GIT_TAG v4.3.5
  )
  FetchContent_Declare(
    cppzmq
    GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
    GIT_TAG v4.11.0
  )
  FetchContent_MakeAvailable(libzmq cppzmq)
endif()

set(cppzmq_include_dirs "")
if(TARGET cppzmq)
  get_target_property(cppzmq_include_dirs cppzmq INTERFACE_INCLUDE_DIRECTORIES)
elseif(DEFINED cppzmq_SOURCE_DIR)
  set(cppzmq_include_dirs "${cppzmq_SOURCE_DIR}")
else()
  message(FATAL_ERROR "Could not determine where cppzmq headers are located.")
endif()

set(zmq_target "")
if(TARGET libzmq-static)
  set(zmq_target libzmq-static)
elseif(TARGET libzmq)
  set(zmq_target libzmq)
else()
  message(FATAL_ERROR "Could not determine which libzmq target to use.")
endif()

add_executable(zmq_sender sender.cpp)
target_include_directories(zmq_sender PRIVATE ${cppzmq_include_dirs})
target_link_libraries(zmq_sender PRIVATE ${zmq_target})

add_executable(zmq_receiver receiver.cpp)
target_include_directories(zmq_receiver PRIVATE ${cppzmq_include_dirs})
target_link_libraries(zmq_receiver PRIVATE ${zmq_target})
