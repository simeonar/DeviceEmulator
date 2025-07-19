add_library(mqtt_device mqtt_device.cpp mqtt_client.hpp mqtt_client.cpp)


# --- Paho MQTT C/C++ integration ---
include_directories(
    ${CMAKE_SOURCE_DIR}/../external/paho/eclipse-paho-mqtt-c-win64-1.3.14/include
    ${CMAKE_SOURCE_DIR}/../external/paho/paho.mqtt.cpp-1.3.0/include
    ${CMAKE_SOURCE_DIR}/../external/paho/paho.mqtt.cpp-1.3.0/src
)

target_link_libraries(mqtt_device
    ${CMAKE_SOURCE_DIR}/../external/paho/eclipse-paho-mqtt-c-win64-1.3.14/lib/paho-mqtt3a-static.lib
    ${CMAKE_SOURCE_DIR}/../external/paho/paho.mqtt.cpp-1.3.0/build/src/Release/paho-mqttpp3-static.lib
    ws2_32.lib
)
