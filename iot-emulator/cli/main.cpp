// Entry point for CLI application
#include <iostream>
#include <string>
#include "../core/device_base.hpp"
#include "../devices/temperature_sensor.cpp"
int main(int argc, char* argv[]) {
    std::cout << "IoT Emulator CLI started." << std::endl;
    DeviceManager manager;
    auto tempSensor = std::make_shared<TemperatureSensor>();
    manager.registerDevice(tempSensor);

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
            } else if (input.empty()) {
                continue;
            } else {
                std::cout << "Unknown command: " << input << std::endl;
            }
        }
    }
    return 0;
}
