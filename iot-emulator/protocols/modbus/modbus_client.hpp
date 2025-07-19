#pragma once
#include <string>

class SimpleModbusClient {
public:
    SimpleModbusClient(const std::string& host, int port);
    bool connect();
    bool sendRequest(const std::string& request);
    void disconnect();
    ~SimpleModbusClient();
private:
    std::string host_;
    int port_;
    bool connected_;
};
