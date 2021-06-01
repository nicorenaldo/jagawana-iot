'''
    Created on June 2021
    Nico Renaldo <nicorenald@gmail.com>
'''

from config import MQTT_Custom
mqttx = MQTT_Custom()
mqttx.connect()
mqttx.publish("req", "record")