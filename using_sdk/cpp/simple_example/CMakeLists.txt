cmake_minimum_required(VERSION 3.21)

include(FetchContent)

project(sonasoft_sdk_cpp_simple_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  FetchContent_Declare(
    farsounder_sdk
    URL https://github.com/farsounder/farsounder-cpp-client/releases/download/v0.1.4/farsounder-sdk-windows-release.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
else()
  FetchContent_Declare(
    farsounder_sdk
    URL https://github.com/farsounder/farsounder-cpp-client/releases/download/v0.1.4/farsounder-sdk-ubuntu-release.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
endif()

FetchContent_MakeAvailable(farsounder_sdk)

list(APPEND CMAKE_PREFIX_PATH ${farsounder_sdk_SOURCE_DIR})

find_package(farsounder REQUIRED)

set(farsounder_target "")
if(TARGET farsounder::farsounder)
  set(farsounder_target farsounder::farsounder)
elseif(TARGET farsounder)
  set(farsounder_target farsounder)
else()
  message(FATAL_ERROR "Found farsounder config package, but no usable target was exported.")
endif()

add_executable(sonasoft_sdk_cpp_simple_example main.cpp)
target_link_libraries(sonasoft_sdk_cpp_simple_example PRIVATE ${farsounder_target})

if(WIN32)
  add_custom_command(
    TARGET sonasoft_sdk_cpp_simple_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:sonasoft_sdk_cpp_simple_example>
            $<TARGET_FILE_DIR:sonasoft_sdk_cpp_simple_example>
    COMMAND_EXPAND_LISTS
  )
endif()
