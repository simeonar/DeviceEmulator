#pragma once
#include <vector>
#include <memory>
#include "device_base.hpp"
#include "protocol_client.hpp"

class SimulationEngine {
public:
    struct DeviceContext {
        std::shared_ptr<DeviceBase> device;
        std::shared_ptr<ProtocolClient> protocol;
    };

    void addDevice(const std::shared_ptr<DeviceBase>& device, const std::shared_ptr<ProtocolClient>& protocol);
    void run();
    void stop();
private:
    std::vector<DeviceContext> devices_;
    bool running_ = false;
};
