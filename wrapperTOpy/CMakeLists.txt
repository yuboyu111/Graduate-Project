cmake_minimum_required(VERSION 3.10)

project(Camera LANGUAGES CXX) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 添加 pybind11 库
find_package(pybind11 REQUIRED)
find_package(OpenCV REQUIRED)

aux_source_directory(../src BASE_SOURCE)
aux_source_directory(../modules MODULE_SOURCE)

pybind11_add_module(DVS MODULE ${BASE_SOURCE} ${MODULE_SOURCE} ../wrapperTOpy/pic.cpp ../wrapperTOpy/wrapper.cpp)

# add_library(DVS ${BASE_SOURCE} ${MODULE_SOURCE} ../pic.cpp)

target_include_directories(DVS PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(DVS PUBLIC ../include)
target_include_directories(DVS PUBLIC ../wrapperTOpy)

target_link_libraries(DVS PRIVATE ${OpenCV_LIBRARIES} pybind11::module -lpthread usb-1.0)