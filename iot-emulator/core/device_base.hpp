 #pragma once
#include <string>
// Abstract device class
enum class DeviceStatus {
    Inactive,
    Running,
    Stopped
};

class DeviceBase {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual DeviceStatus getStatus() const = 0;
    virtual std::string getName() const = 0;
    virtual ~DeviceBase() {}
};

// Simple device manager for demonstration
#include <vector>
#include <memory>
#include <map>
#include <string>

class DeviceManager {
public:
    void registerDevice(std::shared_ptr<DeviceBase> device) {
        devices_[device->getName()] = device;
    }
    std::map<std::string, DeviceStatus> getAllStatuses() const {
        std::map<std::string, DeviceStatus> result;
        for (const auto& [name, dev] : devices_) {
            result[name] = dev->getStatus();
        }
        return result;
    }
private:
    std::map<std::string, std::shared_ptr<DeviceBase>> devices_;
};
