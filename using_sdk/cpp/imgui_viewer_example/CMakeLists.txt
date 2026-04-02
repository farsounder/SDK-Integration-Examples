cmake_minimum_required(VERSION 3.21)

include(FetchContent)

project(sonasoft_sdk_cpp_imgui_viewer_example LANGUAGES C CXX)

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

FetchContent_Declare(
  glfw
  URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.6.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
  glad
  URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR cmake
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(farsounder_sdk glfw glad)

if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
endif()

list(APPEND CMAKE_PREFIX_PATH ${farsounder_sdk_SOURCE_DIR})

find_package(OpenGL REQUIRED)
find_package(farsounder REQUIRED)

glad_add_library(glad_gl_core_45 STATIC REPRODUCIBLE LOADER API gl:core=4.5)

set(farsounder_target "")
if(TARGET farsounder::farsounder)
  set(farsounder_target farsounder::farsounder)
elseif(TARGET farsounder)
  set(farsounder_target farsounder)
else()
  message(FATAL_ERROR "Found farsounder config package, but no usable target was exported.")
endif()

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(
  imgui
  PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

target_link_libraries(imgui PUBLIC glfw glad_gl_core_45 OpenGL::GL)

add_executable(
  sonasoft_sdk_cpp_imgui_viewer_example
  main.cpp
)

target_include_directories(
  sonasoft_sdk_cpp_imgui_viewer_example
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  sonasoft_sdk_cpp_imgui_viewer_example
  PRIVATE
    ${farsounder_target}
    glad_gl_core_45
    imgui
    glfw
    OpenGL::GL
)

if(WIN32)
  add_custom_command(
    TARGET sonasoft_sdk_cpp_imgui_viewer_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:sonasoft_sdk_cpp_imgui_viewer_example>
            $<TARGET_FILE_DIR:sonasoft_sdk_cpp_imgui_viewer_example>
    COMMAND_EXPAND_LISTS
  )
endif()
