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

'''It's important to provide the msg dictionnary here as it might be used in a multitude of ways
   by other modules
'''
def mesh_on_broadcast(msg):
    log.info("rf  > %s %s : %s"%(msg["src"],mesh.node_name(msg["src"]),mesh.inv_pid[int(msg["id"])]))
    if(config["mqtt"]["rf_2_mqtt"]):
        publishing = mesh.publish(msg)
        for topic,payload in publishing.items():
            clientMQTT.publish(topic,payload)
    return

''' the return mesntions if the logto the user is handled or if not
    the raw line will be logged
'''
def mesh_on_cmd_response(resp):
    global node_id
    if(resp["cmd"] == "get_node_id"):
        node_id = int(resp["node_id"])
        log.info("rf  > response node_id => : %d",node_id)
        return True
    return False

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
    print("cmd > set_channel:",chan)
    mesh.command("set_channel",[chan])
    loop(2)
    return
def get_channel():
    mesh.command("get_channel",[])
    loop(2)
    return
def get_node_id():
    print("cmd > get_node_id:",chan)
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
parser.add_argument("-c","--channel",default=2)
parser.add_argument("-f","--function",default="x")
args = parser.parse_args()

node_id = 0
#TODO this have a default that comes from the config
#so that the command line can only override the config if required
chan = int(args.channel)

mesh.start(config,mesh_on_broadcast,mesh_on_cmd_response)

set_channel(chan)

get_node_id()

loop_forever()
