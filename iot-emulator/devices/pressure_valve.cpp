// Pressure valve emulator
#include "../core/device_base.hpp"
#include <string>
#include "open62541/server.h"
#include "open62541/server_config_default.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

class PressureValve : public DeviceBase {
private:
    DeviceStatus status_;
    std::atomic<bool> running_;
    std::thread worker_;
    std::string protocol_;
public:
    PressureValve(const std::string& protocol = "unknown") : status_(DeviceStatus::Inactive), running_(false), protocol_(protocol) {}
    void start() override {
        if (status_ == DeviceStatus::Running) return;
        status_ = DeviceStatus::Running;
        running_ = true;
        worker_ = std::thread([this]() {
            // --- OPC UA server ---
            UA_Server* server = UA_Server_new();
            UA_ServerConfig* config = UA_Server_getConfig(server);
            double pressure = 100.0;
            UA_VariableAttributes attr = UA_VariableAttributes_default;
            attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Pressure");
            attr.value.type = &UA_TYPES[UA_TYPES_DOUBLE];
            attr.value.storageType = UA_VARIANT_DATA_NODELETE;
            attr.value.data = &pressure;
            UA_NodeId pressureNodeId = UA_NODEID_STRING(2, (char*)"Pressure");
            UA_QualifiedName pressureName = UA_QUALIFIEDNAME(2, (char*)"Pressure");
            UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
            UA_Server_addVariableNode(server, pressureNodeId, parentNodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), pressureName, UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
            std::cout << "[PressureValve] OPC UA server started on port 4840" << std::endl;
            while (running_) {
                pressure = 100.0 + (rand() % 20);
                UA_Server_run_iterate(server, false);
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
            UA_Server_delete(server);
        });
    }
    void stop() override {
        running_ = false;
        status_ = DeviceStatus::Stopped;
        if (worker_.joinable()) worker_.join();
    }
    DeviceStatus getStatus() const override { return status_; }
    std::string getName() const override { return "PressureValve"; }
    std::string getProtocol() const override { return protocol_; }
    std::string simulate(const std::string& scenario) override {
        if (scenario == "leak") {
            return "PressureValve: Leak simulated!";
        } else if (scenario == "jam") {
            stop();
            status_ = DeviceStatus::Inactive;
            return "PressureValve: Jam simulated!";
        } else {
            return "PressureValve: Unknown scenario: " + scenario;
        }
    }
    std::vector<std::string> getScenarios() const override {
        return {"leak", "jam"};
    }
    ~PressureValve() { stop(); }
};
