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

## DeviceRunner: универсальный запуск устройств

DeviceRunner — это универсальный модуль для запуска любого устройства в отдельном фоновом потоке. Каждый эмулируемый девайс оборачивается в DeviceRunner, который управляет его жизненным циклом (start/stop) и обеспечивает независимую работу устройства в фоне. Это позволяет:
- Запускать несколько устройств параллельно, каждое в своём потоке
- Гибко управлять состоянием устройств через DeviceManager
- Легко расширять систему новыми типами устройств без изменения логики запуска

**Принцип работы:**
- Для каждого устройства создаётся объект DeviceRunner, которому передаётся shared_ptr<DeviceBase>.
- При старте DeviceRunner запускает поток, в котором вызывается device->start() и поддерживается работа устройства (например, публикация данных, эмуляция событий).
- Остановка устройства корректно завершает поток и освобождает ресурсы.
- DeviceManager хранит map<имя, shared_ptr<DeviceRunner>> и управляет всеми устройствами через раннеры.

## How to build and run

### Build C++ project

```sh
cmake -S iot-emulator -B build && cmake --build build
```

### Run Python monitor server

```sh
.venv/Scripts/python.exe iot-emulator/monitor/app.py
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
