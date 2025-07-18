add_library(core device_base.cpp device_base.hpp device_runner.hpp simulation_engine.cpp config_loader.cpp)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# REST server for device management
add_executable(rest_server rest_server.cpp httplib.h)
target_link_libraries(rest_server PRIVATE core nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp)
