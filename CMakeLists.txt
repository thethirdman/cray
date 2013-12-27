# BASIC CMAKE HEADER
cmake_minimum_required(VERSION 2.6)
project (cray)

find_package( OpenCV REQUIRED )

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wextra")

file (
  GLOB_RECURSE
  SOURCE_FILES
  src/*.cc
)

file (
  GLOB_RECURSE
  HEADER_FILES
  src/*.hh
  src/*.hhx
)

add_definitions(-g)

add_subdirectory(tinyxml)
include_directories(tinyxml)

add_executable (cray ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(cray ${OpenCV_LIBS} tinyxml2)

install (TARGETS cray DESTINATION bin)
