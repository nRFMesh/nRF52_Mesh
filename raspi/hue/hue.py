#https://github.com/studioimaginaire/phue

#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from phue import Bridge
#just to get host name
import socket 
from time import sleep
from math import ceil
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start
import threading

def aqara_cube(name,payload):
    if(name == "Aqara Cube 1"):
        log.debug("Aqara Cube 1 - action")
        jval = json.loads(payload)
        if("event" in jval):
            if(jval["event"] == "flip"):
                lights["Stairs Up Left"].on = not lights["Stairs Up Left"].on
    return

def wall_switch(name):
    if(name == "LargeSingleSwitch"):
        log.debug("LargeSingleSwitch - pressed")
        if(lights["Bed Leds Cupboard"].on):
            lights["Bed Leds Cupboard"].on = False
        else:
            lights["Bed Leds Cupboard"].on = True
            lights["Bed Leds Cupboard"].brightness = 1
    return

def stairs_off_callback():
    lights["Stairs Up Left"].on = False
    log.debug("Stairs Up Left - off_callback")
    return

def motion_presence(name):
    if(name == "MotionLight 1"):
        log.debug("MotionLight - presence")
        command =  {'transitiontime' : 30, 'on' : True, 'bri' : 254}
        b.set_light("Stairs Up Left", command)
        threading.Timer(60, stairs_off_callback).start()
    return


def mqtt_on_message(client, userdata, msg):
    topic_parts = msg.topic.split('/')
    if(len(topic_parts) == 3):
        modelid = topic_parts[1]
        name = topic_parts[2]
        if(modelid == "lumi.sensor_cube.aqgl01"):
            aqara_cube(name,msg.payload)
        elif(modelid == "lumi.sensor_86sw1"):
            wall_switch(name)
        elif(modelid == "lumi.sensor_motion.aq2"):
            motion_presence(name)
    else:
        log.error("topic: "+msg.topic + "size not matching")
        

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

# -------------------- Philips Hue Client -------------------- 
log.info("Check Bridge Presence")

if(cfg.ping(config["bridges"]["LivingRoom"])):
    b = Bridge(config["bridges"]["LivingRoom"])
    log.info("Bridge Connection")
    b.connect()
    log.info("Light Objects retrieval")
    lights = b.get_light_objects('name')

    log.info("Hue Lights available :")
    for name, light in lights.items():
        log.info(name)
    
else:
    log.info("Bridge ip not responding")


# -------------------- Mqtt Client -------------------- 
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,True)

while(True):
    sleep(0.2)
    #The MQTT keeps looping on a thead
    #All there is to do here is not to exit
