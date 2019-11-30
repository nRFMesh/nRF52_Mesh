#https://github.com/studioimaginaire/phue


#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from time import sleep
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start
import threading


def set_fan(fan_val):
    topic = "shellies/shellyswitch25-B8A4EE/relay/1/command"
    clientMQTT.publish(topic,fan_val)
    log.debug(f"fan set to {fan_val} state: ({state})")
    return

def light_active_5_min():
    log.debug(f"timer> trigger")
    if(state["light"] == True):
        set_fan("on")
    return

def activate_switch():
    #start timer
    threading.Timer(60*5, light_active_5_min).start()
    log.debug(f"activate switch  state:({state})")
    return

def deactivate_switch():
    #check humidity and stop
    if(state["humidity"] < 60):
        set_fan("off")
    log.debug(f"deactivate switch state:({state})")
    return

def bathroom_switch(payload):
    global state
    switch_state = int(payload)
    if(switch_state == 1) and (state["light"] == False):
        state["light"] = True
        activate_switch()
    elif(switch_state == 0) and (state["light"] == True):
        state["light"] = False
        deactivate_switch()
    return

def bathroom_humidity(payload):
    humidity_level = float(payload)
    if(state["humidity"] >= 60) and (humidity_level < 60):
        if(state["light"] == False):
            log.debug(f"humidity_act> off")
            set_fan("off")
    elif(state["humidity"] <= 70) and (humidity_level > 70):
        set_fan("on")
        log.debug(f"humidity_act> on")
    return

def mqtt_on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split('/')
        if(len(topic_parts) == 4 and topic_parts[0] == "shellies"):
            name = topic_parts[1]
            if(name == "shellyswitch25-B8A4EE"):
                bathroom_switch(msg.payload)
        elif(len(topic_parts) == 3 and topic_parts[0] == "Nodes"):
            if((topic_parts[1] == "82") and(topic_parts[2] == "humidity")):
                bathroom_humidity(msg.payload)
        else:
            log.error("unknown topic: "+msg.topic)
    except Exception as e:
        log.error("mqtt_on_message> Exception :%s"%e)
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

# -------------------- Mqtt Client -------------------- 
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,True)

state = {
    "light":False,
    "humidity":50.0
    }



while(True):
    sleep(0.2)
    #The MQTT keeps looping on a thead
    #All there is to do here is not to exit
