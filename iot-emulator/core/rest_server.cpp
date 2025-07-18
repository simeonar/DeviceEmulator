// Simple REST server for device management (using cpp-httplib)
#include "device_base.hpp"
#include "device_runner.hpp"
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <map>
#include "../devices/temperature_sensor.cpp"

using json = nlohmann::json;
using namespace httplib;

DeviceManager manager;

void setup_example_devices() {
    // Example: register a temperature sensor
    auto temp = std::make_shared<TemperatureSensor>();
    manager.registerDevice("temperature_sensor", temp);
}

int main() {
    setup_example_devices();
    Server svr;

    svr.Get("/devices", [](const Request&, Response& res) {
        json arr = json::array();
        for (const auto& [name, runner] : manager.getAllStatuses()) {
            auto dev = manager.getDevice(name);
            arr.push_back({
                {"id", name},
                {"name", dev ? dev->getName() : name},
                {"status", (int)runner},
                {"protocol", "unknown"}, // TODO: set real protocol if available
                {"scenarios", dev ? dev->getScenarios() : std::vector<std::string>{}}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    svr.Post(R"(/devices/(.+)/scenarios/(.+))", [](const Request& req, Response& res) {
        std::string device_id = req.matches[1];
        std::string scenario = req.matches[2];
        auto dev = manager.getDevice(device_id);
        if (!dev) {
            res.status = 404;
            res.set_content("{\"error\":\"Device not found\"}", "application/json");
            return;
        }
        std::string result = dev->simulate(scenario);
        res.set_content(json({{"result", result}}).dump(), "application/json");
    });

    svr.Post(R"(/devices/(.+)/start)", [](const Request& req, Response& res) {
        std::string device_id = req.matches[1];
        auto dev = manager.getDevice(device_id);
        if (!dev) {
            res.status = 404;
            res.set_content("{\"error\":\"Device not found\"}", "application/json");
            return;
        }
        manager.startDevice(device_id);
        res.set_content("{\"result\":\"Device started\"}", "application/json");
    });

    svr.Post(R"(/devices/(.+)/stop)", [](const Request& req, Response& res) {
        std::string device_id = req.matches[1];
        auto dev = manager.getDevice(device_id);
        if (!dev) {
            res.status = 404;
            res.set_content("{\"error\":\"Device not found\"}", "application/json");
            return;
        }
        manager.stopDevice(device_id);
        res.set_content("{\"result\":\"Device stopped\"}", "application/json");
    });

    svr.Get(R"(/devices/(.+)/status)", [](const Request& req, Response& res) {
        std::string device_id = req.matches[1];
        auto dev = manager.getDevice(device_id);
        if (!dev) {
            res.status = 404;
            res.set_content("{\"error\":\"Device not found\"}", "application/json");
            return;
        }
        json j = {
            {"id", device_id},
            {"status", (int)dev->getStatus()}
        };
        res.set_content(j.dump(), "application/json");
    });

    svr.Get(R"(/devices/(.+)/history)", [](const Request& req, Response& res) {
        // TODO: implement real history, now returns stub
        std::string device_id = req.matches[1];
        json arr = json::array();
        arr.push_back({{"timestamp", "2025-07-18T12:00:00"}, {"value", 23}});
        arr.push_back({{"timestamp", "2025-07-18T12:01:00"}, {"value", 24}});
        res.set_content(arr.dump(), "application/json");
    });

    svr.listen("0.0.0.0", 8081);
    return 0;
}
