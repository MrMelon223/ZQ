cmake_minimum_required(VERSION 3.10)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross-compiler
set(CMAKE_C_COMPILER "C:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10/bin/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10/bin/arm-none-eabi-g++.exe")

project(ZQ LANGUAGES CXX)

# Adjust the search behavior to find libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/permissive- /volatile:iso)
endif()

# Add your source files
add_executable(ZQ source/main.cpp source/ZQapp.cpp source/ZQcamera.cpp source/ZQcompute.cpp source/ZQhelper.cpp source/ZQlevel.cpp source/ZQmodel.cpp source/ZQshader.cpp source/ZQtexture.cpp)