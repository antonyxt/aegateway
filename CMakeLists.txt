cmake_minimum_required(VERSION 3.10)
project(PiApp CXX)

# Force the ARM cross-compiler
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Crucial Multiarch Configuration
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH /)

# Configure find paths: find libraries and headers exclusively in target system paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Help CMake look precisely in the ARM multiarch library directories
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/aarch64-linux-gnu")

set(CMAKE_CXX_STANDARD 17)

# Find the Paho C++ package
find_package(PahoMqttCpp REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(ae_bridge_app ${SOURCES})

target_include_directories(ae_bridge_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link against the official Paho C++ CMake Target
target_link_libraries(ae_bridge_app PRIVATE PahoMqttCpp::paho-mqttpp3)