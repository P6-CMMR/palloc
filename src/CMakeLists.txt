file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*palloc\\.cpp$")

add_library(libpalloc STATIC ${LIB_SOURCES})
target_include_directories(libpalloc PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(libpalloc PUBLIC ortools::ortools glaze::glaze)
set_target_properties(libpalloc PROPERTIES PREFIX "")

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/palloc.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE libpalloc)
set_source_files_properties(palloc.cpp PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers")
