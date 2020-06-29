cmake_minimum_required(VERSION 3.15)
project(smart_pointer)

set(CMAKE_CXX_STANDARD 17)

# 只有在必要的时候才构建自动测试
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(gtest)
    add_executable(unit_test test/unit_test.cpp)
    target_include_directories(unit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(unit_test gtest)
endif ()