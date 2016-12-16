set(CMAKE_CXX_FLAGS --std=c++11)


include_directories(${CMAKE_SOURCE_DIR}/valapp
                    ${AALSDK}/include)

link_directories(${AALSDK}/lib)

add_executable(cmdq main.cpp)
target_link_libraries(cmdq wafu wutils)

install(TARGETS cmdq 
        RUNTIME DESTINATION bin)
