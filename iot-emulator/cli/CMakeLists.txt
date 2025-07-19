
add_executable(iot_emulator_cli main.cpp)
target_include_directories(iot_emulator_cli PRIVATE
    ${CMAKE_SOURCE_DIR}/../external/open62541/include
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated/open62541
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated
)
target_link_libraries(iot_emulator_cli core devices mqtt_device opcua_device rest_device yaml-cpp::yaml-cpp iphlpapi)
