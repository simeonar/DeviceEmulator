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
