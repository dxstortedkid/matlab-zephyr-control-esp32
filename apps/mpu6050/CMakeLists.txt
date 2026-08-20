# Minimum CMake version required by Zephyr RTOS
cmake_minimum_required(VERSION 3.20.0)

# Locate and import the Zephyr build system
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Define project name
project(gyrocube)

# Generate compile_commands.json for IDE support and language servers (clangd, VS Code, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Private include directories for application modules
target_include_directories(app PRIVATE
    modules
)

# Application source files
target_sources(app PRIVATE
    src/main.cpp
    modules/LowPassFilter/LowPassFilter.cpp
    modules/SimplePid/SimplePid.cpp
    modules/mpu6050/mpu6050.cpp
    modules/ComplementaryFilter/ComplementaryFilter.cpp
    modules/MatlabLink/MatlabLink.cpp
)