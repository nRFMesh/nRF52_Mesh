import sys
import logging as log
import argparse
import cfg
from time import sleep,time
import json
from mqtt import mqtt_start
import socket
import mesh as mesh

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

def mqtt_on_message(client, userdata, msg):
    topics = msg.topic.split('/')
    cmd = topics[2]
    params = []
    if(len(msg.payload) != 0):
        try:
            params = json.loads(msg.payload)
        except json.decoder.JSONDecodeError:
            log.error("mqtt_req > json.decoder.JSONDecodeError parsing payload: %s",msg.payload)
    if(topics[1] == "request"):
        if(topics[0] == "cmd"):
            if(execute_command(cmd,params)):
                log.info("mqtt> Command Request : %s",cmd)
            else:
                log.error("mqtt> Error: Command Request not executed : %s",cmd)
        elif(topics[0] == "remote_cmd"):
            if(remote_execute_command(cmd,params)):
                log.info("mqtt> Remote Command Request : %s",cmd)
            else:
                log.error("mqtt> Error: Remote Command Request not executed : %s",cmd)
    elif(topics[0] == "Nodes"):
        action = topics[2]
        if(action in config["mqtt"]["actions"]):
            log.info("mqtt> Action %s",action)
            mesh_do_action(cmd,topics[1],params)
    return

'''It's important to provide the msg dictionnary here as it might be used in a multitude of ways
   by other modules
'''
def mesh_on_broadcast(msg):
    log.info("rf  > %s %s : %s"%(msg["src"],mesh.node_name(msg["src"]),mesh.inv_pid[int(msg["pid"])]))
    if(config["mqtt"]["rf_2_mqtt"]):
        publishing = mesh.publish(msg)
        for topic,payload in publishing.items():
            clientMQTT.publish(topic,payload)
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
        clientMQTT.publish(topic,payload)
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
    clientMQTT.publish(topic,json.dumps(resp))
    if(resp["cmd"] == "get_node_id"):
        this_node_id = int(resp["node_id"])
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

def test1():
    remote_set_channel(74,10)
    set_channel(10)
    ping(74)
    loop_forever()
    return
# -------------------- main -------------------- 
#python client.py -p COM4 -n 24 -c 10
config = cfg.get_local_json()

cfg.configure_log(config["log"])

log.info("hci client started")

parser = argparse.ArgumentParser()
parser.add_argument("-c","--channel",default=10)
parser.add_argument("-f","--function",default="x")
args = parser.parse_args()

this_node_id = 0
#TODO this have a default that comes from the config
#so that the command line can only override the config if required
chan = int(args.channel)

#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message)

mesh.start(config,mesh_on_broadcast,mesh_on_message,mesh_on_cmd_response)

set_channel(chan)

get_node_id()

if(args.function == 'l'):
    loop_forever()

loop_forever()

#ping(75)
#test1()
