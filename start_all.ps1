
# Automatic launch of all DeviceEmulator services
# Launches: MQTT topic monitor, C++ REST server, Python monitor


## 1. Open a window for MQTT topic monitoring (mosquitto_sub)
Write-Host "[MQTT] Opening window for mosquitto_sub..."
Start-Process cmd.exe -ArgumentList "/c start cmd.exe /k `"mosquitto_sub -h 127.0.0.1 -p 1883 -t sensors/temperature`""



## 2. Start C++ REST server in a new window
Write-Host "[C++] Starting REST server (separate window)..."
try {
    Start-Process cmd.exe -ArgumentList "/c start cmd.exe /k `"cd /d $PSScriptRoot && build\core\Release\rest_server.exe`""
} catch {
    Write-Host "[C++] Failed to start REST server: $_" -ForegroundColor Red
}


## 3. Start Python monitor server in a new window
Write-Host "[Python] Starting monitor server (separate window)..."
try {
    Start-Process cmd.exe -ArgumentList "/c start cmd.exe /k `"cd /d $PSScriptRoot && .venv\\Scripts\\python.exe iot-emulator\\monitor\\app.py`""
} catch {
    Write-Host "[Python] Failed to start monitor server: $_" -ForegroundColor Red
}

Write-Host "All services have been started in separate windows!"
