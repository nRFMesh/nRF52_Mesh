import paho.mqtt.client as mqtt
import datetime
import logging as log
import cfg
from time import sleep,time
import json
import socket

conf = {}

# -------------------- mqtt events -------------------- 
def on_connect(lclient, userdata, flags, rc):
    global conf
    log.info("mqtt> connected with result code "+str(rc))
    if(conf["mqtt"]["subscribe"]):
        for sub in userdata["mqtt"]["subscriptions"]:
            log.info("mqtt> Subscription to %s",sub)
            lclient.subscribe(sub)
    else:
        log.info("mqtt> Subscriptions not enabled")
    if(conf["mqtt"]["publish"]):
        log.info("mqtt> Publishing enabled")
    else:
        log.info("mqtt> Publishing not enabled")
            
    

def ruler_loop_forever():
    while(True):
        sleep(10)
    return


def mqtt_start(config,mqtt_on_message,start_looping):
    def mqtt_connect_retries(client):
        connected = False
        while(not connected):
            try:
                client.connect(config["mqtt"]["host"], config["mqtt"]["port"], config["mqtt"]["keepalive"])
                connected = True
                log.info(  "mqtt> connected to "+config["mqtt"]["host"]+":"+str(config["mqtt"]["port"])+" with id: "+ cid )
            except socket.error:
                log.error("socket.error will try a reconnection in 10 s")
                sleep(10)
        return

    global conf
    conf = config
    clientMQTT = None
    if(config["mqtt"]["publish"] or config["mqtt"]["subscribe"]):
        config["mqtt"]["enable"] = True
        cid = config["mqtt"]["client_id"] +"_"+socket.gethostname()
        clientMQTT = mqtt.Client(client_id=cid,userdata=config)
        clientMQTT.on_connect = on_connect
        clientMQTT.on_message = mqtt_on_message
        mqtt_connect_retries(clientMQTT)
        if(start_looping):
            #the loop could be called in the run main loop()
            clientMQTT.loop_start()
    else:
       config["mqtt"]["enable"] = False
    return clientMQTT
