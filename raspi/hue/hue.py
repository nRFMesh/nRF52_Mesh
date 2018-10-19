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
    if(name == "lumi.remote.b1acn01 16") or (name == "LargeSingleSwitch"):
        log.debug("LargeSingleSwitch - pressed")
        if(lights["Bed Leds Cupboard"].on):
            lights["Bed Leds Cupboard"].on = False
        else:
            #command so that it does not go to previous level before adjusting the brightness
            b.set_light("Bed Leds Cupboard", {'on' : True, 'bri' : 1})
    return

def stairs_off_callback():
    lights["Stairs Up Left"].on = False
    lights["Stairs Down Right"].on = False
    log.debug("Stairs - off_callback")
    return

g_stairs_up_light = 0.0
g_stairs_down_light = 0.0

def stairs_presence(name,payload):
    global g_stairs_up_light
    global g_stairs_down_light
    jval = json.loads(payload)
    if("light" in jval):
        if(name == "MotionLight 1"):
            g_stairs_up_light = float(jval["light"])
            log.debug("MotionLight Up - light : %f"%g_stairs_up_light)
        if(name == "MotionLightHue"):
            g_stairs_down_light = float(jval["light"])
            log.debug("MotionLightHue - light : %f"%g_stairs_down_light)
    if("presence" in jval):
        if(jval["presence"]):
            if(g_stairs_up_light < 2):
                brightness = 254
            elif(g_stairs_up_light < 25):
                brightness = 128
            else:
                brightness = 0
            if(brightness > 0):
                if(name == "MotionLight 1"):
                    log.debug(f"MotionLight Up - presence - brightness:{brightness}")
                    b.set_light("Stairs Up Left", {'transitiontime' : 30, 'on' : True, 'bri' : brightness})
                    b.set_light("Stairs Down Right", {'transitiontime' : 10, 'on' : True, 'bri' : int(brightness/2)})
                    threading.Timer(60, stairs_off_callback).start()
                if(name == "MotionLightHue"):
                    log.debug(f"MotionLight Down - presence - brightness:{brightness}")
                    b.set_light("Stairs Down Right", {'transitiontime' : 10, 'on' : True, 'bri' : brightness})
                    b.set_light("Stairs Up Left", {'transitiontime' : 30, 'on' : True, 'bri' : int(brightness/2)})
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
        elif((modelid == "lumi.sensor_motion.aq2") or (modelid == "SML001") ):
            stairs_presence(name,msg.payload)
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
