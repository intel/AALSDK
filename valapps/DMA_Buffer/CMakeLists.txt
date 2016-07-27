cmake_minimum_required(VERSION 2.8.4)
project(DMA_Buffer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES main.cpp)
add_executable(DMA_Buffer ${SOURCE_FILES})
target_link_libraries(DMA_Buffer appbase)

include(CTest)
enable_testing()

# Request and write test
foreach(i RANGE 1 5)
  add_test(NAME "DMA_Buffer_request_write_test_${i}MB"
    COMMAND ${CMAKE_COMMAND}
    -DTEST_PROG=$<TARGET_FILE:DMA_Buffer>
    -DSOURCEDIR=${PROJECT_SOURCE_DIR}
    -DI=${i}
    -P ${PROJECT_SOURCE_DIR}/runtest_request_write.cmake)
endforeach()

# Request and release test
# foreach(i RANGE 1 4)
#   add_test(NAME "DMA_Buffer_request_release_test_${i}MB"
#     COMMAND ${CMAKE_COMMAND}
#     -DTEST_PROG=$<TARGET_FILE:DMA_Buffer>
#     -DSOURCEDIR=${PROJECT_SOURCE_DIR}
#     -DI=${i}
#     -P ${PROJECT_SOURCE_DIR}/runtest_request_release.cmake)
# endforeach()
