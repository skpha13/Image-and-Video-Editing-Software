cmake_minimum_required(VERSION 3.24)
project(Image_and_Video_Editing_Software)

set(CMAKE_CXX_STANDARD 17)

set(ENV{OPENCV_DIR} "D:\\opencv\\build")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIR})

add_executable(Image_and_Video_Editing_Software main.cpp)

target_link_libraries(Image_and_Video_Editing_Software ${OpenCV_LIBS})
