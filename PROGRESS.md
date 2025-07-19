# Progress tracking for DeviceEmulator MQTT integration

## 2025-07-19
- Реализована универсальная публикация параметров для TemperatureSensor:
  - Параметры (имя, min, max, step, unit) задаются в config/devices.yaml
  - Любое количество параметров
  - Публикация JSON с таймстэмпом, device, data
  - Используется nlohmann/json
- Пример секции parameters добавлен в devices.yaml
- Проект успешно собирается и работает

## 2025-07-19
- Added MqttProtocolClient adapter in protocols/mqtt for universal protocol client architecture.

## 2025-07-19
- Refactored simulation_engine for universal protocol client support (ProtocolClient interface, device-protocol binding). Ready for OPC UA and other protocols.

## 2025-07-19
- Added protocol stubs for Modbus RTU/TCP, CAN/CANopen, CoAP, HTTP/REST in protocols folder (headers and cpp files, by analogy with mqtt).
