
# Progress tracking for DeviceEmulator MQTT integration

## 2025-07-19
- Implemented universal parameter publishing for TemperatureSensor:
  - Parameters (name, min, max, step, unit) are set in config/devices.yaml
  - Any number of parameters
  - Publishes JSON with timestamp, device, data
  - Uses nlohmann/json
- Example of parameters section added to devices.yaml
- Project builds and works successfully

## 2025-07-19
- Added MqttProtocolClient adapter in protocols/mqtt for universal protocol client architecture.

## 2025-07-19
- Refactored simulation_engine for universal protocol client support (ProtocolClient interface, device-protocol binding). Ready for OPC UA and other protocols.

## 2025-07-19
- Added protocol stubs for Modbus RTU/TCP, CAN/CANopen, CoAP, HTTP/REST in protocols folder (headers and cpp files, by analogy with mqtt).
