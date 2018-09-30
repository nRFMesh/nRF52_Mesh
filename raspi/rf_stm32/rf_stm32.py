import sys
import logging as log
import argparse
from time import sleep,time
import json
from mqtt import mqtt_start
import socket
import mesh as mesh
import cfg


def send_heat_duration(heat,duration):
    size = 0x07
    control = 0x72
    pid = 0x09
    src = 65
    dst = 56
    mesh.send([ size,control,mesh.pid["heat"],src,dst,int(heat),int(duration)])
    log.info("send heat %s during %s minutes",heat, duration)
    return



def mqtt_on_message(client, userdata, msg):
    if(msg.topic == "Bed Heater"):
        if(len(msg.payload) != 0):
            try:
                params = json.loads(msg.payload)
                if("heat" in params) and ("time_mn" in params):
                    send_heat_duration(params["heat"],params["time_mn"])
            except json.decoder.JSONDecodeError:
                log.error("mqtt_req > json.decoder.JSONDecodeError parsing payload: %s",msg.payload)
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

