cmake_minimum_required(VERSION 3.28)

# Try to use g++-14 for better c++23 support
find_program(GCC14 "gcc-14")
find_program(GPP14 "g++-14")

if(GCC14 AND GPP14)
    set(CMAKE_C_COMPILER "${GCC14}")
    set(CMAKE_CXX_COMPILER "${GPP14}")
endif()

project(palloc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directory
if(WIN32)
    set(PLATFORM_DIR "${CMAKE_BINARY_DIR}/palloc-win")
elseif(LINUX)
    set(PLATFORM_DIR "${CMAKE_BINARY_DIR}/palloc-linux")
elseif(APPLE)
    set(PLATFORM_DIR "${CMAKE_BINARY_DIR}/palloc-macos")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PLATFORM_DIR}/bin")

# Fix for MSVC insisting on adding more folders
if(MSVC)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${PLATFORM_DIR}/bin")
    endforeach()
endif()

# Project root macro
add_definitions(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}")

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /Od /DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE}")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE} -g -fno-omit-frame-pointer")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE_UPPER)
message(STATUS "${CMAKE_BUILD_TYPE} CXX Flags: ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER}}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Argz
set(ARGZ_HEADER_URL "https://raw.githubusercontent.com/mtygesen/argz/main/include/argz/argz.hpp")
set(ARGZ_HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/argz/argz.hpp")

if(NOT EXISTS "${ARGZ_HEADER_PATH}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/argz")
    file(DOWNLOAD "${ARGZ_HEADER_URL}" "${ARGZ_HEADER_PATH}" SHOW_PROGRESS)
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# Glaze
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG v5.0.0
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)

set(ORTOOLS_VERSION "9.12")
set(ORTOOLS_BUILD "4544")

if(WIN32)
    set(ORTOOLS_PACKAGE "or-tools_x64_VisualStudio2022_cpp_v${ORTOOLS_VERSION}.${ORTOOLS_BUILD}")
    set(ORTOOLS_ARCHIVE_EXT "zip")
elseif(APPLE)
    set(ORTOOLS_PACKAGE "or-tools_arm64_macOS-15.3.1_cpp_v${ORTOOLS_VERSION}.${ORTOOLS_BUILD}")
    set(ORTOOLS_ARCHIVE_EXT "tar.gz")
else()
    set(ORTOOLS_PACKAGE "or-tools_amd64_ubuntu-24.04_cpp_v${ORTOOLS_VERSION}.${ORTOOLS_BUILD}")
    set(ORTOOLS_ARCHIVE_EXT "tar.gz")
endif()

set(ORTOOLS_URL "https://github.com/google/or-tools/releases/download/v${ORTOOLS_VERSION}/${ORTOOLS_PACKAGE}.${ORTOOLS_ARCHIVE_EXT}")
set(ORTOOLS_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/or-tools-download")
set(ORTOOLS_ARCHIVE "${ORTOOLS_DOWNLOAD_DIR}/or-tools.${ORTOOLS_ARCHIVE_EXT}")
set(ORTOOLS_EXTRACT_DIR "${PLATFORM_DIR}/lib/or-tools")

if(NOT EXISTS "${ORTOOLS_EXTRACT_DIR}")
    file(DOWNLOAD "${ORTOOLS_URL}" "${ORTOOLS_ARCHIVE}" SHOW_PROGRESS)
    file(MAKE_DIRECTORY "${ORTOOLS_EXTRACT_DIR}")
    
    execute_process(
        COMMAND tar --strip-components=1 -xf "${ORTOOLS_ARCHIVE}" -C "${ORTOOLS_EXTRACT_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
        ERROR_VARIABLE EXTRACT_ERROR
    )
    
    if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract OR-Tools: ${EXTRACT_ERROR}")
    endif()
endif()

if(WIN32)
    file(GLOB DLLS "${ORTOOLS_EXTRACT_DIR}/bin/*.dll")
    foreach(DLL ${DLLS})
        file(COPY "${DLL}" DESTINATION "${PLATFORM_DIR}/bin")
    endforeach()
endif()

list(APPEND CMAKE_PREFIX_PATH "${ORTOOLS_EXTRACT_DIR}")

find_package(ortools REQUIRED)

add_subdirectory(src)

# Put test executable in a different directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")
enable_testing()
add_subdirectory(tests)
