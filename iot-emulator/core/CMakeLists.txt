
add_library(core device_base.cpp device_base.hpp device_runner.hpp simulation_engine.cpp config_loader.cpp)
target_include_directories(core PRIVATE
    ${CMAKE_SOURCE_DIR}/../external/open62541/include
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated/open62541
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# REST server for device management
add_executable(rest_server rest_server.cpp httplib.h)
target_include_directories(rest_server PRIVATE
    ${CMAKE_SOURCE_DIR}/../external/open62541/include
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated/open62541
    ${CMAKE_SOURCE_DIR}/../external/open62541/build/src_generated
)
target_link_libraries(rest_server PRIVATE core devices nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp iphlpapi)
