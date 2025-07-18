#pragma once
#include "device_base.hpp"
#include <thread>
#include <atomic>
#include <memory>
#include <chrono>

// Универсальный раннер для любого устройства
class DeviceRunner {
public:
    DeviceRunner(std::shared_ptr<DeviceBase> device)
        : device_(device), running_(false) {}

    void start() {
        if (running_) return;
        running_ = true;
        device_->start();
        worker_ = std::thread([this]() {
            while (running_ && device_->getStatus() == DeviceStatus::Running) {
                // Можно расширить: публикация, REST, MQTT и т.д.
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        });
    }
    void stop() {
        running_ = false;
        device_->stop();
        if (worker_.joinable()) worker_.join();
    }
    bool isRunning() const { return running_; }
    std::shared_ptr<DeviceBase> getDevice() { return device_; }
    ~DeviceRunner() { stop(); }
private:
    std::shared_ptr<DeviceBase> device_;
    std::atomic<bool> running_;
    std::thread worker_;
};
