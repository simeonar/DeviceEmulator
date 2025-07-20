# IoT Device Emulator

Modular, extensible emulator for IoT devices with C++ core and Python monitoring.

## System Architecture

**Key components:**
- **DeviceRunner**: Universal background runner for any device. Each device runs in its own thread, managed by DeviceManager.
- **DeviceManager**: Centralized device registry and lifecycle manager. Provides access to all devices and their runners.
- **REST API (Flask, Python)**: Unified backend for both web interface and CLI. Exposes endpoints for device status, user data, protocol info, scenario management (list, run, edit).
- **Web interface (monitor)**: Visualizes device/user data, protocol/service info, allows scenario selection and editing for each device (dropdown + editor UI).
- **Console (CLI)**: Allows direct device management, scenario execution, and data streaming via command line. CLI communicates with backend via REST API for full synchronization with web interface.

**How it works:**
- All device logic and scenario execution is managed by DeviceManager/DeviceRunner in C++.
- Python Flask REST API provides endpoints for device list, status, user/service data, scenario management (list, run, CRUD).
- Web UI and CLI both use the same REST API, so all actions (start/stop device, run scenario, edit scenario) are reflected everywhere.
- Scenario list for each device is available as a dropdown in the web UI, with an editor for scenario content.
- CLI can trigger scenarios and receive live data, fully synchronized with the web interface.

**Benefits:**
- Single source of truth for device state and scenarios
- Easy extensibility (new devices, protocols, scenarios)
- Unified user experience in web and CLI
- Modular, scalable, and maintainable architecture


## DeviceRunner: universal device runner

DeviceRunner is a universal module for running any device in a separate background thread. Each emulated device is wrapped in a DeviceRunner, which manages its lifecycle (start/stop) and ensures independent operation in the background. This allows:
- Running multiple devices in parallel, each in its own thread
- Flexible device state management via DeviceManager
- Easy extension with new device types without changing the runner logic

**How it works:**
- For each device, a DeviceRunner object is created, receiving a shared_ptr<DeviceBase>.
- On start, DeviceRunner launches a thread that calls device->start() and maintains device operation (e.g., data publishing, event emulation).
- Stopping a device cleanly terminates the thread and releases resources.
- DeviceManager stores a map<name, shared_ptr<DeviceRunner>> and manages all devices via runners.



## How to add and configure a device

All devices are described in the `config/devices.yaml` file. Example:

```yaml
temperature_sensor:
  class: TemperatureSensor
  protocol: mqtt
  port: 1883
  mqtt:
    host: 127.0.0.1
    port: 1883
    topic: sensors/temperature
  initial_state: inactive
  scenarios:
    - overheat
    - disconnect

pressure_valve:
  class: PressureValve
  protocol: opcua
  port: 4840
  initial_state: inactive
  scenarios:
    - leak
    - jam
```


**To add a new device:**
1. Add a section with a unique name (e.g., `my_device:`).
2. Specify the device class (`class`), supported protocol (`protocol`: mqtt, opcua, rest, etc.), port, and protocol parameters.
3. For MQTT, specify parameters in the `mqtt` section (host, port, topic).
4. For OPC UA — only port and class name are needed.
5. Add a list of scenarios (scenarios) if required.

**Пример для MQTT:**
```yaml
my_mqtt_device:
  class: MyMqttDevice
  protocol: mqtt
  mqtt:
    host: 127.0.0.1
    port: 1883
    topic: sensors/mydevice
  initial_state: active
  scenarios:
    - custom_scenario
```

**Пример для OPC UA:**
```yaml
my_opcua_device:
  class: MyOpcuaDevice
  protocol: opcua
  port: 4840
  initial_state: active
  scenarios:
    - opcua_scenario
```


After editing `devices.yaml`, restart the services to apply changes.

---

## How to build and run



### Automatic launch of all services

It is recommended to use PowerShell scripts to start and stop the entire system:

```pwsh
powershell -ExecutionPolicy Bypass -File ../start_all.ps1
```


