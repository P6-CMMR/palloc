include(AddCatch2)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(palloc_tests ${TEST_SOURCES})

target_link_libraries(palloc_tests PRIVATE dpllib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(palloc_tests)
