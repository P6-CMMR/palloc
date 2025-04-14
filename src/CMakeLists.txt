file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*palloc\\.cpp$")

set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/palloc.cpp")
add_library(libpalloc STATIC ${LIB_SOURCES})
target_include_directories(libpalloc PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(libpalloc PUBLIC ortools::ortools glaze::glaze)
set_target_properties(libpalloc PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} PRIVATE libpalloc)

if(NOT WIN32)
    set_source_files_properties(${PROJECT_NAME}.cpp PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers")
endif()