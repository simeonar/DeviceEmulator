from flask import Flask, jsonify, request

app = Flask(__name__)

# --- MOCK DATA ---
# В реальной интеграции эти данные будут приходить из C++ backend через bindings или IPC
MOCK_DEVICES = {
    "temperature_sensor": {
        "name": "TemperatureSensor",
        "status": "Running",
        "protocol": "MQTT",
        "data": {"temperature": 23, "timestamp": "2025-07-18T12:00:00"},
        "scenarios": ["overheat", "disconnect"]
    }
}

@app.route("/api/devices", methods=["GET"])
def get_devices():
    # Список всех устройств с краткой инфой
    return jsonify([
        {"id": k, "name": v["name"], "status": v["status"], "protocol": v["protocol"]}
        for k, v in MOCK_DEVICES.items()
    ])

@app.route("/api/devices/<device_id>", methods=["GET"])
def get_device(device_id):
    # Подробная инфа по устройству
    dev = MOCK_DEVICES.get(device_id)
    if not dev:
        return jsonify({"error": "Device not found"}), 404
    return jsonify(dev)

@app.route("/api/devices/<device_id>/scenarios", methods=["GET"])
def get_device_scenarios(device_id):
    dev = MOCK_DEVICES.get(device_id)
    if not dev:
        return jsonify({"error": "Device not found"}), 404
    return jsonify(dev["scenarios"])

@app.route("/api/devices/<device_id>/scenarios/<scenario>", methods=["POST"])
def run_device_scenario(device_id, scenario):
    # Здесь будет вызов C++ backend для запуска сценария
    dev = MOCK_DEVICES.get(device_id)
    if not dev or scenario not in dev["scenarios"]:
        return jsonify({"error": "Device or scenario not found"}), 404
    # TODO: интеграция с DeviceManager/DeviceRunner
    return jsonify({"result": f"Scenario '{scenario}' started for device '{device_id}'"})

# --- CRUD для сценариев (заглушки) ---
@app.route("/api/devices/<device_id>/scenarios", methods=["POST"])
def add_scenario(device_id):
    # Добавить сценарий (заглушка)
    return jsonify({"result": "Scenario added (stub)"})

@app.route("/api/devices/<device_id>/scenarios/<scenario>", methods=["DELETE"])
def delete_scenario(device_id, scenario):
    # Удалить сценарий (заглушка)
    return jsonify({"result": "Scenario deleted (stub)"})

if __name__ == "__main__":
    app.run(port=5001, debug=True)
