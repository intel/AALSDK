
set(CMAKE_CXX_FLAGS --std=c++11)
#set(GTEST_ROOT /usr/src/gtest)
find_package(Threads)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}
                    ${AALSDK}/include
                    ${CMAKE_SOURCE_DIR}/valapp
                    )

link_directories(${AALSDK}/lib)

add_library(gtreconfigure SHARED gtPartialReconfig.cpp)
target_link_libraries(gtreconfigure ${GTEST_LIBRARIES} wafu)

add_library(gtreset SHARED gtReset.cpp)
target_link_libraries(gtreset ${GTEST_LIBRARIES} wafu)

add_library(gtmmio SHARED gtMmioMapping.cpp)
target_link_libraries(gtmmio ${GTEST_LIBRARIES} wafu)

add_library(gtdma_buffer SHARED gtDmaBuffer.cpp)
target_link_libraries(gtdma_buffer ${GTEST_LIBRARIES} wafu)


add_library(gtperfc SHARED gtPerfCounters.cpp)
target_link_libraries(gtperfc ${GTEST_LIBRARIES} wafu)

add_library(gtRAS SHARED gtRAS.cpp)
target_link_libraries(gtRAS ${GTEST_LIBRARIES} wafu)

add_library(gtsocketid SHARED gtResourcebySocketID.cpp)
target_link_libraries(gtsocketid ${GTEST_LIBRARIES} wafu)

add_executable(gtapp main.cpp)
target_link_libraries(gtapp 
    ${GTEST_BOTH_LIBRARIES} 
    wafu 
    wutils 
    ${CMAKE_THREAD_LIBS_INIT}
    )


install(TARGETS gtapp gtreconfigure gtreset gtdma_buffer gtperfc gtmmio gtsocketid gtRAS
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION tests
        ARCHIVE DESTINATION lib)
