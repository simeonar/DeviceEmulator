cmake_minimum_required(VERSION 3.15)
project(iot_emulator)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(core)
add_subdirectory(devices)
add_subdirectory(protocols/mqtt)
add_subdirectory(protocols/opcua)
add_subdirectory(protocols/rest)
add_subdirectory(cli)

# Optionally add tests, etc.
