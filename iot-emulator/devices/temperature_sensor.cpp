// Temperature sensor emulator
#include "../core/device_base.hpp"
#include "../protocols/mqtt/mqtt_client.hpp"

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
    std::string protocol_;
    // MQTT config
    std::string mqtt_host_ = "127.0.0.1";
    int mqtt_port_ = 1883;
    std::string mqtt_topic_ = "sensors/temperature";
public:
    TemperatureSensor(const std::string& protocol = "unknown") : status_(DeviceStatus::Inactive), running_(false), protocol_(protocol) {}
    void setMqttConfig(const std::string& host, int port, const std::string& topic) {
        mqtt_host_ = host;
        mqtt_port_ = port;
        mqtt_topic_ = topic;
    }
    void start() override {
        if (status_ == DeviceStatus::Running) return;
        status_ = DeviceStatus::Running;
        running_ = true;
        worker_ = std::thread([this]() {
            SimpleMqttClient mqtt(mqtt_host_, mqtt_port_);
            mqtt.connect();
            while (running_) {
                int temp = 20 + rand() % 10;
                std::cout << "[TemperatureSensor] Current temperature: " << temp << " C" << std::endl;
                mqtt.publish(mqtt_topic_, std::to_string(temp));
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            mqtt.disconnect();
        });
    }
    void stop() override {
        running_ = false;
        status_ = DeviceStatus::Stopped;
        if (worker_.joinable()) worker_.join();
    }
    DeviceStatus getStatus() const override { return status_; }
    std::string getName() const override { return "TemperatureSensor"; }
    std::string getProtocol() const override { return protocol_; }
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
