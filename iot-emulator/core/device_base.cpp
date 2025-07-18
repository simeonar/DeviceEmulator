#include "device_base.hpp"
#include "device_runner.hpp"

void DeviceManager::registerDevice(const std::string& name, std::shared_ptr<DeviceBase> device) {
    runners_[name] = std::make_shared<DeviceRunner>(device);
}
void DeviceManager::startDevice(const std::string& name) {
    if (runners_.count(name)) runners_[name]->start();
}
void DeviceManager::stopDevice(const std::string& name) {
    if (runners_.count(name)) runners_[name]->stop();
}
std::map<std::string, DeviceStatus> DeviceManager::getAllStatuses() const {
    std::map<std::string, DeviceStatus> result;
    for (const auto& [name, runner] : runners_) {
        result[name] = runner->getDevice()->getStatus();
    }
    return result;
}
std::shared_ptr<DeviceBase> DeviceManager::getDevice(const std::string& name) {
    if (runners_.count(name)) return runners_[name]->getDevice();
    return nullptr;
}
std::shared_ptr<DeviceRunner> DeviceManager::getRunner(const std::string& name) {
    if (runners_.count(name)) return runners_[name];
    return nullptr;
}
