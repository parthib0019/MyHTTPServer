cmake_minimum_required(VERSION 4.2.3)  # CMake version check
project(My_Server)               # Create project "simple_example"\

# Add main.cpp file of the project root directory as a source file
set(SOURCE_FILES main.c src/myserver.c include/myserver.h)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(My_Server ${SOURCE_FILES})

target_link_libraries(My_Server pthread)