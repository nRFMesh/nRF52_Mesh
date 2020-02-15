#https://github.com/studioimaginaire/phue


#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from phue import Bridge
#just to get host name
import socket 
from time import sleep
import time
from math import ceil
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start
import threading
import time

def debounce(in_time):
    current_time = time.time()
    delta = current_time - in_time
    return (delta > 2),current_time

debounce_1_prev = 0
def debounce_1():
    global debounce_1_prev
    res,debounce_1_prev = debounce(debounce_1_prev)
    return res

debounce_2_prev = 0
def debounce_2():
    global debounce_2_prev
    res,debounce_2_prev = debounce(debounce_2_prev)
    return res

debounce_3_prev = 0
def debounce_3():
    global debounce_3_prev
    res,debounce_3_prev = debounce(debounce_3_prev)
    return res

def bed_light_button(payload):
    if(debounce_1()):
        log.debug("bed_light_button> taken")
        sensor = json.loads(payload)
        if("click" in sensor and sensor["click"] == "single"):
            if(lights["Bed Malm"].on):
                lights["Bed N"].on = False
                lights["Bed Malm"].on = False
                lights["Bed W"].on = False
                log.debug("bed_light_button> set light off")
            else:
                #switch on and brightness command together so that it does not go to previous level before adjusting the brightness
                b.set_light("Bed Malm", {'on' : True, 'bri' : 254})
                b.set_light("Bed N", {'on' : True, 'bri' : 254})
                b.set_light("Bed W", {'on' : True, 'bri' : 254})
                log.debug("bed_light_button> set light to MAX")
        elif("action" in sensor and sensor["action"] == "hold"):
            b.set_light("Bed Malm", {'on' : True, 'bri' : 1})
            lights["Bed N"].on = False
            lights["Bed W"].on = False
            log.debug("bed_light_button> set light to min")
    #else:
        #log.debug("bed_light_button> debounced")
    return

def bathroom_shelly_light(cmd):
    topic = "shellies/shellyswitch25-B8A4EE/relay/0/command"
    clientMQTT.publish(topic,cmd)
    log.debug(f"set_light_relay> to {cmd}")
    return

def bathroom_light_hue():
    #switch on and brightness command together so that it does not go to previous level before adjusting the brightness
    b.set_light("Bathroom main", {'on' : True, 'bri' : 1})
    b.set_light("Bathroom main", {'on' : True, 'bri' : 1})
    log.debug("bathroom_light_hue> set light to min")
    return

def bathroom_light_button(payload):
    if(debounce_2()):
        log.debug("bathroom light> taken")
        sensor = json.loads(payload)
        if("click" in sensor and sensor["click"] == "single"):
            #state = b.get_light("Bathroom main")
            #if(not state["state"]["reachable"]):
            bathroom_shelly_light("on")
            threading.Timer(1, bathroom_light_hue).start()
            #else:
            #    if(lights["Bathroom main"].on):
            #        lights["Bathroom main"].on = False
            #        log.debug("bathroom light> set light off")
            #    else:
            #        b.set_light("Bathroom main", {'on' : True, 'bri' : 1})
            #        log.debug("bathroom_light_button> set light to min")
        elif("action" in sensor and sensor["action"] == "hold"):
            b.set_light("Bathroom main", {'on' : True, 'bri' : 1})
            log.debug("bathroom light> set light to min")
    return

def livroom_light_button(payload):
    if(debounce_3()):
        log.debug("living room light> taken")
        sensor = json.loads(payload)
        if("click" in sensor and sensor["click"] == "single"):
            if(lights["LivingTop5"].on):
                lights["LivingTop1"].on = False
                lights["LivingTop2"].on = False
                lights["LivingTop3"].on = False
                lights["LivingTop4"].on = False
                lights["LivingTop5"].on = False
                log.debug("living room light> set light off")
            else:
                #switch on and brightness command together so that it does not go to previous level before adjusting the brightness
                b.set_light("LivingTop1", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop2", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop3", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop4", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop5", {'on' : True, 'bri' : 254})
                log.debug("living room light> set light to MAX")
        elif("action" in sensor and sensor["action"] == "hold"):
            b.set_light("LivingTop1", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop2", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop3", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop4", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop5", {'on' : True, 'bri' : 1})
            log.debug("living room light> set light to min")
    return

def office_switch(payload):
    switch = json.loads(payload)
    if("click" in switch and switch["click"] == "single"):
        if(lights["Office main"].on):
            lights["Office main"].on = False
            log.debug("office_light> off")
        else:
            #command so that it does not go to previous level before adjusting the brightness
            b.set_light("Office main", {'on' : True, 'bri' : 255})
            log.debug("office_light> on")
    elif("action" in switch and switch["action"] == "hold"):
            b.set_light("Office main", {'on' : True, 'bri' : 1})
            log.debug("office_light> low")
    #else:
    #    log.debug("office_light>no click")
    return

def entrance_light(payload):
    jval = json.loads(payload)
    if("click" in jval and jval["click"] == "single"):
        if(lights["Entrance White 1"].on):
            lights["Entrance White 1"].on = False
            lights["Entrance White 2"].on = False
            log.debug("entrance_light> off")
        else:
            #command so that it does not go to previous level before adjusting the brightness
            b.set_light("Entrance White 1", {'on' : True, 'bri' : 255})
            b.set_light("Entrance White 2", {'on' : True, 'bri' : 255})
            log.debug("entrance_light> on")
    elif("contact" in jval and jval["contact"] == False):
        #TODO check brightness here - and diff between coming or going away
        log.debug("entrance_door>open")
    else:
        log.debug("entrance_light>no click")
    return

def mqtt_on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split('/')
        if(len(topic_parts) == 2):
            name = topic_parts[1]
            if(name == "bed light button") or (name == "bed nic button"):
                bed_light_button(msg.payload)
            elif(name == "office switch"):
                office_switch(msg.payload)
            elif(name == "tree button"):
                bathroom_light_button(msg.payload)
            elif(name == "liv light 1 button"):
                livroom_light_button(msg.payload)
        else:
            log.error("topic: "+msg.topic + "size not matching")
    except Exception as e:
        log.error("mqtt_on_message> Exception :%s"%e)
    return

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