This will start:
- MQTT broker (Docker)
- C++ REST server
- Python monitor server


To stop all services, use:

```pwsh
powershell -ExecutionPolicy Bypass -File ../stop_all.ps1
```

---


### Manual launch (alternative)

**Build C++ project:**
```sh
cmake -S iot-emulator -B build && cmake --build build
```


**Run Python monitor server:**
```pwsh
.venv\Scripts\python.exe iot-emulator\monitor\app.py
```


**Run C++ REST server:**
```pwsh
build\core\Release\rest_server.exe
```


**Run MQTT broker (Docker):**
```sh
docker run -d --name iot-mqtt-broker -p 1883:1883 eclipse-mosquitto
```


**Stop the broker:**
```sh
docker stop iot-mqtt-broker
docker rm iot-mqtt-broker
```

## How to run C++ REST API and integrate with Python

### Start C++ REST server

Build and run the C++ REST server to provide device management API:

```sh
cmake -S iot-emulator -B build && cmake --build build
build/core/Debug/rest_server.exe
```

The server will listen on http://localhost:8081. You can check available devices:
- Open http://localhost:8081/devices in your browser
- Or use curl: `curl http://localhost:8081/devices`

### Python integration (backend_bridge)

The Python backend_bridge module is configured to connect to the C++ REST API at http://localhost:8081. All web and CLI actions (list devices, run scenario) are now performed via real C++ backend.

No additional configuration is needed if both servers run on the same machine and port.

## How to run MQTT broker for device emulation

To test MQTT publishing from devices (e.g., TemperatureSensor), you need a running MQTT broker. The recommended way is to use Mosquitto in Docker:

```sh
# Start MQTT broker (Mosquitto) in Docker:
docker run -d --name iot-mqtt-broker -p 1883:1883 eclipse-mosquitto
```

- The broker will be available at `127.0.0.1:1883`.
- You can use this address in your `devices.yaml` config (see `mqtt.host` and `mqtt.port`).
- Default topic for temperature sensor: `sensors/temperature`.

To stop and remove the broker:
```sh
docker stop iot-mqtt-broker
docker rm iot-mqtt-broker
```

## How to check MQTT publishing

To verify that the TemperatureSensor (or any device) is publishing data to the MQTT broker, use the following command (requires Mosquitto tools):

```sh
mosquitto_sub -h 127.0.0.1 -p 1883 -t sensors/temperature
```

- This will subscribe to the topic used by the temperature sensor (see `devices.yaml`, usually `sensors/temperature`).
- When the device is running and publishing, you will see temperature values printed in the terminal.

If you see numbers appearing every few seconds — publishing works!

## Console usage principles

- The CLI supports two modes:
  - **Single command mode:**
    - Run with a command as an argument, e.g. `iot_emulator_cli.exe test`.
    - The program executes the command and exits.
  - **Interactive mode (REPL):**
    - Run without arguments: `iot_emulator_cli.exe`
    - You will see a prompt `> ` and can enter commands one by one.
    - Type `exit` or `quit` to leave the console.
- Supported commands:
  - `test` — prints a test confirmation message.
  - `exit`/`quit` — exits interactive mode.
  - Any other command — prints an unknown command message.

## External dependencies: Paho MQTT

This project requires Paho MQTT C and C++ libraries for MQTT device emulation. These are not included in the repository due to licensing and size. You must download and place them manually:

1. Download prebuilt Paho MQTT C for Windows (eclipse-paho-mqtt-c-win64-*) from https://github.com/eclipse/paho.mqtt.c/releases
2. Download Paho MQTT C++ sources (paho.mqtt.cpp-*) from https://github.com/eclipse/paho.mqtt.cpp/releases
3. Place both folders in `external/paho/` so you have:
   - `external/paho/eclipse-paho-mqtt-c-win64-1.3.14/include` and `lib`
   - `external/paho/paho.mqtt.cpp-1.3.0/include`, `lib`, and `build` (after build)
4. Build paho.mqtt.cpp as described above before building the main project.

These folders must remain in the project for successful build and linking.
