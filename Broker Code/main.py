'''
    Created on June 2021
    Nico Renaldo <nicorenald@gmail.com>
'''

# Receive data from ESP32 and send it to GCP
from google.cloud import pubsub_v1
import os
import json
import paho.mqtt.client as mqtt
from config import MQTT_Custom

# Credentials GCP
os.environ['GOOGLE_APPLICATION_CREDENTIALS'] = "C:\\Nico\\RandomStuff\\Programming Project\\Bangkit\\xenon-anthem-312407-cb2e4d941db7.json"

# GCP Variables
project_id = 'xenon-anthem-312407'
topic_id = 'testpub'
publisher = pubsub_v1.PublisherClient()
topic_path = publisher.topic_path(project_id, topic_id)

# Variable to store incoming byte data
stored_data = b''
payload = {
    "idDevice" : "00002",
    "Longitude" : 113.921327,
    "Latitude" : -0.789275,
}

def save():
    global stored_data
    f = open("./test.wav", "wb")
    f.write(stored_data)
    f.close()
    print("Saving file done")
    stored_data = b''

def send(payload):
    f = open("./test.wav", "rb")
    payload["data"] = bytearray(f.read())
    f.close()
    data = json.dumps(payload).encode("utf-8")
    future = publisher.publish(topic_path, data)
    print(future.result())
    print(f"Published messages to {topic_path}.")

def on_message(mqttc, obj, msg):
    global stored_data
    if(msg.topic != "res/data"):
        print("Received message on topic :", msg.topic)

    if(msg.topic == "res/status"):
        payload = msg.payload.decode("utf-8")
        print("Status = {}".format(payload))
        if(payload == "send-done"):
            save()
    elif(msg.topic == "res/data"):
        stored_data += msg.payload
    elif(msg.topic == "req"):
        payload = msg.payload.decode("utf-8")
        print("Message : {}".format(payload))
    # stored_data += msg.payload    
    # i=9;
    # payload = {
    #     "idDevice" : "0000"+ str(i),
    #     "Longitude" : 113.921327+i,
    #     "Latitude" : -0.789275+i,
    #     "data" : "asd"
    # }
    # data = json.dumps(payload).encode("utf-8")
    # future = publisher.publish(topic_path, data)
    # print(f"Published messages to GCP : {topic_path}.")

# Credentials Local Broker
mqttx = MQTT_Custom()
mqttx.connect()
mqttx.mqttc.on_message = on_message

mqttx.subscribe("res/+")
mqttx.subscribe("req")
mqttx.loop_forever()
