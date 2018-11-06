#https://github.com/studioimaginaire/phue


#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from phue import Bridge
#just to get host name
import socket 
from time import sleep
from math import ceil
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start
import threading

def aqara_cube(name,payload):
    if(name == "Aqara Cube 1"):
        log.debug("action => Aqara Cube 1")
        jval = json.loads(payload)
        if("event" in jval):
            if(jval["event"] == "flip"):
                lights["Stairs Up Left"].on = not lights["Stairs Up Left"].on
    return

def aqara_button(name):
    if(lights["Bed Leds Cupboard"].on):
        lights["Bed Leds Cupboard"].on = False
        log.debug("aqara_button> set light off")
    else:
        #command so that it does not go to previous level before adjusting the brightness
        b.set_light("Bed Leds Cupboard", {'on' : True, 'bri' : 1, 'hue':8101, 'sat':194})
        log.debug("aqara_button> set light to 1")
    return

def aqara_button_sequence(name):
    if(lights["Bed Leds Cupboard"].on):
        if(lights["Bed Leds Cupboard"].brightness == 11):
            lights["Bed Leds Cupboard"].brightness = 21
            log.debug("aqara_button> set light to 21")
        elif(lights["Bed Leds Cupboard"].brightness == 1):
            lights["Bed Leds Cupboard"].brightness = 11
            log.debug("aqara_button> set light to 11")
        else:
            lights["Bed Leds Cupboard"].on = False
            log.debug("aqara_button> set light off")
    else:
        #command so that it does not go to previous level before adjusting the brightness
        b.set_light("Bed Leds Cupboard", {'on' : True, 'bri' : 1, 'hue':8101, 'sat':194})
        log.debug("aqara_button> set light to 1")
    return

def entrance_light(payload):
    jval = json.loads(payload)
    if("click" in jval and jval["click"] == "single"):
        if(lights["Entrance White 1"].on):
            lights["Entrance White 1"].on = False
            lights["Entrance White 2"].on = False
            log.debug("entrance_light> off")
        else:
            #command so that it does not go to previous level before adjusting the brightness
            b.set_light("Entrance White 1", {'on' : True, 'bri' : 255})
            b.set_light("Entrance White 2", {'on' : True, 'bri' : 255})
            log.debug("entrance_light> on")
    else:
        log.debug("entrance_light>no click")
    return


def double_steps_brightness(light,short_time_brightness,final):
    b.set_light(light, {'on' : True, 'bri' : short_time_brightness, 'hue':8101, 'sat':194, 'transitiontime' : 30})
    log.debug("bedroom_sunrise> fast 3 sec to %d",short_time_brightness)
    #the rest souldshould catch in 10 min average from 0 to max
    if(short_time_brightness < 255):
        brightness_left = int(255 - short_time_brightness)
        time_left = int(brightness_left * 10 * 60 * 10 / 250)
        b.set_light(light, {'on' : True, 'bri' : final, 'hue':8101, 'sat':194, 'transitiontime' : time_left})
        log.debug("bedroom_sunrise> slow till %d in %d sec",final,time_left/10)
    return

def bedroom_sunrise(payload):
    jval = json.loads(payload)
    if("click" in jval and jval["click"] == "single"):
        if(not lights["Bed Leds Cupboard"].on):
            current_brightness = 0
            short_time_brightness = 1
        else:
            current_brightness = lights["Bed Leds Cupboard"].brightness
            if(current_brightness < 215):
                short_time_brightness = current_brightness + 40
            else:
                short_time_brightness = 255
        log.debug("beroom_sunrise>current brightness %d",current_brightness)
        double_steps_brightness("Bed Leds Cupboard",short_time_brightness,255)
    elif("action" in jval and jval["action"] == "hold"):
        if(not lights["Bed Leds Cupboard"].on):
            log.warn("beroom_sunrise>already off nothing to do")
        else:
            current_brightness = lights["Bed Leds Cupboard"].brightness
            if(current_brightness > 41):
                short_time_brightness = current_brightness - 40
            elif(current_brightness > 3):
                short_time_brightness = 1
            else:
                lights["Bed Leds Cupboard"].on = False
        log.debug("beroom_sunrise>current brightness %d",current_brightness)
        double_steps_brightness("Bed Leds Cupboard",short_time_brightness,0)
    else:
        log.debug("bedroom_sunrise>no click")
    return


