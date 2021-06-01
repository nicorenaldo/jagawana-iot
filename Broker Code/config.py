'''
    Created on June 2021
    Nico Renaldo <nicorenald@gmail.com>
'''

import paho.mqtt.client as mqtt

class MQTT_Custom:
    def __init__(self, mqttHost="192.168.43.71", mqttPort=1883):
        self.mqttHost = mqttHost
        self.mqttPort = mqttPort

    def on_connect(self, mqttc, obj, flags, rc):
        print("MQTT Established...")
        print("rc: "+str(rc))

    def on_message(self, mqttc, obj, msg):
        print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))

    def on_publish(self, mqttc, obj, mid):
        print("mid: "+str(mid))

    def on_subscribe(self, mqttc, obj, mid, granted_qos):
        print("Subscribed: "+str(mid)+" "+str(granted_qos))

    def connect(self):
        self.mqttc = mqtt.Client()
        self.mqttc.on_connect = self.on_connect
        self.mqttc.on_subscribe = self.on_subscribe
        self.mqttc.on_message = self.on_message
        self.mqttc.connect(self.mqttHost, self.mqttPort, 60)

    def publish(self, topic, msg):
        self.mqttc.publish(topic, msg)
    
    def subscribe(self, topic):
        self.mqttc.subscribe(topic)
    
    def loop_forever(self):
        self.mqttc.loop_forever()