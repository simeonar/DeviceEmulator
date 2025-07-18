# IoT Device Emulator

Modular, extensible emulator for IoT devices with C++ core and Python monitoring.

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
