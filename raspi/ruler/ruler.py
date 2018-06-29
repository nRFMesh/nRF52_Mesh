import paho.mqtt.client as mqtt
import datetime
import logging as log
import cfg
from time import sleep,time
import json
import rules
import socket

# -------------------- mqtt events -------------------- 
def on_connect(lclient, userdata, flags, rc):
    log.info("mqtt connected with result code "+str(rc))
    for rule_name,rule in config["rules"].iteritems():
        log.info("Subscription for rule:%s %s -> %s",rule_name,rule["input"],rule["output"])
        lclient.subscribe(rule["input"])

def on_message(client, userdata, msg):
    topic_parts = msg.topic.split('/')
    for rule_name,rule in config["rules"].iteritems():
        if msg.topic == rule["input"]:
            if(rule["enable"]):
                #call the Fuction with the same name as the Rule 
                payload = getattr(rules,rule_name)(msg.payload)
                if(payload):
                    clientMQTT.publish(rule["output"],payload)


def ruler_loop_forever():
    while(True):
        sleep(10)
    return


def mqtt_start():
    def mqtt_connect_retries(client):
        connected = False
        while(not connected):
            try:
                client.connect(config["mqtt"]["host"], config["mqtt"]["port"], config["mqtt"]["keepalive"])
                connected = True
                log.info(  "mqtt connected to "+config["mqtt"]["host"]+":"+str(config["mqtt"]["port"])+" with id: "+ cid )
            except socket.error:
                log.error("socket.error will try a reconnection in 10 s")
            sleep(10)
        return
    cid = config["mqtt"]["client_id"] +"_"+socket.gethostname()
    client = mqtt.Client(client_id=cid)
    clientMQTT = mqtt.Client()
    clientMQTT.on_connect = on_connect
    clientMQTT.on_message = on_message
    mqtt_connect_retries(clientMQTT)
    clientMQTT.loop_start()
    return clientMQTT

# -------------------- main -------------------- 
config = cfg.get_local_json("config.json")

cfg.configure_log(config["log"])

log.info("ruler started @ :"+str(datetime.datetime.utcnow()))

#will start a separate thread for looping
clientMQTT = mqtt_start()

#loop forever
ruler_loop_forever()
