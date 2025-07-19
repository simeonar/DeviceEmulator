# Автоматический запуск всех сервисов DeviceEmulator
# Запускает: MQTT брокер (Docker), C++ REST сервер, Python монитор


## 1. Open a window for MQTT topic monitoring (mosquitto_sub)
Write-Host "[MQTT] Opening window for mosquitto_sub..."
Start-Process cmd.exe -ArgumentList "/c start cmd.exe /k `"mosquitto_sub -h 127.0.0.1 -p 1883 -t sensors/temperature`""


# 2. Запуск C++ REST сервера в новом окне
Write-Host "[C++] Запуск REST сервера (отдельное окно)..."
Start-Process cmd.exe -ArgumentList "/c start cmd.exe /k `"cd /d $PSScriptRoot && build/core/Release/rest_server.exe`""

# 3. Запуск Python монитор-сервера в новом окне
Write-Host "[Python] Запуск монитор-сервера (отдельное окно)..."
Start-Process cmd.exe -ArgumentList "/c start cmd.exe /k `"cd /d $PSScriptRoot && .venv\\Scripts\\python.exe iot-emulator\\monitor\\app.py`""

Write-Host "Все сервисы запущены в отдельных окнах!"
