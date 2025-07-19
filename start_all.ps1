# Автоматический запуск всех сервисов DeviceEmulator
# Запускает: MQTT брокер (Docker), C++ REST сервер, Python монитор

# 1. Запуск MQTT брокера (если не запущен)
Write-Host "[MQTT] Запуск брокера Mosquitto в Docker..."
docker start iot-mqtt-broker 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) {
    docker run -d --name iot-mqtt-broker -p 1883:1883 eclipse-mosquitto
}

# 2. Запуск C++ REST сервера
Write-Host "[C++] Запуск REST сервера..."
Start-Process -NoNewWindow -WorkingDirectory "$PSScriptRoot" -FilePath "build/core/Release/rest_server.exe"

# 3. Запуск Python монитор-сервера
Write-Host "[Python] Запуск монитор-сервера..."
Start-Process -NoNewWindow -WorkingDirectory "$PSScriptRoot" -FilePath ".venv/Scripts/python.exe" -ArgumentList "iot-emulator/monitor/app.py"

Write-Host "Все сервисы запущены!"
