cmake_minimum_required(VERSION 3.28)
project(VGU_TEST)

find_package(OpenCV REQUIRED)
add_executable(cam_app main.cpp)

target_include_directories(cam_app PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(cam_app ${OpenCV_LIBS})