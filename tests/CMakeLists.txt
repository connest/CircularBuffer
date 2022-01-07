cmake_minimum_required(VERSION 3.20 FATAL_ERROR)


enable_testing()


include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/googletest
    )
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)


if(NOT googletest_POPULATED)
    FetchContent_MakeAvailable(
        googletest
    )
endif()


add_executable(${PROJECT_NAME}_test
    main.cpp
    tst_circular_buffer_blocked_mrmw.h
    tst_circular_buffer_lockfree_mrmw.h
    tst_circular_buffer_lockfree_srsw.h
    )

target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
        ${PROJECT_NAME}
        gtest
        gmock
    )

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
