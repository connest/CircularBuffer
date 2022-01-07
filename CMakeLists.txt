cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(CircularBuffer VERSION 2.0.0 LANGUAGES CXX)



add_library(${PROJECT_NAME} INTERFACE
        include/circular_buffer/circular_buffer_blocked_mrmw.h
        include/circular_buffer/circular_buffer_lockfree_mrmw.h
        include/circular_buffer/circular_buffer_lockfree_srsw.h
    )

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)



# tests
set(${PROJECT_NAME}_BUILD_TESTS ON)
if(${PROJECT_NAME}_BUILD_TESTS)
    add_subdirectory(tests)
endif()



set(${PROJECT_NAME}_BUILD_EXAMPLE ON)
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
