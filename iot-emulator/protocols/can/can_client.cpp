#include "can_client.hpp"
#include <iostream>

SimpleCanClient::SimpleCanClient(const std::string& interface)
    : interface_(interface), connected_(false) {}

bool SimpleCanClient::connect() {
    std::cout << "[CAN] Connect to interface " << interface_ << std::endl;
    connected_ = true;
    return connected_;
}

bool SimpleCanClient::sendFrame(const std::string& frame) {
    if (!connected_) return false;
    std::cout << "[CAN] Send frame: " << frame << std::endl;
    return true;
}

void SimpleCanClient::disconnect() {
    if (connected_) {
        std::cout << "[CAN] Disconnect" << std::endl;
        connected_ = false;
    }
}

SimpleCanClient::~SimpleCanClient() {
    disconnect();
}
