#https://github.com/studioimaginaire/phue


#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from time import sleep
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start
import requests

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

def status_notify(notif):
    if(notif == "windows closed"):
        requests.put(config["status"]["blue"]["off"])
        print("status blue off")
    if(notif == "window open"):
        requests.put(config["status"]["blue"]["on"])
        print("status blue on")
    return


def check_all_contacts():
    ''' if any is open then all are open and contact => False'''
    for name,aperture in aperture_state.items():
        if(aperture == False):
            return False
    #print(f"res={res} for {apertures}")
    return True

def window_open(aperture,payload):
    global aperture_state
    sensor = json.loads(payload)
    log.debug(f"{aperture} => contact = {sensor['contact']}")
    prev_all_contacts = check_all_contacts()
    aperture_state[aperture] = sensor["contact"]
    current_all_contacts = check_all_contacts()
    if(current_all_contacts != prev_all_contacts):
        if(current_all_contacts):
            status_notify("windows closed")
            log.info(f"state changed : => windows closed")
        else:
            status_notify("window open")
            log.info(f"state changed : => window open")
    else:
        log.debug(f" no state change all_contact = {current_all_contacts}")
    return


def mqtt_on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split('/')
        if(len(topic_parts) == 2):
            name = topic_parts[1]
            if(name in config["apertures"]):
                window_open(name,msg.payload)
        else:
            log.debug(f"topic: {msg.topic} : heat")
    except Exception as e:
        log.error("mqtt_on_message> Exception :%s"%e)
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

heat_state = {}
aperture_state = {}
for heater in config["heaters"]:
    heat_state[heater] = True
for aperture in config["apertures"]:
    aperture_state[aperture] = True


# -------------------- Mqtt Client -------------------- 
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,True)




while(True):
    sleep(0.2)
    #The MQTT keeps looping on a thead
    #All there is to do here is not to exit
