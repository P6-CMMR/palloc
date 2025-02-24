add_executable(palloc palloc.cpp)
target_include_directories(palloc PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(palloc PRIVATE ortools::ortools)
