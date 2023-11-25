import json
import datetime
import os
import psycopg2
from psycopg2 import sql
from urllib.parse import urlparse
import paho.mqtt.client as mqtt


def get_db_connection():
    result = urlparse(os.environ["HACKEPS_DATABASE_URI"])
    username = result.username
    password = result.password
    database = result.path[1:]
    hostname = result.hostname
    port = result.port
    return psycopg2.connect(
        database=database,
        user=username,
        password=password,
        host=hostname,
        port=port
        )


def save_to_db(data):
    json_data = json.loads(data.decode('utf-8'))
    device_id = json_data['id']
    light = json_data['light']
    soil_humidity = json_data['soil_humidity']
    air_humidity = json_data['air_humidity']
    temperature = json_data['temperature']

    conn = get_db_connection()

    cursor = conn.cursor()

    insert_query = sql.SQL("""
                           INSERT INTO data (
                               device_id, light, soil_humidity, air_humidity,
                               temperature, created_at)
                           VALUES (%s, %s, %s, %s, %s, %s)
                           """)

    try:
        cursor.execute(insert_query, (device_id, light, soil_humidity,
                                      air_humidity, temperature,
                                      datetime.datetime.now()))
        conn.commit()
        print("Data added to the database successfully.")
    except psycopg2.IntegrityError:
        conn.rollback()
        print("Data with the same id already exists in the database.")

    conn.close()
    cursor.close()


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("hackeps/eurecat")


def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    save_to_db(msg.payload)
    # more callbacks, etc


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.publish("hackeps/eurecat", b'{"humidity": "all"}')
client.username_pw_set("grupo_2", "grupo_2")
client.connect("84.88.76.18", 1883, 60)

client.loop_forever()
