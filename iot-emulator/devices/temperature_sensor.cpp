// Temperature sensor emulator
#include "../core/device_base.hpp"
#include <string>

class TemperatureSensor : public DeviceBase {
public:
    TemperatureSensor() : status_(DeviceStatus::Inactive) {}
    void start() override { status_ = DeviceStatus::Running; }
    void stop() override { status_ = DeviceStatus::Stopped; }
    DeviceStatus getStatus() const override { return status_; }
    std::string getName() const override { return "TemperatureSensor"; }
    std::string simulate(const std::string& scenario) {
        if (scenario == "overheat") {
            return "TemperatureSensor: Overheat simulated!";
        } else if (scenario == "disconnect") {
            status_ = DeviceStatus::Inactive;
            return "TemperatureSensor: Disconnected!";
        } else {
            return "TemperatureSensor: Unknown scenario: " + scenario;
        }
    }
private:
    DeviceStatus status_;
};
