import requests

# Stub for integration with C++ backend (DeviceManager/DeviceRunner)
# In the future, can be implemented via pybind11, REST, or IPC

class BackendBridge:
    def __init__(self, base_url="http://localhost:8081"):
        self.base_url = base_url

    def list_devices(self):
        resp = requests.get(f"{self.base_url}/devices")
        resp.raise_for_status()
        return resp.json()

    def get_device(self, device_id):
        # В REST API нет отдельного endpoint, ищем по списку
        for dev in self.list_devices():
            if dev["id"] == device_id:
                return dev
        return None

    def run_scenario(self, device_id, scenario):
        url = f"{self.base_url}/devices/{device_id}/scenarios/{scenario}"
        resp = requests.post(url)
        if resp.status_code == 200:
            return True, resp.json().get("result", "OK")
        else:
            return False, resp.json().get("error", "Error")

    def add_scenario(self, device_id, scenario):
        # Not implemented in C++ REST API (stub)
        return False, "Not implemented"

    def delete_scenario(self, device_id, scenario):
        # Not implemented in C++ REST API (stub)
        return False, "Not implemented"
