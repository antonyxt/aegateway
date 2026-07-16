cmake_minimum_required(VERSION 3.10)
project(PiApp CXX)

# Removed cross-compilation overrides (SYSTEM_NAME, PROCESSOR, COMPILERS)
# to let CMake automatically find the native system compilers.

set(CMAKE_CXX_STANDARD 17)

# Find the Paho C++ package natively installed on Debian
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
