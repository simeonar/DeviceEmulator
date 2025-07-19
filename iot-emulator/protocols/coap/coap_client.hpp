#pragma once
#include <string>

class SimpleCoapClient {
public:
    SimpleCoapClient(const std::string& host, int port);
    bool connect();
    bool sendRequest(const std::string& uri, const std::string& payload);
    void disconnect();
    ~SimpleCoapClient();
private:
    std::string host_;
    int port_;
    bool connected_;
};
