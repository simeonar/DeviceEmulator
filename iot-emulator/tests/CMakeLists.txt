enable_testing()
# Unit test build config for IoT Device Emulator

enable_testing()

# GoogleTest setup
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
    test_temperature_sensor.cpp
)

target_link_libraries(unit_tests PRIVATE core devices gtest_main)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/iot-emulator/core
    ${CMAKE_SOURCE_DIR}/iot-emulator/devices
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
