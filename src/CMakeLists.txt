file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*palloc\\.cpp$")

set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/palloc.cpp")
add_library(libpalloc STATIC ${LIB_SOURCES})
target_include_directories(libpalloc PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(libpalloc PUBLIC ortools::ortools glaze::glaze)
set_target_properties(libpalloc PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} PRIVATE libpalloc)

# Stop MSVC from making subdirectories
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PLATFORM_DIR}/bin")
endif()

if(NOT MSVC)
    set_source_files_properties(${PROJECT_NAME}.cpp PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers")
endif()

# Move or-tools license file to lib/or-tools
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PLATFORM_DIR}/licenses/or-tools"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PLATFORM_DIR}/lib/or-tools/share/doc/ortools/LICENSE"
        "${PLATFORM_DIR}/licenses/or-tools/LICENSE"
    COMMENT "Copying OR-Tools license to output directory"
)
