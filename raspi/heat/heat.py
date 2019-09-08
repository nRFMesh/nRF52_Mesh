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

def heater_notify(name,message):

    topic = "zig/living heat/set"
    json_msg = {"eurotronic_system_mode":2**(eurotronic_system_mode[message])}
    text_msg = json.dumps(json_msg)
    clientMQTT.publish(topic,text_msg)

    return

def check_all_contacts(apertures):
    ''' if any is open then all are open and contact => False'''
    res = True
    for name,aperture in apertures.items():
        if(aperture["contact"] == False):
            res = False
    #print(f"res={res} for {apertures}")
    return res

def window_open(name,payload):
    global apertures
    sensor = json.loads(payload)
    prev_all_contacts = check_all_contacts(apertures)
    apertures[name]["contact"] = sensor["contact"]
    current_all_contacts = check_all_contacts(apertures)
    log.debug(f"{name} => contact = {sensor['contact']}")
    if(current_all_contacts != prev_all_contacts):
        if(current_all_contacts):
            heater_notify("living heat","disable open window")
            log.info("state changed : living heat => disable open window")
        else:
            heater_notify("living heat","set open window")
            log.info("state changed : living heat => set open window")
    else:
        log.debug(f" no state change all_contact = {current_all_contacts}")
    return


def mqtt_on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split('/')
        if(len(topic_parts) == 2 and topic_parts[0] == "zig"):
            name = topic_parts[1]
            if(name == "balcony door") or (name == "balcony window left") or (name == "balcony window right"):
                window_open(name,msg.payload)
        else:
            log.error("topic: "+msg.topic + "size not matching")
    except Exception as e:
        log.error("mqtt_on_message> Exception :%s"%e)
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

# -------------------- Mqtt Client -------------------- 
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,True)

apertures = {
    "balcony door":{"contact":False},
    "balcony window left":{"contact":False},
    "balcony window right":{"contact":False}
    }

state = {
    "living heat":{"apertures":apertures},
    "all_contact":False
    }



while(True):
    sleep(0.2)
    #The MQTT keeps looping on a thead
    #All there is to do here is not to exit
