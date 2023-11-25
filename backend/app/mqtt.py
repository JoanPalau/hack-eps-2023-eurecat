import json
import paho.mqtt.client as mqtt


client = mqtt.Client()
client.username_pw_set("grupo_2", "grupo_2")
client.connect("84.88.76.18", 1883, 60)


def publish_data(data):
    client.publish("hackeps/eurecat", json.dumps(data))
