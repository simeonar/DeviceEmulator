# Progress tracking for DeviceEmulator MQTT integration

## 2025-07-19
- Реализована универсальная публикация параметров для TemperatureSensor:
  - Параметры (имя, min, max, step, unit) задаются в config/devices.yaml
  - Любое количество параметров
  - Публикация JSON с таймстэмпом, device, data
  - Используется nlohmann/json
- Пример секции parameters добавлен в devices.yaml
- Проект успешно собирается и работает
