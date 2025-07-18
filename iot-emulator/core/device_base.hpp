// Forward declaration
class DeviceRunner;

#pragma once
#include <memory>
#include <string>

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
    virtual std::string simulate(const std::string& scenario) { return "Not implemented"; }
    // Возвращает список поддерживаемых сценариев
    virtual std::vector<std::string> getScenarios() const { return {}; }
    virtual ~DeviceBase() {}
};

// Simple device manager for demonstration
#include <map>
#include <memory>
#include <string>

class DeviceManager {
public:
    void registerDevice(const std::string& name, std::shared_ptr<DeviceBase> device);
    void startDevice(const std::string& name);
    void stopDevice(const std::string& name);
    std::map<std::string, DeviceStatus> getAllStatuses() const;
    std::shared_ptr<DeviceBase> getDevice(const std::string& name);
    std::shared_ptr<DeviceRunner> getRunner(const std::string& name);
private:
    std::map<std::string, std::shared_ptr<DeviceRunner>> runners_;
};
