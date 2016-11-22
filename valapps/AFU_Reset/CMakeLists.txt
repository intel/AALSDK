cmake_minimum_required(VERSION 2.8.4)
project(AFU_Reset)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES main.cpp AFU_Reset.h)
add_executable(AFU_Reset ${SOURCE_FILES})
target_link_libraries(AFU_Reset)

include(CTest)
enable_testing()

add_test(NAME AFU_Reset_test
  COMMAND ${CMAKE_COMMAND}
  -DTEST_PROG=$<TARGET_FILE:AFU_Reset>
  -DSOURCEDIR=${PROJECT_SOURCE_DIR}
  -P ${PROJECT_SOURCE_DIR}/runtest.cmake)
