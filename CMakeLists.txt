cmake_minimum_required(VERSION 3.28)
project(palloc)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
set(CMAKE_BUILD_TYPE Release)

include(FetchContent)

# Argz
set(ARGZ_HEADER_URL "https://raw.githubusercontent.com/stephenberry/argz/main/include/argz/argz.hpp")
set(ARGZ_HEADER_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/argz/argz.hpp")

if(NOT EXISTS "${ARGZ_HEADER_PATH}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/argz")
    file(DOWNLOAD "${ARGZ_HEADER_URL}" "${ARGZ_HEADER_PATH}" SHOW_PROGRESS)
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# Catch2
FetchContent_Declare(
    Catch2 
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

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
set(ORTOOLS_EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/or-tools")
set(ORTOOLS_ARCHIVE "${ORTOOLS_DOWNLOAD_DIR}/or-tools.${ORTOOLS_ARCHIVE_EXT}")

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

list(APPEND CMAKE_PREFIX_PATH "${ORTOOLS_EXTRACT_DIR}")
find_package(ortools REQUIRED)

add_subdirectory(src)
