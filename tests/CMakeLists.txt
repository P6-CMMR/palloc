# Catch2
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()
    
    FetchContent_Declare(
        Catch2 
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0
    )

    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    file(GLOB_RECURSE TEST_SOURCES "*.cpp")
    if (TEST_SOURCES)
        message(STATUS "Found test sources: ${TEST_SOURCES}")
        add_executable(palloc_tests ${TEST_SOURCES})
        target_link_libraries(palloc_tests PRIVATE palloclib Catch2::Catch2WithMain)

        include(CTest)
        include(Catch)
        catch_discover_tests(palloc_tests)
    endif()
endif()
