#include <gtest/gtest.h>
#include "../devices/temperature_sensor.cpp"

TEST(TemperatureSensorTest, BasicFunctionality) {
    TemperatureSensor sensor;
    EXPECT_EQ(sensor.getName(), "TemperatureSensor");
    EXPECT_EQ(sensor.getStatus(), DeviceStatus::Inactive);
    sensor.start();
    EXPECT_EQ(sensor.getStatus(), DeviceStatus::Running);
    sensor.stop();
    EXPECT_EQ(sensor.getStatus(), DeviceStatus::Stopped);
}

TEST(TemperatureSensorTest, Scenarios) {
    TemperatureSensor sensor;
    auto scenarios = sensor.getScenarios();
    ASSERT_EQ(scenarios.size(), 2);
    EXPECT_EQ(scenarios[0], "overheat");
    EXPECT_EQ(scenarios[1], "disconnect");
    EXPECT_EQ(sensor.simulate("overheat"), "TemperatureSensor: Overheat simulated!");
    EXPECT_NE(sensor.simulate("disconnect").find("Disconnected"), std::string::npos);
    EXPECT_NE(sensor.simulate("unknown").find("Unknown scenario"), std::string::npos);
}
