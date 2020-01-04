#https://github.com/studioimaginaire/phue


#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from time import sleep
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start

eurotronic_system_mode = {
    "mirror display":1,
    "boost":2,
    "disable open window":4,
    "set open window":5,
    "child protection":7
}

def heater_notify(name,command):
    topic = config["heatings"][name]["topic"]
    json_msg = {"eurotronic_system_mode":2**(eurotronic_system_mode[command])}
    text_msg = json.dumps(json_msg)
    clientMQTT.publish(topic,text_msg)
    return

def check_all_contacts(apertures):
    ''' if any is open then all are open and contact => False'''
    res = True
    for name,aperture in apertures.items():
        if(aperture == False):
            res = False
    #print(f"res={res} for {apertures}")
    return res

def window_open(aperture,payload):
    global state
    sensor = json.loads(payload)
    log.debug(f"{aperture} => contact = {sensor['contact']}")
    heater = topic_to_heater[aperture]
    prev_all_contacts = check_all_contacts(state[heater]["apertures"])
    state[heater]["apertures"][aperture] = sensor["contact"]
    current_all_contacts = check_all_contacts(state[heater]["apertures"])
    if(current_all_contacts != prev_all_contacts):
        if(current_all_contacts):
            heater_notify(heater,"disable open window")
            log.info(f"state changed : {heater} => disable open window")
        else:
            heater_notify(heater,"set open window")
            log.info(f"state changed : {heater} => set open window")
    else:
        log.debug(f" no state change all_contact = {current_all_contacts}")
    return


def mqtt_on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split('/')
        if(len(topic_parts) == 2):
            name = topic_parts[1]
            if(name in topics_list):
                window_open(name,msg.payload)
        else:
            log.error(f"topic: {msg.topic} size not matching")
    except Exception as e:
        log.error("mqtt_on_message> Exception :%s"%e)
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

state = {}
topics_list = []
topic_to_heater = {}
for name,heater in config["heatings"].items():
    state[name] = {}
    state[name]["apertures"] = {}
    for aperture in heater["Apertures"]:
        state[name]["apertures"][aperture] = True
        topics_list.append(aperture)
        topic_to_heater[aperture] = name


# -------------------- Mqtt Client -------------------- 
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,True)




while(True):
    sleep(0.2)
    #The MQTT keeps looping on a thead
    #All there is to do here is not to exit
