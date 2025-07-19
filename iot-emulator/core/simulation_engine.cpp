// Simulation engine logic
#include "simulation_engine.hpp"
#include <iostream>

void SimulationEngine::addDevice(const std::shared_ptr<DeviceBase>& device, const std::shared_ptr<ProtocolClient>& protocol) {
    devices_.push_back({device, protocol});
}

void SimulationEngine::run() {
    running_ = true;
    std::cout << "Simulation started. Devices: " << devices_.size() << std::endl;
    for (auto& ctx : devices_) {
        if (ctx.protocol) ctx.protocol->connect();
    }
    // Здесь может быть основной цикл симуляции
}

void SimulationEngine::stop() {
    running_ = false;
    for (auto& ctx : devices_) {
        if (ctx.protocol) ctx.protocol->disconnect();
    }
    std::cout << "Simulation stopped." << std::endl;
}
