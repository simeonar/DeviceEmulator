#pragma once
#include <string>

class SimpleCanClient {
public:
    SimpleCanClient(const std::string& interface);
    bool connect();
    bool sendFrame(const std::string& frame);
    void disconnect();
    ~SimpleCanClient();
private:
    std::string interface_;
    bool connected_;
};
