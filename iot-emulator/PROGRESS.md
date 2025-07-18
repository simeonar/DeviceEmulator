# Project Progress

- [x] Project structure created according to plan
- [x] C++ core folders and stubs added
- [x] Python monitor (Flask) stub added
- [x] Device and protocol stubs added
- [x] CMake and Python environment configured
- [x] Build/test run and working
- [x] Device status tracking and DeviceManager implemented
- [x] Universal DeviceRunner implemented for modular device execution (background thread)
- [x] DeviceManager refactored to use DeviceRunner for each device
- [x] Build system and includes refactored for modularity (DeviceRunner in separate file)
- [x] DeviceBase/TemperatureSensor: поддержка сценариев (getScenarios)
- [x] Проектирование REST API для управления устройствами и сценариями (Flask)
- [x] Реализован Flask API (заглушки): список устройств, сценарии, запуск сценария, CRUD сценариев
- [x] C++ REST API (rest_server) implemented and runs on http://localhost:8081
- [x] Python backend_bridge integrated with C++ REST API via requests
- [x] Web and CLI now work with real device backend through REST
- [x] Added REST endpoints to Flask API for device start/stop/status/history (monitor/api.py)
- [x] Changes committed to git

---

## Next steps & best practices for device emulator

1. **Device implementation**
   - Each device (e.g. `TemperatureSensor`) inherits from `DeviceBase` and implements all required methods.
   - Add a `simulate()` method for scenario emulation (e.g. overheat, disconnect).
   - Store device configuration in `config/devices.yaml` (class, protocol, port, initial state, scenarios).

2. **Protocol abstraction**
   - Implement protocol adapters (MQTT, REST, OPC UA) as separate classes.
   - Device should not depend on protocol details; use dependency injection or factory pattern.
   - Allow dynamic protocol selection via config.

3. **CLI improvements**
   - Add commands: `start`, `stop`, `simulate`, `status` for device management.
   - Support arguments: `--device`, `--scenario`, etc.
   - Example: `./emulator_cli start --device temperature_sensor`

4. **Scenario simulation**
   - Devices should support scenario execution (call `simulate()` with scenario name).
   - Example: `./emulator_cli simulate --device temperature_sensor --scenario overheat`

5. **Monitoring & integration**
   - Devices publish data via MQTT/REST to IoT Gateway or monitoring server.
   - Python monitor subscribes to device data and displays status in web UI.
   - Use IPC, REST, or MQTT for communication between C++ and Python parts.
   - [x] Веб-интерфейс и CLI работают через единый REST API (Flask)

6. **Extensibility**
   - New devices: add new class in `devices/`, register in config.
   - New protocols: add new adapter in `protocols/`.

7. **Testing & validation**
   - Add unit tests for device logic and protocol adapters.
   - Provide example configs and usage scripts.

---

**Best practices:**
- Separate device logic from protocol logic (SRP, dependency injection).
- Use YAML/JSON for flexible configuration.
- Document all commands and configuration options in README.
- Keep code modular and easy to extend.
