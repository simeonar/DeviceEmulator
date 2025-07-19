# OPC UA → MQTT Bridge

Этот скрипт позволяет пересылать значения из OPC UA сервера (например, PressureValve) в MQTT-топик.

## Требования
- Python 3.8+
- pip install asyncua paho-mqtt

## Запуск
```sh
python opcua2mqtt.py --opcua-endpoint opc.tcp://localhost:4840 --nodeid ns=2;s=PressureValve --mqtt-host 127.0.0.1 --mqtt-port 1883 --mqtt-topic sensors/pressure
```

- `--opcua-endpoint` — адрес OPC UA сервера
- `--nodeid` — идентификатор узла (например, ns=2;s=PressureValve)
- `--mqtt-host`/`--mqtt-port` — параметры MQTT брокера
- `--mqtt-topic` — топик для публикации
- `--interval` — интервал опроса (сек)

## Пример
```
python opcua2mqtt.py --opcua-endpoint opc.tcp://localhost:4840 --nodeid ns=2;s=PressureValve --mqtt-topic sensors/pressure
```

## Описание
Скрипт подключается к OPC UA серверу, периодически читает значение указанного nodeId и публикует его в MQTT.
