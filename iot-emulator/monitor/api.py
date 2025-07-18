from flask import Flask, jsonify, request
from backend_bridge import BackendBridge

app = Flask(__name__)
bridge = BackendBridge()

@app.route("/api/devices", methods=["GET"])
def get_devices():
    # List all devices with brief info
    return jsonify([
        {"id": d["id"], "name": d["name"], "status": d["status"], "protocol": d["protocol"]}
        for d in bridge.list_devices()
    ])

@app.route("/api/devices/<device_id>", methods=["GET"])
def get_device(device_id):
    # Detailed info about the device
    dev = bridge.get_device(device_id)
    if not dev:
        return jsonify({"error": "Device not found"}), 404
    return jsonify(dev)

@app.route("/api/devices/<device_id>/scenarios", methods=["GET"])
def get_device_scenarios(device_id):
    dev = bridge.get_device(device_id)
    if not dev:
        return jsonify({"error": "Device not found"}), 404
    return jsonify(dev["scenarios"])

@app.route("/api/devices/<device_id>/scenarios/<scenario>", methods=["POST"])
def run_device_scenario(device_id, scenario):
    # This is where the C++ backend will be called to run the scenario
    ok, msg = bridge.run_scenario(device_id, scenario)
    if not ok:
        return jsonify({"error": msg}), 404
    return jsonify({"result": msg})

# --- CRUD for scenarios ---
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

if __name__ == "__main__":
    app.run(port=5001, debug=True)
