cmake_minimum_required (VERSION 3.0.0)
project (twimagewall)
add_definitions(-std=c++1y)

# Enable debug symbols by default
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

set(EXECUTABLE_NAME "twimagewall")
add_executable (${EXECUTABLE_NAME} src/twimagewall.cpp)

# SFML
set(SFML_ROOT "SFML")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/SFML/cmake/Modules/" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})

  # and sfml included freetype
  target_link_libraries(${EXECUTABLE_NAME} "${CMAKE_SOURCE_DIR}/SFML/extlibs/libs-osx/Frameworks/freetype.framework/Versions/A/freetype")
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
if (OPENGL_FOUND)
  target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
  # or: target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_gl_LIBRARY})
  target_link_libraries(${EXECUTABLE_NAME} m)  # if you use maths.h
endif()

# Boost
find_package(Boost COMPONENTS system iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})

# OpenSSL
set(OpenSSL_ROOT_DIR /usr/local/Cellar/openssl/1.0.2a-1)
# find_package(OpenSSL)
# if (OPENSSL_FOUND)
#   include_directories(${OpenSSL_INCLUDE_DIR})
#   target_link_libraries(${EXECUTABLE_NAME} ${OpenSSL_LIBRARIES})
# endif()

include_directories("${OpenSSL_ROOT_DIR}/include")
target_link_libraries(${EXECUTABLE_NAME} "${OpenSSL_ROOT_DIR}/lib/libcrypto.dylib")
target_link_libraries(${EXECUTABLE_NAME} "${OpenSSL_ROOT_DIR}/lib/libssl.dylib")

# PicoJson
include_directories("${CMAKE_SOURCE_DIR}/picojson")

# OAuth
include_directories("${CMAKE_SOURCE_DIR}/liboauthcpp/include")
target_link_libraries(${EXECUTABLE_NAME} "${CMAKE_SOURCE_DIR}/liboauthcpp/build/liboauthcpp.a")

# AHJS
include_directories("${CMAKE_SOURCE_DIR}/async-https-json-stream")
