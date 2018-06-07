import sys
import logging as log
import argparse
import cfg
from time import sleep,time
import json
from mqtt import mqtt_start
import socket
import mesh as mesh

def mqtt_on_message(client, userdata, msg):
    log.info("mqtt> %s : %s",msg.topic,msg.payload)
    return

def mesh_on_broadcast(msg):
    log.info("id : %s",msg["id"])
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

def set_channel(chan):
    print("set_channel:",chan)
    mesh.command("set_channel",[chan])
    loop(2)
    return
def get_channel():
    mesh.command("get_channel",[])
    loop(2)
    return
def get_node_id():
    mesh.command("get_node_id",[])
    loop(2)
    return




# -------------------- main -------------------- 
#python client.py -p COM4 -n 24 -c 10
config = cfg.get_local_json()

cfg.configure_log(config["log"])

log.info("hci client started")

#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message)

parser = argparse.ArgumentParser()
parser.add_argument("-p","--port",default="COM4")
parser.add_argument("-n","--node",default=73)
parser.add_argument("-c","--channel",default=2)
parser.add_argument("-f","--function",default="x")
args = parser.parse_args()

node_id = args.node
chan = args.channel

mesh.start(config,mesh_on_broadcast)

set_channel(chan)

get_node_id()

loop_forever()
