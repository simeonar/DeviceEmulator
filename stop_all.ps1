# Stop all DeviceEmulator services
# Stops: MQTT broker (Docker), C++ REST server, Python monitor

# 1. Stop MQTT broker (Docker)
Write-Host "[MQTT] Stopping Mosquitto broker..."
docker stop iot-mqtt-broker 2>$null | Out-Null

# 2. Stop C++ REST server
Write-Host "[C++] Stopping rest_server.exe..."
Get-Process rest_server -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "*DeviceEmulator*" } | Stop-Process -Force

# 3. Stop Python monitor server
Write-Host "[Python] Stopping python.exe (monitor)..."
Get-Process python -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "*DeviceEmulator*" } | Stop-Process -Force

Write-Host "All services stopped!"
