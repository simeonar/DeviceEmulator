// MQTT client interface
#pragma once
#include <string>

class SimpleMqttClientImpl;
class SimpleMqttClient {
public:
    SimpleMqttClient(const std::string& host, int port);
    bool connect();
    bool publish(const std::string& topic, const std::string& payload);
    void disconnect();
    ~SimpleMqttClient();
private:
    std::string host_;
    int port_;
    bool connected_ = false;
    SimpleMqttClientImpl* impl_ = nullptr;
};
