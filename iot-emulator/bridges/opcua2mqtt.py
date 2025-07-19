"""
Bridge OPC UA → MQTT

- Connects to OPC UA server (emulated device)
- Subscribes to node(s) (e.g. PressureValve value)
- Publishes updates to MQTT topic

Usage:
  python opcua2mqtt.py --opcua-endpoint opc.tcp://localhost:4840 --nodeid ns=2;s=PressureValve --mqtt-host 127.0.0.1 --mqtt-port 1883 --mqtt-topic sensors/pressure
"""
import argparse
from asyncua import Client
import paho.mqtt.client as mqtt
import asyncio

async def main(args):
    mqttc = mqtt.Client()
    mqttc.connect(args.mqtt_host, args.mqtt_port)
    async with Client(url=args.opcua_endpoint) as client:
        node = client.get_node(args.nodeid)
        print(f"Subscribed to OPC UA node: {args.nodeid}")
        while True:
            val = await node.read_value()
            mqttc.publish(args.mqtt_topic, str(val))
            await asyncio.sleep(args.interval)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--opcua-endpoint', required=True)
    parser.add_argument('--nodeid', required=True)
    parser.add_argument('--mqtt-host', default='127.0.0.1')
    parser.add_argument('--mqtt-port', type=int, default=1883)
    parser.add_argument('--mqtt-topic', default='sensors/pressure')
    parser.add_argument('--interval', type=float, default=1.0, help='Polling interval (s)')
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    asyncio.run(main(args))
