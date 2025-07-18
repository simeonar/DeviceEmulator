// Temperature sensor emulator
#include "../core/device_base.hpp"

#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

class TemperatureSensor : public DeviceBase {
private:
    DeviceStatus status_;
    std::atomic<bool> running_;
    std::thread worker_;
public:
    TemperatureSensor() : status_(DeviceStatus::Inactive), running_(false) {}
    void start() override {
        if (status_ == DeviceStatus::Running) return;
        status_ = DeviceStatus::Running;
        running_ = true;
        worker_ = std::thread([this]() {
            while (running_) {
                // Эмуляция публикации данных
                int temp = 20 + rand() % 10;
                std::cout << "[TemperatureSensor] Current temperature: " << temp << " C" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        });
    }
    void stop() override {
        running_ = false;
        status_ = DeviceStatus::Stopped;
        if (worker_.joinable()) worker_.join();
    }
    DeviceStatus getStatus() const override { return status_; }
    std::string getName() const override { return "TemperatureSensor"; }
    std::string simulate(const std::string& scenario) override {
        if (scenario == "overheat") {
            return "TemperatureSensor: Overheat simulated!";
        } else if (scenario == "disconnect") {
            stop();
            status_ = DeviceStatus::Inactive;
            return "TemperatureSensor: Disconnected!";
        } else {
            return "TemperatureSensor: Unknown scenario: " + scenario;
        }
    }
    std::vector<std::string> getScenarios() const override {
        return {"overheat", "disconnect"};
    }
    ~TemperatureSensor() { stop(); }
};
