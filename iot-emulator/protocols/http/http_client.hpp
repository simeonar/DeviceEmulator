#pragma once
#include <string>

class SimpleHttpClient {
public:
    SimpleHttpClient(const std::string& host, int port);
    bool connect();
    bool sendRequest(const std::string& endpoint, const std::string& payload);
    void disconnect();
    ~SimpleHttpClient();
private:
    std::string host_;
    int port_;
    bool connected_;
};
