cmake_minimum_required(VERSION 3.10)
project(PiApp CXX)

# Force the ARM cross-compiler
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64) # Use arm for 32-bit Pi
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_CXX_STANDARD 17)

add_executable(my_pi_app main.cpp)

