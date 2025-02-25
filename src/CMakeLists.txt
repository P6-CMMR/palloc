file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(palloc palloc.cpp ${SOURCES})
target_include_directories(palloc PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(palloc PRIVATE ortools::ortools)
