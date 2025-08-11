cmake_minimum_required(VERSION 3.31)
project(EasyXStudy_Survival)

set(CMAKE_CXX_STANDARD 11)

add_executable(EasyXStudy_Survival WIN32 main.cpp)

target_link_libraries(EasyXStudy_Survival libeasyx.a)
target_link_libraries(EasyXStudy_Survival libmsimg32.a)
target_link_libraries(EasyXStudy_Survival libwinmm.a)