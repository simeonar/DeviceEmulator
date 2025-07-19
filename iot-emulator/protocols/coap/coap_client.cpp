#include "coap_client.hpp"
#include <iostream>

SimpleCoapClient::SimpleCoapClient(const std::string& host, int port)
    : host_(host), port_(port), connected_(false) {}

bool SimpleCoapClient::connect() {
    std::cout << "[CoAP] Connect to " << host_ << ":" << port_ << std::endl;
    connected_ = true;
    return connected_;
}

bool SimpleCoapClient::sendRequest(const std::string& uri, const std::string& payload) {
    if (!connected_) return false;
    std::cout << "[CoAP] Send request to " << uri << " with payload: " << payload << std::endl;
    return true;
}

void SimpleCoapClient::disconnect() {
    if (connected_) {
        std::cout << "[CoAP] Disconnect" << std::endl;
        connected_ = false;
    }
}

SimpleCoapClient::~SimpleCoapClient() {
    disconnect();
}
