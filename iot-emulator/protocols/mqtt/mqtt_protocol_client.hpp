#pragma once
#include "../../core/protocol_client.hpp"
#include "mqtt_client.hpp"
#include <memory>
#include <string>

class MqttProtocolClient : public ProtocolClient {
public:
    MqttProtocolClient(const std::string& host, int port);
    bool connect() override;
    bool send(const std::string& destination, const std::string& payload) override;
    void disconnect() override;
private:
    std::unique_ptr<SimpleMqttClient> client_;
};
