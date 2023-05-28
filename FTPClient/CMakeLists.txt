cmake_minimum_required(VERSION 3.22)
project(FTPClient C)

set(CMAKE_C_STANDARD 11)

# Disable all warnings
add_compile_options(-w)

# Add the "package" target
add_custom_target(package
        COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "${CMAKE_BINARY_DIR}/ftpclient.tar.gz"
        "${CMAKE_BINARY_DIR}/FTPClient"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating tar.gz archive"
        )

# Set the optimization level
set(CMAKE_BUILD_TYPE Release)

# Add the ftpclient executable target
add_executable(FTPClient ftp-client.c)

# Additional build options
target_compile_options(FTPClient PRIVATE -O2)

