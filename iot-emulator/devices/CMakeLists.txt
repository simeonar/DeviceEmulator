add_library(devices temperature_sensor.cpp cnc_machine.cpp pressure_valve.cpp)
target_link_libraries(devices PUBLIC mqtt_device)
