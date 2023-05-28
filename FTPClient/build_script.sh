#!/bin/bash

# Run CMake to generate build files
cmake -S /home/hlebk/CLionProjects/CapstoneProject/SPoVM_capstone_project/FTPClient/ -B /home/hlebk/CLionProjects/CapstoneProject/SPoVM_capstone_project/FTPClient/build/

# Set the target platform
cmake -DCMAKE_SYSTEM_NAME=Linux -P ${CMAKE_BINARY_DIR}/CMakeSystem.cmake

# Set the optimization level
cmake -DCMAKE_BUILD_TYPE=Release -P ${CMAKE_BINARY_DIR}/CMakeBuildType.cmake

# Build the project
cmake --build /home/hlebk/CLionProjects/CapstoneProject/SPoVM_capstone_project/FTPClient/build/

# Clean unnecessary files
cmake --build /home/hlebk/CLionProjects/CapstoneProject/SPoVM_capstone_project/FTPClient/build/ --target clean

# Create the tar.gz archive
cmake --build /home/hlebk/CLionProjects/CapstoneProject/SPoVM_capstone_project/FTPClient/build/ --target package

