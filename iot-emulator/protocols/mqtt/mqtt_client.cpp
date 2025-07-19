
// Real MQTT client using Paho MQTT C++
#include "mqtt_client.hpp"
#include <mqtt/async_client.h>
#include <iostream>

class SimpleMqttClientImpl {
public:
    SimpleMqttClientImpl(const std::string& host, int port)
        : server_uri_("tcp://" + host + ":" + std::to_string(port)),
          client_(server_uri_, "DeviceEmulatorClient") {}

    bool connect() {
        try {
            mqtt::connect_options connOpts;
            connOpts.set_clean_session(true);
            client_.connect(connOpts)->wait();
            return true;
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Connect failed: " << exc.what() << std::endl;
            return false;
        }
    }

    bool publish(const std::string& topic, const std::string& payload) {
        try {
            client_.publish(topic, payload.c_str(), payload.size(), 1, false)->wait();
            return true;
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Publish failed: " << exc.what() << std::endl;
            return false;
        }
    }

    void disconnect() {
        try {
            client_.disconnect()->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Disconnect failed: " << exc.what() << std::endl;
        }
    }

    ~SimpleMqttClientImpl() {
        disconnect();
    }
private:
    std::string server_uri_;
    mqtt::async_client client_;
};

// PIMPL idiom for SimpleMqttClient
SimpleMqttClient::SimpleMqttClient(const std::string& host, int port)
    : host_(host), port_(port), connected_(false), impl_(new SimpleMqttClientImpl(host, port)) {}

bool SimpleMqttClient::connect() {
    connected_ = impl_->connect();
    return connected_;
}

bool SimpleMqttClient::publish(const std::string& topic, const std::string& payload) {
    if (!connected_) connect();
    return impl_->publish(topic, payload);
}

void SimpleMqttClient::disconnect() {
    if (connected_) {
        impl_->disconnect();
        connected_ = false;
    }
}

SimpleMqttClient::~SimpleMqttClient() {
    delete impl_;
}
