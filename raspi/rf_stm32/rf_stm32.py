import sys
import logging as log
import argparse
from time import sleep,time
import json
from mqtt import mqtt_start
import socket
import mesh as mesh
import cfg


def serial_send_heat_duration(heat,duration):
    size = 0x07
    control = 0x72
    pid = 0x09
    src = config["heat"]["node_src"]
    dst = config["heat"]["node_dst"]
    mesh.send([ size,control,mesh.pid["heat"],src,dst,int(heat),int(duration)])
    log.info("send heat %s during %s minutes",heat, duration)
    return

#tmsg 0x07 0x71 0x0D 0x41 0x19 0x06 0xD0
def serial_send_brightness_all(brightness):
    bri = int(brightness)
    size = 0x07
    control = 0x71
    pid = 0x0D
    src = config["light"]["node_src"]
    dst = config["light"]["node_dst"]
    brightness_high = bri / 256
    brightness_low = bri % 256
    mesh.send([ size,control,mesh.pid["dimmer"],src,dst,int(brightness_high),int(brightness_low)])
    log.info("send brightness to all channels %d ",bri)
    return

def mqtt_handle_heat_request(topics,payload):
    if(topics[1] == "json"):
        try:
            params = json.loads(payload)
            if("heat" in params) and ("time_mn" in params):
                serial_send_heat_duration(params["heat"],params["time_mn"])
        except json.decoder.JSONDecodeError:
            log.error("mqtt_req > json.decoder.JSONDecodeError parsing payload: %s",msg.payload)
    elif(topics[1] == "1h"):
        serial_send_heat_duration(int(payload),60)
    elif(topics[1] == "20mn"):
        serial_send_heat_duration(int(payload),20)
    return

def mqtt_handle_dimmer_request(topics,payload):
    if(topics[1] == "all"):
        serial_send_brightness_all(payload)
    elif(topics[1] == "channels"):
        try:
            params = json.loads(payload)
            if("heat" in params) and ("time_mn" in params):
                send_heat_duration(params["heat"],params["time_mn"])
        except json.decoder.JSONDecodeError:
            log.error("mqtt_req > json.decoder.JSONDecodeError parsing payload: %s",payload)
    return    


def mqtt_on_message(client, userdata, msg):
    topics = msg.topic.split('/')
    if(len(topics) == 2) and (len(msg.payload) != 0):
        if(topics[0] == "Bed Heater"):
            mqtt_handle_heat_request(topics,msg.payload)
        elif(topics[0] == "Retro Light Upstairs"):
            mqtt_handle_dimmer_request(topics,msg.payload)
    return

def loop_forever():
    while(True):
        sleep(0.1)
        mesh.run()
        if(config["mqtt"]["enable"]):
            clientMQTT.loop()
    return
def loop(nb):
    while(nb > 0):
        sleep(0.05)
        mesh.run()
        nb = nb - 1
    return

def mesh_on_cmd_response():
    return
def mesh_on_message():
    return
def mesh_on_broadcast(msg):
    #log.info("rf  > %s %s : %s"%(msg["src"],mesh.node_name(msg["src"]),mesh.inv_pid[int(msg["pid"])]))
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,False)

mesh.start(config,mesh_on_broadcast,mesh_on_message,mesh_on_cmd_response)

loop_forever()

