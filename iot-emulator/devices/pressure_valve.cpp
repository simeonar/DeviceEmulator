// Pressure valve emulator
#include "../core/device_base.hpp"
#include <string>
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
            while (running_) {
                int pressure = 100 + rand() % 20;
                std::cout << "[PressureValve] Current pressure: " << pressure << " kPa" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
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
