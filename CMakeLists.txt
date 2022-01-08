cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(CircularBuffer VERSION 2.0.0 LANGUAGES CXX)



add_library(${PROJECT_NAME} INTERFACE
        include/circular_buffer/circular_buffer_blocked_mrmw.h
        include/circular_buffer/circular_buffer_lockfree_mrmw.h
        include/circular_buffer/circular_buffer_lockfree_srsw.h
    )

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)



# tests
option(${PROJECT_NAME}_BUILD_TESTS OFF)
if(${PROJECT_NAME}_BUILD_TESTS)
    add_subdirectory(tests)
endif()



option(${PROJECT_NAME}_BUILD_EXAMPLE OFF)
if(${PROJECT_NAME}_BUILD_EXAMPLE)
    add_executable(${PROJECT_NAME}_example
        main.cpp
    )

    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME}_example
        PRIVATE
            ${PROJECT_NAME}
            Threads::Threads
    )

endif()
