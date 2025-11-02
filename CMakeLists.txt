cmake_minimum_required(VERSION 3.10)

project(Camera LANGUAGES CXX) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(OpenCV_DIR "/home/lab/opencv-3.4.16/build")


find_package(OpenCV REQUIRED)

aux_source_directory(./src BASE_SOURCE)
aux_source_directory(./modules MODULE_SOURCE)
add_executable(camera ${BASE_SOURCE} ${MODULE_SOURCE} main.cpp)
target_include_directories(camera PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(camera PUBLIC ./include)
target_link_libraries(camera ${OpenCV_LIBRARIES} -lpthread usb-1.0)


