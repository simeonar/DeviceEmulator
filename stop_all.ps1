# Остановка всех сервисов DeviceEmulator
# Останавливает: MQTT брокер (Docker), C++ REST сервер, Python монитор

# 1. Остановить MQTT брокер (Docker)
Write-Host "[MQTT] Остановка брокера Mosquitto..."
docker stop iot-mqtt-broker 2>$null | Out-Null

# 2. Остановить C++ REST сервер
Write-Host "[C++] Остановка rest_server.exe..."
Get-Process rest_server -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "*DeviceEmulator*" } | Stop-Process -Force

# 3. Остановить Python монитор-сервер
Write-Host "[Python] Остановка python.exe (монитор)..."
Get-Process python -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "*DeviceEmulator*" } | Stop-Process -Force

Write-Host "Все сервисы остановлены!"
