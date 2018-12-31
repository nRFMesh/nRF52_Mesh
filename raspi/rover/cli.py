#%%
import sys
import logging as log
import argparse
from time import sleep,time
import json
import socket
import os

# attempt for jupyter
#nb_dir = os.path.split(os.getcwd())[0]
#if nb_dir not in sys.path:
#    sys.path.append(nb_dir)
#from raspi.rf_uart.mqtt import mqtt_start
#import raspi.rf_uart.mesh as mesh
#import raspi.rf_uart.cfg as cfg

import mesh as mesh
import cfg
from mqtt import mqtt_start

def mesh_do_action(cmd,remote,params):
    control = 0x71
    try:
        if(cmd == "dimmer"):
            #TODO
            log.info("action> dimmer TODO")
        elif(cmd == "ping"):
            mesh.send([ control,mesh.pid["ping"],this_node_id,int(remote)])
    except KeyError:
        log.error("mqtt_remote_req > KeyError Exception for %s",cmd)
    except ValueError:
        log.error("mqtt_remote_req > ValueError Exception for %s , '%s'",cmd, remote)
    return

def remote_execute_command(cmd,params):
    control = 0x21
    try:
        if(cmd == "set_channel"):
            mesh.send([ control,mesh.pid["exec_cmd"],this_node_id,int(params["remote"]),
                        mesh.exec_cmd[cmd],int(params["channel"])]
                    )
        elif(cmd == "get_channel"):
            mesh.send([ control,mesh.pid["exec_cmd"],this_node_id,int(params["remote"]),
                        mesh.exec_cmd[cmd]]
            )
        else:
            return False
    except (KeyError,TypeError):
        log.error("mqtt_remote_req > Error Exception for %s",cmd)
    return True

def execute_command(cmd,params):
    try:
        if(cmd == "set_node_id"):
            mesh.command(cmd,[int(params["node_id"])])
        elif(cmd == "get_node_id"):
            mesh.command(cmd)
        elif(cmd == "set_channel"):
            mesh.command(cmd,[int(params["channel"])])
        elif(cmd == "get_channel"):
            mesh.command(cmd)
        elif(cmd == "set_tx_power"):
            mesh.command(cmd,[int(params["tx_power"])])
        elif(cmd == "get_tx_power"):
            mesh.command(cmd)
        else:
            return False
    except KeyError:
        log.error("mqtt_req > KeyError Exception for %s",cmd)
    return True

'''It's important to provide the msg dictionnary here as it might be used in a multitude of ways
   by other modules
'''
def mesh_on_broadcast(msg):
    try:
        log_text = f'rf > src:{msg["src"]} - {mesh.node_name(msg["src"])} : pid={mesh.inv_pid[int(msg["pid"])]}'
        log.debug(log_text)
        if(config["mqtt"]["publish"]):
            publishing = mesh.publish(msg)
            for topic,payload in publishing.items():
                clientMQTT.publish(topic,payload)
    except KeyError :
        log.error(f"no pid,msg in {json.dumps(msg)}")
    return

def node_log(msg):
    log_text = f'log >{json.dumps(msg)}'
    log.debug(log_text)
    if(config["mqtt"]["publish"]):
        topic = "jNodes/"+msg["src"]+"/log"
        clientMQTT.publish(topic,msg)
    return

def mesh_on_message(msg):
    #ack explanation not required
    if("ack" in msg):
        log.info(   "ack > %s %s -> %s",
                    mesh.inv_pid[int(msg["pid"])],
                    msg["src"],
                    msg["dest"]
                    )
        topic = "Nodes/"+msg["src"]+"/ack"
        payload = 1
    return
''' the return mesntions if the logto the user is handled or if not
    the raw line will be logged
'''
def mesh_on_cmd_response(resp,is_remote):
    global this_node_id
    if(is_remote):
        topic = "remote_cmd/response/"+resp["cmd"]
    else:
        topic = "cmd/response/"+resp["cmd"]
    print(f"cmd_resp> {topic} : {json.dumps(resp)}")
    if(resp["cmd"] == "get_node_id"):
        this_node_id = int(resp["node_id"])
    return

def mqtt_on_message(client, userdata, msg):
    log.error("mqtt> Unexpected topic %s",msg.topic)
    return

def loop(nb):
    while(nb > 0):
        #sleep(0.05)
        mesh.run()
        nb = nb - 1
    return

def remote_set_channel(remote,chan):
    log.debug("remote_set_channel(nodeid %d @ chan %d)",remote,chan)
    control = 0x21
    mesh.send([control,mesh.pid["exec_cmd"],this_node_id,remote,mesh.exec_cmd["set_channel"],remote,chan])
    loop(2)
    return

def set_channel(chan):
    log.debug("cmd > set_channel: %d",chan)
    mesh.command("set_channel",[chan])
    loop(2)
    return
def get_channel():
    log.debug("cmd > get_channel()")
    mesh.command("get_channel",[])
    loop(2)
    return
def get_node_id():
    log.debug("cmd > get_node_id()")
    mesh.command("get_node_id",[])
    loop(2)
    return

def ping(target_node):
    log.debug("msg > ping %d -> %d ",this_node_id,target_node)
    control = 0x70
    mesh.send([control,mesh.pid["ping"],this_node_id,target_node])
    loop(2)
    return

def bldc(target_node,alpha):
    log.debug(f"msg > bldc from {this_node_id} -> {target_node} set alpha = {alpha}")
    control = 0x70
    mesh.send([control,mesh.pid["bldc"],this_node_id,target_node,alpha])
    #loop(2)
    return


def test1():
    remote_set_channel(74,2)
    set_channel(2)
    ping(74)
    loop_forever()
    return
# -------------------- main -------------------- 
#python client.py -p COM4 -n 24 -c 10
config = cfg.configure_log(__file__)

parser = argparse.ArgumentParser()
parser.add_argument("-c","--channel",default=10)
parser.add_argument("-f","--function",default="x")
args = parser.parse_args()

this_node_id = 0
#TODO this have a default that comes from the config
#so that the command line can only override the config if required
chan = int(args.channel)

clientMQTT = mqtt_start(config,mqtt_on_message)

mesh.start(config,mesh_on_broadcast,mesh_on_message,mesh_on_cmd_response,node_log)

set_channel(chan)

get_node_id()

#%%
if(args.function == 'l'):
    loop(1000000)


while(True):
    mesh.run()

#bldc(75,37)
#loop(200)

#for i in range(5000):
#    bldc(75,i%256)
#    sleep(0.002)
