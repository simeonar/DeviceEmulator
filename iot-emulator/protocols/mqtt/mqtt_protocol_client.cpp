#include "mqtt_protocol_client.hpp"

MqttProtocolClient::MqttProtocolClient(const std::string& host, int port)
    : client_(std::make_unique<SimpleMqttClient>(host, port)) {}

bool MqttProtocolClient::connect() {
    return client_->connect();
}

bool MqttProtocolClient::send(const std::string& destination, const std::string& payload) {
    return client_->publish(destination, payload);
}

void MqttProtocolClient::disconnect() {
    client_->disconnect();
}
