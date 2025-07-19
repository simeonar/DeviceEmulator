#include "modbus_client.hpp"
#include <iostream>

SimpleModbusClient::SimpleModbusClient(const std::string& host, int port)
    : host_(host), port_(port), connected_(false) {}

bool SimpleModbusClient::connect() {
    std::cout << "[Modbus] Connect to " << host_ << ":" << port_ << std::endl;
    connected_ = true;
    return connected_;
}

bool SimpleModbusClient::sendRequest(const std::string& request) {
    if (!connected_) return false;
    std::cout << "[Modbus] Send request: " << request << std::endl;
    return true;
}

void SimpleModbusClient::disconnect() {
    if (connected_) {
        std::cout << "[Modbus] Disconnect" << std::endl;
        connected_ = false;
    }
}

SimpleModbusClient::~SimpleModbusClient() {
    disconnect();
}
