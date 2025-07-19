// Temperature sensor emulator

#include "../core/device_base.hpp"
#include "../protocols/mqtt/mqtt_client.hpp"
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <map>
#include <random>
#include <ctime>


class TemperatureSensor : public DeviceBase {
private:
    DeviceStatus status_;
    std::atomic<bool> running_;
    std::thread worker_;
    std::string protocol_;
    std::string mqtt_host_ = "127.0.0.1";
    int mqtt_port_ = 1883;
    std::string mqtt_topic_ = "sensors/temperature";
    struct ParamConfig {
        double min = 0;
        double max = 100;
        double step = 1;
        std::string unit;
        double value = 0;
    };
    std::map<std::string, ParamConfig> parameters_;
    std::mt19937 rng_;
public:
    TemperatureSensor(const std::string& protocol = "unknown") : status_(DeviceStatus::Inactive), running_(false), protocol_(protocol) {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
    void setMqttConfig(const std::string& host, int port, const std::string& topic) {
        mqtt_host_ = host;
        mqtt_port_ = port;
        mqtt_topic_ = topic;
    }
    void setParametersFromYaml(const YAML::Node& node) {
        if (!node["parameters"]) return;
        for (auto it = node["parameters"].begin(); it != node["parameters"].end(); ++it) {
            std::string pname = it->first.as<std::string>();
            const auto& pnode = it->second;
            ParamConfig cfg;
            cfg.min = pnode["min"] ? pnode["min"].as<double>() : 0;
            cfg.max = pnode["max"] ? pnode["max"].as<double>() : 100;
            cfg.step = pnode["step"] ? pnode["step"].as<double>() : 1;
            cfg.unit = pnode["unit"] ? pnode["unit"].as<std::string>() : "";
            cfg.value = cfg.min;
            parameters_[pname] = cfg;
        }
    }
    void randomizeParams() {
        for (auto& [name, cfg] : parameters_) {
            std::uniform_real_distribution<double> dist(cfg.min, cfg.max);
            double raw = dist(rng_);
            // Округление к ближайшему step
            double steps = std::round((raw - cfg.min) / cfg.step);
            cfg.value = cfg.min + steps * cfg.step;
        }
    }
    static std::string getCurrentTimestamp() {
        std::time_t t = std::time(nullptr);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
        return buf;
    }
    void start() override {
        if (status_ == DeviceStatus::Running) return;
        status_ = DeviceStatus::Running;
        running_ = true;
        worker_ = std::thread([this]() {
            SimpleMqttClient mqtt(mqtt_host_, mqtt_port_);
            mqtt.connect();
            while (running_) {
                randomizeParams();
                nlohmann::json j;
                j["device"] = getName();
                j["timestamp"] = getCurrentTimestamp();
                for (const auto& [name, cfg] : parameters_) {
                    j["data"][name] = {
                        {"value", cfg.value},
                        {"unit", cfg.unit}
                    };
                }
                std::cout << "[TemperatureSensor] Publish: " << j.dump() << std::endl;
                mqtt.publish(mqtt_topic_, j.dump());
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
