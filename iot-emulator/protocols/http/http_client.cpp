#include "http_client.hpp"
#include <iostream>

SimpleHttpClient::SimpleHttpClient(const std::string& host, int port)
    : host_(host), port_(port), connected_(false) {}

bool SimpleHttpClient::connect() {
    std::cout << "[HTTP] Connect to " << host_ << ":" << port_ << std::endl;
    connected_ = true;
    return connected_;
}

bool SimpleHttpClient::sendRequest(const std::string& endpoint, const std::string& payload) {
    if (!connected_) return false;
    std::cout << "[HTTP] Send request to " << endpoint << " with payload: " << payload << std::endl;
    return true;
}

void SimpleHttpClient::disconnect() {
    if (connected_) {
        std::cout << "[HTTP] Disconnect" << std::endl;
        connected_ = false;
    }
}

SimpleHttpClient::~SimpleHttpClient() {
    disconnect();
}
