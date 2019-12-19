#https://github.com/studioimaginaire/phue


#https://pypi.python.org/pypi/paho-mqtt/1.1
import paho.mqtt.client as mqtt
import json
from phue import Bridge
#just to get host name
import socket 
from time import sleep
import time
from math import ceil
import logging as log
import sys,os
import cfg
from mqtt import mqtt_start
import threading

def aqara_cube(payload):
    log.debug("action => Aqara Cube 1")
    sensor = json.loads(payload)
    if("action" in sensor):
        if(sensor["action"] == "flip90"):
            lights["Stairs Up Left"].on = not lights["Stairs Up Left"].on
    return

def debounce(in_time):
    current_time = time.time()
    delta = current_time - in_time
    return (delta > 2),current_time

debounce_1_prev = 0
def debounce_1():
    global debounce_1_prev
    res,debounce_1_prev = debounce(debounce_1_prev)
    return res

debounce_2_prev = 0
def debounce_2():
    global debounce_2_prev
    res,debounce_2_prev = debounce(debounce_2_prev)
    return res

debounce_3_prev = 0
def debounce_3():
    global debounce_3_prev
    res,debounce_3_prev = debounce(debounce_3_prev)
    return res

def bed_light_button(payload):
    if(debounce_1()):
        log.debug("bed_light_button> taken")
        sensor = json.loads(payload)
        if("click" in sensor and sensor["click"] == "single"):
            if(lights["Bed Malm"].on):
                lights["Bed N"].on = False
                lights["Bed Malm"].on = False
                lights["Bed W"].on = False
                log.debug("bed_light_button> set light off")
            else:
                #switch on and brightness command together so that it does not go to previous level before adjusting the brightness
                b.set_light("Bed Malm", {'on' : True, 'bri' : 254})
                b.set_light("Bed N", {'on' : True, 'bri' : 254})
                b.set_light("Bed W", {'on' : True, 'bri' : 254})
                log.debug("bed_light_button> set light to MAX")
        elif("action" in sensor and sensor["action"] == "hold"):
            b.set_light("Bed Malm", {'on' : True, 'bri' : 1})
            lights["Bed N"].on = False
            lights["Bed W"].on = False
            log.debug("bed_light_button> set light to min")
    #else:
        #log.debug("bed_light_button> debounced")
    return

def bathroom_light_button(payload):
    if(debounce_2()):
        log.debug("bathroom light> taken")
        sensor = json.loads(payload)
        if("click" in sensor and sensor["click"] == "single"):
            if(lights["Bathroom main"].on):
                lights["Bathroom main"].on = False
                log.debug("bathroom light> set light off")
            else:
                #switch on and brightness command together so that it does not go to previous level before adjusting the brightness
                b.set_light("Bathroom main", {'on' : True, 'bri' : 254})
                log.debug("bathroom light> set light to MAX")
        elif("action" in sensor and sensor["action"] == "hold"):
            b.set_light("Bathroom main", {'on' : True, 'bri' : 1})
            log.debug("bathroom light> set light to min")
    return

