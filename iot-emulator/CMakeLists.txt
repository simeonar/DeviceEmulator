cmake_minimum_required(VERSION 3.15)
project(iot_emulator)
set(CMAKE_CXX_STANDARD 17)


find_package(yaml-cpp CONFIG REQUIRED)
add_subdirectory(core)
add_subdirectory(devices)
add_subdirectory(protocols/mqtt)
add_subdirectory(protocols/opcua)
add_subdirectory(protocols/rest)

add_subdirectory(cli)
add_subdirectory(tests)

# Optionally add tests, etc.
