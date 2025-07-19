// Entry point for CLI application
#include <iostream>
#include <string>
#include "../core/device_base.hpp"
#include "../core/device_runner.hpp"
#include "../devices/temperature_sensor.cpp"
#include <yaml-cpp/yaml.h>
#include <fstream>
int main(int argc, char* argv[]) {
    std::cout << "IoT Emulator CLI started." << std::endl;

    // Load devices from YAML (relative to executable location)
    DeviceManager manager;
    try {
        std::string exe_path = argv[0];
        std::string base_dir = exe_path.substr(0, exe_path.find_last_of("/\\"));
        std::string yaml_path = base_dir + "/../../../iot-emulator/config/devices.yaml";
        std::cout << "[DEBUG] Trying to load devices.yaml from: " << yaml_path << std::endl;
        YAML::Node config = YAML::LoadFile(yaml_path);
        if (!config || config.size() == 0) {
            std::cerr << "[DEBUG] devices.yaml loaded but empty!" << std::endl;
        }
        for (auto it = config.begin(); it != config.end(); ++it) {
            std::string dev_name = it->first.as<std::string>();
            std::string dev_class = it->second["class"].as<std::string>();
            std::cout << "[DEBUG] Registering device: " << dev_name << ", class: " << dev_class << std::endl;
            if (dev_class == "TemperatureSensor") {
                auto dev = std::make_shared<TemperatureSensor>();
                std::cout << "[DEBUG] Calling setParametersFromYaml for " << dev_name << std::endl;
                dev->setParametersFromYaml(it->second);
                manager.registerDevice(dev_name, dev);
            }
            // Add other device types here
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load devices.yaml: " << e.what() << std::endl;
    }

    auto print_statuses = [&manager]() {
        auto statuses = manager.getAllStatuses();
        std::cout << "Device statuses:" << std::endl;
        for (const auto& [name, status] : statuses) {
            std::string status_str;
            switch (status) {
                case DeviceStatus::Inactive: status_str = "Inactive"; break;
                case DeviceStatus::Running: status_str = "Running"; break;
                case DeviceStatus::Stopped: status_str = "Stopped"; break;
            }
            std::cout << "  " << name << ": " << status_str << std::endl;
        }
    };

    if (argc > 1) {
        std::string cmd = argv[1];
        if (cmd == "test") {
            std::cout << "Test command executed successfully!" << std::endl;
            return 0;
        } else if (cmd == "status") {
            print_statuses();
            return 0;
        } else if (cmd == "start") {
            std::string device = "temperature_sensor";
            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--device" && i + 1 < argc) {
                    device = argv[++i];
                }
            }
            if (manager.getRunner(device)) {
                std::cout << "Starting device: " << device << std::endl;
                manager.startDevice(device);
                std::cout << "Press Ctrl+C to stop." << std::endl;
                while (manager.getDevice(device) && manager.getDevice(device)->getStatus() == DeviceStatus::Running) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            } else {
                std::cout << "Device not found: " << device << std::endl;
            }
            return 0;
        } else if (cmd == "simulate") {
            std::string device = "temperature_sensor";
            std::string scenario;
            for (int i = 2; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--device" && i + 1 < argc) {
                    device = argv[++i];
                } else if (arg == "--scenario" && i + 1 < argc) {
                    scenario = argv[++i];
                }
            }
            if (scenario.empty()) {
                std::cout << "Usage: simulate --device <name> --scenario <name>" << std::endl;
                return 1;
            }
            if (manager.getDevice(device)) {
                std::cout << manager.getDevice(device)->simulate(scenario) << std::endl;
            } else {
                std::cout << "Device not found: " << device << std::endl;
            }
            return 0;
        } else {
            std::cout << "Unknown command: " << cmd << std::endl;
            return 1;
        }
    } else {
        // Interactive REPL mode
        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);
            if (input == "exit" || input == "quit") {
                std::cout << "Exiting CLI." << std::endl;
                break;
            } else if (input == "test") {
                std::cout << "Test command executed successfully!" << std::endl;
            } else if (input == "status") {
                print_statuses();
            } else if (input.rfind("simulate", 0) == 0) {
                // parse: simulate --device <name> --scenario <name>
                std::string device = "temperature_sensor";
                std::string scenario;
                size_t dev_pos = input.find("--device ");
                size_t scen_pos = input.find("--scenario ");
                if (dev_pos != std::string::npos) {
                    size_t start = dev_pos + 9;
                    size_t end = input.find(' ', start);
                    device = input.substr(start, end - start);
                }
                if (scen_pos != std::string::npos) {
                    size_t start = scen_pos + 11;
                    scenario = input.substr(start);
                    size_t space = scenario.find(' ');
                    if (space != std::string::npos) scenario = scenario.substr(0, space);
                }
                if (scenario.empty()) {
                    std::cout << "Usage: simulate --device <name> --scenario <name>" << std::endl;
                } else if (manager.getDevice(device)) {
                    std::cout << manager.getDevice(device)->simulate(scenario) << std::endl;
                } else {
                    std::cout << "Device not found: " << device << std::endl;
                }
            } else if (input.empty()) {
                continue;
            } else {
                std::cout << "Unknown command: " << input << std::endl;
            }
        }
    }
    return 0;
}
