add_library(devices temperature_sensor.cpp cnc_machine.cpp pressure_valve.cpp)
target_include_directories(devices PRIVATE
    ${CMAKE_SOURCE_DIR}/../external/open62541/include
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated/open62541
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated
)
target_link_libraries(devices PUBLIC mqtt_device yaml-cpp::yaml-cpp
    debug ${CMAKE_SOURCE_DIR}/../external/open62541/build/bin/Debug/open62541.lib
    optimized ${CMAKE_SOURCE_DIR}/../external/open62541/build/bin/Release/open62541.lib
)
