#pragma once
// Abstract device class
class DeviceBase {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual ~DeviceBase() {}
};
