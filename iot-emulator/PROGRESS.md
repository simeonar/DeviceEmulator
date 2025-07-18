# Project Progress

- [x] Project structure created according to plan
- [x] C++ core folders and stubs added
- [x] Python monitor (Flask) stub added
- [x] Device and protocol stubs added
- [x] CMake and Python environment configured
- [x] Build/test run and working
- [x] Device status tracking and DeviceManager implemented

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
