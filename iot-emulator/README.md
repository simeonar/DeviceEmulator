# IoT Device Emulator

Modular, extensible emulator for IoT devices with C++ core and Python monitoring.

## How to build and run

### Build C++ project

```sh
cmake -S iot-emulator -B build && cmake --build build
```

### Run Python monitor server

```sh
.venv/Scripts/python.exe iot-emulator/monitor/app.py
```
