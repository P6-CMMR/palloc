file(GLOB_RECURSE LIB_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*palloc\\.cpp$")

add_library(palloclib STATIC ${LIB_SOURCES})
target_include_directories(palloclib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(palloclib PUBLIC ortools::ortools glaze::glaze)

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src/palloc.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE palloclib)
set_source_files_properties(palloc.cpp PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers")
