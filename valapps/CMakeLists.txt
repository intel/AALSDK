project(valapps)
cmake_minimum_required(VERSION 3.7)

include(CPack)
set(AALSDK $ENV{AALSDK} CACHE PATH "Path to aalsdk installation")

set(VALAPPS_VERSION_MAJOR 0)
set(VALAPPS_VERSION_MINOR 0)
set(VALAPPS_VERSION_PATCH 1)

cmake_host_system_information(RESULT HOSTNAME QUERY HOSTNAME)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_subdirectory(valapp)
add_subdirectory(gtest)
add_subdirectory(cmdq)

configure_file(CPackConfig.cmake.in CPackConfig.cmake @ONLY)
configure_file(build.json.in build.json @ONLY)
configure_file(env.sh.in env.sh @ONLY)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/env.sh DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/buildenv.py DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/runit.py DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/fetchers.py DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/installers.py DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/unpackers.py DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/build.json DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/services.json DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/tests.json DESTINATION .)



find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API docs" VERBATIM
        )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DESTINATION .)
endif(DOXYGEN_FOUND)
