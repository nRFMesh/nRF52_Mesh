import sys
import logging as log
import argparse
from time import sleep,time
import json
from mqtt import mqtt_start
import socket
import mesh as mesh
import cfg
import threading

g_lights_upstairs_on = False
g_esp_heating_value = 0


def off_callback():
    size = 0x07
    control = 0x72
    pid = 0x09
    heat = 0
    duration = 0
    src = config["heat"]["node_src"]
    dst = config["heat"]["node_dst"]
    mesh.send([ size,control,mesh.pid["heat"],src,dst,int(heat),int(duration)])
    if(config["mqtt"]["publish"]):
        topic = "Nodes/"+str(config["heat"]["node_src"])+"/heat"
        payload = int(heat)
        clientMQTT.publish(topic,payload)
    return

def serial_send_heat_duration(heat,duration_mn):
    size = 0x07
    control = 0x72
    pid = 0x09
    src = config["heat"]["node_src"]
    dst = config["heat"]["node_dst"]
    mesh.send([ size,control,mesh.pid["heat"],src,dst,int(heat),int(duration_mn)])
    if(config["mqtt"]["publish"]):
        topic = "Nodes/"+str(config["heat"]["node_src"])+"/heat"
        payload = int(heat)
        clientMQTT.publish(topic,payload)
    threading.Timer(duration_mn*60, off_callback).start()
    log.info("send heat %s during %s minutes",heat, duration_mn)

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

def mqtt_handle_heat_request(topic_action,payload):
    if(topic_action == "json"):
        try:
            params = json.loads(payload)
            if("heat" in params) and ("time_mn" in params):
                serial_send_heat_duration(params["heat"],params["time_mn"])
        except json.decoder.JSONDecodeError:
            log.error("mqtt_req > json.decoder.JSONDecodeError parsing payload: %s",msg.payload)
    elif(topic_action == "1h"):
        serial_send_heat_duration(int(payload),60)
    elif(topic_action == "20mn"):
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

def mqtt_handle_switch_lights_upstairs(payload):
    jval = json.loads(payload)
    if("click" in jval and jval["click"] == "single"):
        global g_lights_upstairs_on
        if(g_lights_upstairs_on):
            mqtt_handle_dimmer_request(["Retro Light Upstairs","all"],0)
            g_lights_upstairs_on = False
            log.info("lights_upstairs> Off")
        else:
            mqtt_handle_dimmer_request(["Retro Light Upstairs","all"],7000)
            g_lights_upstairs_on = True
            log.info("lights_upstairs> On")
    if("action" in jval and jval["action"] == "hold"):
            mqtt_handle_dimmer_request(["Retro Light Upstairs","all"],2000)
            g_lights_upstairs_on = True
            log.info("lights_upstairs> dimmed")
    if("click" in jval and jval["click"] == "double"):
            mqtt_handle_dimmer_request(["Retro Light Upstairs","all"],7000)
            g_lights_upstairs_on = True
            log.info("lights_upstairs> double bright up")
    else:
        log.debug("lights_upstairs>no click")
    return

def mqtt_handle_switch_bed_heater(payload):
    jval = json.loads(payload)
    global g_esp_heating_value
    topic = "esp/bed heater/1h"
    request = None
    if("click" in jval):
        if(jval["click"] == "single"):
            request = g_esp_heating_value + 4
        elif(jval["click"] == "double"):
            request = 0
    elif("action" in jval):
        if(jval["action"] == "hold"):
            request = g_esp_heating_value - 4
            if(request < 0):
                request = 0
    else:
        log.debug("bed heater>no click no action")
    if(request != None):
        log.info("bed heater>previous %d , request to %d",g_esp_heating_value, request)
        #update temporarily with best guess, for correct reaction without waiting the real feedback
        g_esp_heating_value = request
        payload = str(request)
        clientMQTT.publish(topic,payload)
    return

def mqtt_handle_esp_bed_heater(topic_action,payload):
    if(topic_action == "heating"):
        global g_esp_heating_value
        g_esp_heating_value = int(payload)
        log.debug("esp_heater> %d",g_esp_heating_value)
    return


def mqtt_on_message(client, userdata, msg):
    log.debug("mqtt>%s",msg.topic)
    topics = msg.topic.split('/')
    if(len(topics) == 2) and (len(msg.payload) != 0):
        #"Bed Heater" is the STM32 dongle to serial topic
        if(topics[0] == "Bed Heater"):
            mqtt_handle_heat_request(topics[1],msg.payload)
        elif(topics[0] == "Retro Light Upstairs"):
            mqtt_handle_dimmer_request(topics,msg.payload)
        elif(topics[0] == "zig"):
            name = topics[1]
            if(name == "retrolight1") or (name == "retrolight2"):
                mqtt_handle_switch_lights_upstairs(msg.payload)
            elif(name == "bed heater"):
                mqtt_handle_switch_bed_heater(msg.payload)
    elif(len(topics) == 3):
        if((topics[0] == "esp") and (topics[1] == "bed heater")):
            mqtt_handle_esp_bed_heater(topics[2],msg.payload)
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

