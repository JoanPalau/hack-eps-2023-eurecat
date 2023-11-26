import json
import paho.mqtt.client as mqtt


client = mqtt.Client()
client.username_pw_set("grupo_2", "grupo_2")
client.connect("84.88.76.18", 1883, 60)


def format_data(data):
    return json.dumps({
        "dhtmode": data.get("dhtmode") or -1,
        "pump": data.get("pump") or -1,
        "day": data.get("day") if data.get('day') is not None else -1,
        "fruit": data.get("fruit") or -1,
    })


def publish_data(data):
    client.publish("hackeps/G2", format_data(data))
