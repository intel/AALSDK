cmake_minimum_required(VERSION 2.8.4)
project(appbase)

# Header configuration
set (APPBASE_VERSION_MAJOR 1)
set (APPBASE_VERSION_MINOR 0)
set (APPCONSTANTS_VERSION_MAJOR 1)
set (APPCONSTANTS_VERSION_MINOR 0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/appconstants.h.in"
  "${CMAKE_BINARY_DIR}/include/appconstants.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/appbuffer.h.in"
  "${CMAKE_BINARY_DIR}/include/appbuffer.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/appbase.h.in"
  "${CMAKE_BINARY_DIR}/include/appbase.h"
  )

# Boost library
set(SOURCE_FILES appconstants.cpp appbuffer.cpp appbase.cpp)
add_library(appbase STATIC ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${CMAKE_HELPER_FILES})
target_link_libraries(appbase ${CMAKE_THREAD_LIBS_INIT})
if(Boost_FOUND)
  target_link_libraries(appbase ${Boost_LIBRARIES})
endif()

# add the install targets
install (TARGETS appbase DESTINATION lib)
install (FILES "${PROJECT_BINARY_DIR}/include/appconstants.h"
         DESTINATION include)
install (FILES "${PROJECT_BINARY_DIR}/include/appbase.h"
         DESTINATION include)
install (FILES "${PROJECT_BINARY_DIR}/include/appbuffer.h"
         DESTINATION include)

# Sample executable
set(SOURCE_FILES main.cpp)
add_executable(simple_app ${SOURCE_FILES})

# Check we compile against right library
find_library(appbase libappbase_location)
message("Generated libappbase was placed at: ${libappbase_location}")

# Link to static library
target_link_libraries(simple_app LINK_PUBLIC appbase)
