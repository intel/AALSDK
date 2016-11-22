project(wafu)

set(CMAKE_CXX_FLAGS --std=c++11)
message("${AALSDK}")
include_directories(${CMAKE_CURRENT_SRC_DIR}
                    ${CMAKE_CURRENT_SRC_DIR}/json
                    ${AALSDK}/include
                    )
link_directories(${AALSDK}/lib)

add_library(jsoncpp SHARED jsoncpp.cpp)

set(utils_cpp arguments.cpp
              utils.cpp
              process.cpp
              )

set(utils_h arguments.h
            utils.h
            process.h
            )

add_library(wutils SHARED ${utils_cpp})

set(wafu_cpp afu_client.cpp
             afu_test.cpp
             client_factory.cpp
             nlb_client.cpp
             reconfigure_client.cpp
             service_client.cpp
             service_manager.cpp
             test_context.cpp
             test_manager.cpp
             )

set(wafu_h afu_client.h
           afu_test.h
           nlb_client.h
           reconfigure_client.h
           service_client.h
           service_manager.h
           test_context.h
           test_manager.h
           )

add_library(wafu SHARED ${wafu_cpp})
target_link_libraries(wafu jsoncpp wutils OSAL AAS aalrt dl)

add_library(aalcif SHARED c_service_manager.cpp)
target_link_libraries(aalcif wafu wutils)

install(TARGETS wafu wutils jsoncpp aalcif
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES aal.py 
        DESTINATION lib
)

