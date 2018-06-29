#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
#https://github.com/McSwindler/python-milight
import milight
from milight import Command
#just to get host name
import socket 
from time import sleep
from math import ceil
import logging as log
import sys,os
import cfg

def on_connect(lclient, userdata, flags, rc):
    topic_sub = "Nodes/+/dimmer"
    lclient.subscribe(topic_sub)
    log.info("Subscribed to: "+topic_sub)

def on_message(client, userdata, msg):
    topic_parts = msg.topic.split('/')
    if(len(topic_parts) == 3):
        nodeid = topic_parts[1]
        if(nodeid in config["mapping"]):
            device_name = config["mapping"][nodeid]["device"]
            channel = config["mapping"][nodeid]["channel"]
            dimm_val = int(ceil(float(msg.payload)))
            if(dimm_val > 100):
                dimm_val = 100
            log.debug(  "Action to Node: "+nodeid               +
                    " ; through gateway: "+device_name      +
                    " ; on channel: "+str(channel)            +
                    " ; set value: "+str(dimm_val)
                    )
            controller = milight_controllers[device_name]
            if(dimm_val == 0):
                controller.send(light.off(channel))
            elif(dimm_val == 1):
                controller.send(light.off(channel))
                sleep(0.100)#100 ms
                controller.send(Command(night_mode[channel]))
            else:
                controller.send(light.brightness(dimm_val,channel))
        else:
            log.warning("Node "+nodeid+" route unknown")
    else:
        log.error("topic: "+msg.topic + "size not matching")
        

def mqtt_connect_retries(client):
    connected = False
    while(not connected):
        try:
            client.connect(config["mqtt"]["host"], config["mqtt"]["port"], config["mqtt"]["keepalive"])
            connected = True
            log.info(  "mqtt connected to "+config["mqtt"]["host"]+":"+str(config["mqtt"]["port"])+" with id: "+ cid )
        except socket.error:
            log.error("socket.error will try a reconnection in 10 s")
        sleep(10)
    return

# -------------------- main -------------------- 
config = cfg.get_local_json("config_milight.json")

cfg.configure_log(config["log"])

log.info("milight client started")

# -------------------- Milight Client -------------------- 
milight_controllers = {}
for device_name,device in config["devices"].items():
    milight_controllers[device_name] = milight.MiLight(device)

#light = milight.LightBulb(['rgbw','white','rgb'])
light = milight.LightBulb(['rgbw'])
night_mode = [0xC1, 0xC6, 0xC8, 0xCA, 0xCC]
# -------------------- Mqtt Client -------------------- 
cid = config["mqtt"]["client_id"] +"_"+socket.gethostname()
client = mqtt.Client(client_id=cid)
client.on_connect = on_connect
client.on_message = on_message

mqtt_connect_retries(client)

client.loop_forever()

