from flask import Flask, render_template, jsonify, request
from backend_bridge import BackendBridge

app = Flask(__name__)
bridge = BackendBridge()

@app.route('/')
def index():
    return render_template('index.html')

@app.route("/api/devices", methods=["GET"])
def get_devices():
    devices = []
    for d in bridge.list_devices():
        dev = d.copy()
        # Получаем сценарии для каждого устройства
        full = bridge.get_device(dev["id"])
        dev["scenarios"] = full.get("scenarios", []) if full else []
        devices.append(dev)
    return jsonify(devices)

@app.route("/api/devices/<device_id>", methods=["GET"])
def get_device(device_id):
    dev = bridge.get_device(device_id)
    if not dev:
        return jsonify({"error": "Device not found"}), 404
    return jsonify(dev)

@app.route("/api/devices/<device_id>/start", methods=["POST"])
def start_device(device_id):
    ok, msg = bridge.start_device(device_id)
    if not ok:
        return jsonify({"error": msg}), 404
    return jsonify({"result": msg})

@app.route("/api/devices/<device_id>/stop", methods=["POST"])
def stop_device(device_id):
    ok, msg = bridge.stop_device(device_id)
    if not ok:
        return jsonify({"error": msg}), 404
    return jsonify({"result": msg})

@app.route("/api/devices/<device_id>/status", methods=["GET"])
def get_status(device_id):
    return jsonify(bridge.get_status(device_id))

@app.route("/api/devices/<device_id>/history", methods=["GET"])
def get_history(device_id):
    return jsonify(bridge.get_history(device_id))

@app.route("/api/devices/<device_id>/scenarios", methods=["GET"])
def get_device_scenarios(device_id):
    dev = bridge.get_device(device_id)
    if not dev:
        return jsonify({"error": "Device not found"}), 404
    return jsonify(dev["scenarios"])

@app.route("/api/devices/<device_id>/scenarios/<scenario>", methods=["POST"])
def run_device_scenario(device_id, scenario):
    ok, msg = bridge.run_scenario(device_id, scenario)
    if not ok:
        return jsonify({"error": msg}), 404
    return jsonify({"result": msg})

@app.route("/api/devices/<device_id>/scenarios", methods=["POST"])
def add_scenario(device_id):
    scenario = request.json.get("scenario")
    ok, msg = bridge.add_scenario(device_id, scenario)
    if not ok:
        return jsonify({"error": msg}), 400
    return jsonify({"result": msg})

@app.route("/api/devices/<device_id>/scenarios/<scenario>", methods=["DELETE"])
def delete_scenario(device_id, scenario):
    ok, msg = bridge.delete_scenario(device_id, scenario)
    if not ok:
        return jsonify({"error": msg}), 400
    return jsonify({"result": msg})

if __name__ == '__main__':
    app.run(debug=True)