def aqara_switch(name):
    if(lights["Living 1 Table E27"].on):
        lights["Living 1 Table E27"].on = False
        lights["Living 2 Table E27"].on = False
        lights["Entrance White 1"].on = False
        lights["Entrance White 2"].on = False
        b.set_light("LivRoom Spot 5 Innr", {'on' : True, 'bri' : 125})
        log.debug("aqara_switch> Dining room and Entrence lights off")
    else:
        #command so that it does not go to previous level before adjusting the brightness
        b.set_light("Living 1 Table E27", {'on' : True, 'bri' : 255})
        b.set_light("Living 2 Table E27", {'on' : True, 'bri' : 255})
        log.debug("aqara_switch> switch on Dining room")
    return

def stairs_off_callback():
    lights["Stairs Up Left"].on = False
    lights["Stairs Down Right"].on = False
    log.debug("Stairs - off_callback")
    return

g_stairs_up_light = 0.0
g_stairs_down_light = 0.0

def stairs_presence(name,payload):
    global g_stairs_up_light
    global g_stairs_down_light
    #log.debug("stairs_presence : %s"%payload)
    jval = json.loads(payload)
    if("light" in jval):
        log.debug("light => %f"%jval["light"])
        if(name == "MotionLight 1"):
            g_stairs_up_light = float(jval["light"])
            log.debug("light => MotionLight Up : %f"%g_stairs_up_light)
        if(name == "MotionLightHue"):
            g_stairs_down_light = float(jval["light"])
            log.debug("light => MotionLightHue: %f"%g_stairs_down_light)
    if("presence" in jval):
        log.debug("presence => %d"%jval["presence"])
        if(jval["presence"]):
            if(g_stairs_up_light < 2):
                brightness = 254
            elif(g_stairs_up_light < 12):
                brightness = 128
            else:
                brightness = 10
            if(name == "MotionLight 1"):
                log.debug(f"presence => MotionLight Up - brightness:{brightness}")
                b.set_light("Stairs Up Left", {'transitiontime' : 30, 'on' : True, 'bri' : brightness})
                b.set_light("Stairs Down Right", {'transitiontime' : 10, 'on' : True, 'bri' : int(brightness/2)})
                threading.Timer(60, stairs_off_callback).start()
            if(name == "MotionLightHue"):
                log.debug(f"presence => MotionLight Down - brightness:{brightness}")
                b.set_light("Stairs Down Right", {'transitiontime' : 10, 'on' : True, 'bri' : brightness})
                b.set_light("Stairs Up Left", {'transitiontime' : 30, 'on' : True, 'bri' : int(brightness/2)})
                threading.Timer(60, stairs_off_callback).start()
    return


def mqtt_on_message(client, userdata, msg):
    topic_parts = msg.topic.split('/')
    if(len(topic_parts) == 2 and topic_parts[0] == "zigbee2mqtt"):
        name = topic_parts[1]
        if(name == "entrance light"):
            entrance_light(msg.payload)
        elif(name == "sunrise"):
            bedroom_sunrise(msg.payload)
    elif(len(topic_parts) == 3):
        modelid = topic_parts[1]
        name = topic_parts[2]
        log.debug(f"mqtt> name = {name} ; modelid = {modelid}")
        if(modelid == "lumi.sensor_cube.aqgl01"):
            log.debug("modelid : aqara_cube()")
            aqara_cube(name,msg.payload)
        elif(name == "lumi.remote.b1acn01 16"):
            log.debug("name : aqara button 16")
            aqara_button(name)
        elif(name == "LargeSingleSwitch"):
            log.debug("name : LargeSingleSwitch")
            aqara_switch(name)
        elif((modelid == "lumi.sensor_motion.aq2") or (modelid == "SML001") ):
            log.debug("modelid : stairs_presence()")
            stairs_presence(name,msg.payload)
        else:
            log.warning("WARNING : unused modelid")
    else:
        log.error("topic: "+msg.topic + "size not matching")
        

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

# -------------------- Philips Hue Client -------------------- 
log.info("Check Bridge Presence")

if(cfg.ping(config["bridges"]["LivingRoom"])):
    b = Bridge(config["bridges"]["LivingRoom"])
    log.info("Bridge Connection")
    b.connect()
    log.info("Light Objects retrieval")
    lights = b.get_light_objects('name')
    print("_________________________")
    print(lights["Bed Leds Cupboard"])
    print("_________________________")

    log.info("Hue Lights available :")
    for name, light in lights.items():
        log.info(name)
    
else:
    log.info("Bridge ip not responding")


# -------------------- Mqtt Client -------------------- 
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,True)

while(True):
    sleep(0.2)
    #The MQTT keeps looping on a thead
    #All there is to do here is not to exit