def livroom_light_button(payload):
    if(debounce_3()):
        log.debug("living room light> taken")
        sensor = json.loads(payload)
        if("click" in sensor and sensor["click"] == "single"):
            if(lights["LivingTop5"].on):
                lights["LivingTop1"].on = False
                lights["LivingTop2"].on = False
                lights["LivingTop3"].on = False
                lights["LivingTop4"].on = False
                lights["LivingTop5"].on = False
                log.debug("living room light> set light off")
            else:
                #switch on and brightness command together so that it does not go to previous level before adjusting the brightness
                b.set_light("LivingTop1", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop2", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop3", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop4", {'on' : True, 'bri' : 254})
                b.set_light("LivingTop5", {'on' : True, 'bri' : 254})
                log.debug("living room light> set light to MAX")
        elif("action" in sensor and sensor["action"] == "hold"):
            b.set_light("LivingTop1", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop2", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop3", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop4", {'on' : True, 'bri' : 1})
            b.set_light("LivingTop5", {'on' : True, 'bri' : 1})
            log.debug("living room light> set light to min")
    return



def office_switch(payload):
    switch = json.loads(payload)
    if("click" in switch and switch["click"] == "single"):
        if(lights["Office main"].on):
            lights["Office main"].on = False
            log.debug("office_light> off")
        else:
            #command so that it does not go to previous level before adjusting the brightness
            b.set_light("Office main", {'on' : True, 'bri' : 255})
            log.debug("office_light> on")
    elif("action" in switch and switch["action"] == "hold"):
            b.set_light("Office main", {'on' : True, 'bri' : 1})
            log.debug("office_light> low")
    #else:
    #    log.debug("office_light>no click")
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
    elif("contact" in jval and jval["contact"] == False):
        #TODO check brightness here - and diff between coming or going away
        log.debug("entrance_door>open")
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
    elif("click" in jval and jval["click"] == "double"):
        if(not lights["Bed Leds Cupboard"].on):
            b.set_light(light, {'on' : True, 'bri' : 255, 'hue':8101, 'sat':194})
        else:
            lights["Bed Leds Cupboard"].on = False
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

isLightOn = False

def dining_switch(payload):
    sensor = json.loads(payload)
    if("click" in sensor and sensor["click"] == "single"):
        if(lights["Living 1 Table E27"].on):
            lights["Living 1 Table E27"].on = False
            lights["Living 2 Table E27"].on = False
            log.debug("dining_switch> Dining room and Entrence lights off")
        else:
            #command so that it does not go to previous level before adjusting the brightness
            b.set_light("Living 1 Table E27", {'on' : True, 'bri' : 255})
            b.set_light("Living 2 Table E27", {'on' : True, 'bri' : 255})
            log.debug("dining_switch> switch on Dining room")
    return

def stairs_off_callback():
    lights["Stairs Up Left"].on = False
    lights["Stairs Down Right"].on = False
    log.debug("Stairs - off_callback")
    return

g_stairs_up_light = 0.0
g_stairs_down_light = 0.0

def stairs_up_move(payload):
    global g_stairs_up_light
    global g_stairs_down_light
    #log.debug("stairs_presence : %s"%payload)
    sensor = json.loads(payload)
    if("illuminance" in sensor):
        log.debug("light => %f"%sensor["illuminance"])
        g_stairs_up_light = float(sensor["illuminance"])
        log.debug("light => stairs up : %f"%g_stairs_up_light)
    if("occupancy" in sensor):
        log.debug("presence => %d"%sensor["occupancy"])
        if(sensor["occupancy"]):
            if(g_stairs_up_light < 2):
                brightness = 254
            elif(g_stairs_up_light < 12):
                brightness = 128
            else:
                brightness = 10
            log.debug(f"presence => MotionLight Up - brightness:{brightness}")
            b.set_light("Stairs Up Left", {'transitiontime' : 30, 'on' : True, 'bri' : brightness})
            b.set_light("Stairs Down Right", {'transitiontime' : 10, 'on' : True, 'bri' : int(brightness)})
            threading.Timer(60, stairs_off_callback).start()
    return

def stairs_down_move(payload):
    global g_stairs_up_light
    global g_stairs_down_light
    #log.debug("stairs_presence : %s"%payload)
    sensor = json.loads(payload)
    if("illuminance" in sensor):
        log.debug("light => %f"%sensor["illuminance"])
        g_stairs_down_light = float(sensor["illuminance"])
        log.debug("light => MotionLightHue: %f"%g_stairs_down_light)
    if("occupancy" in sensor):
        log.debug("presence => %d"%sensor["occupancy"])
        if(sensor["occupancy"]):
            if(g_stairs_up_light < 2):
                brightness = 254
            elif(g_stairs_up_light < 12):
                brightness = 128
            else:
                brightness = 10
            log.debug(f"presence => MotionLight Down - brightness:{brightness}")
            b.set_light("Stairs Down Right", {'transitiontime' : 10, 'on' : True, 'bri' : brightness})
            b.set_light("Stairs Up Left", {'transitiontime' : 30, 'on' : True, 'bri' : int(brightness)})
            threading.Timer(60, stairs_off_callback).start()
    return

def mqtt_on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split('/')
        if(len(topic_parts) == 2):
            name = topic_parts[1]
            if(name == "bed light button") or (name == "bed nic button"):
                bed_light_button(msg.payload)
            elif(name == "office switch"):
                office_switch(msg.payload)
            elif(name == "tree button"):
                bathroom_light_button(msg.payload)
            elif(name == "liv light 1 button"):
                livroom_light_button(msg.payload)
        else:
            log.error("topic: "+msg.topic + "size not matching")
    except Exception as e:
        log.error("mqtt_on_message> Exception :%s"%e)
    return

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
