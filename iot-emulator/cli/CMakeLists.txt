add_executable(iot_emulator_cli main.cpp)
target_link_libraries(iot_emulator_cli core devices mqtt_device opcua_device rest_device yaml-cpp::yaml-cpp)
