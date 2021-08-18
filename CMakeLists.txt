cmake_minimum_required(VERSION 2.8)
project(mjpegStreamer)

################# compiler config #################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -pedantic --coverage -fprofile-arcs -ftest-coverage")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

################# source config #################
file(GLOB SRC_LIST "*.cpp")

################# library config #################
# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

################# create executable app #################
add_executable(${PROJECT_NAME} ${SRC_LIST})

################# link connect #################
# OpenCV
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})





















