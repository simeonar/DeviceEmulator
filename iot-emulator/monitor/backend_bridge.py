# Заглушка для интеграции с C++ backend (DeviceManager/DeviceRunner)
# В будущем можно реализовать через pybind11, REST или IPC

class BackendBridge:
    def __init__(self):
        # Здесь можно инициализировать pybind11 binding или IPC клиент
        pass

    def list_devices(self):
        # TODO: получить список устройств из C++ backend
        return [
            {
                "id": "temperature_sensor",
                "name": "TemperatureSensor",
                "status": "Running",
                "protocol": "MQTT",
                "data": {"temperature": 23, "timestamp": "2025-07-18T12:00:00"},
                "scenarios": ["overheat", "disconnect"]
            }
        ]

    def get_device(self, device_id):
        # TODO: получить инфу по устройству из C++ backend
        for dev in self.list_devices():
            if dev["id"] == device_id:
                return dev
        return None

    def run_scenario(self, device_id, scenario):
        # TODO: вызвать C++ backend для запуска сценария
        dev = self.get_device(device_id)
        if not dev or scenario not in dev["scenarios"]:
            return False, "Device or scenario not found"
        # Здесь должен быть реальный вызов
        return True, f"Scenario '{scenario}' started for device '{device_id}'"

    def add_scenario(self, device_id, scenario):
        # TODO: добавить сценарий через C++ backend
        return True, "Scenario added (stub)"

    def delete_scenario(self, device_id, scenario):
        # TODO: удалить сценарий через C++ backend
        return True, "Scenario deleted (stub)"
