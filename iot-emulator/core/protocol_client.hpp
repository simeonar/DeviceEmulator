#pragma once
#include <string>

class ProtocolClient {
public:
    virtual ~ProtocolClient() = default;
    virtual bool connect() = 0;
    virtual bool send(const std::string& destination, const std::string& payload) = 0;
    virtual void disconnect() = 0;
};
