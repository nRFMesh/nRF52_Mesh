import sys
import logging as log
from time import sleep,time
import json
from mqtt import mqtt_start
import cfg
from colour import Color

def interpolate(col1,col2,coeff):
    res = Color(rgb=(   (col1.get_red()   * (1-coeff)) + (col2.get_red()   * coeff),
                        (col1.get_green() * (1-coeff)) + (col2.get_green() * coeff),
                        (col1.get_blue()  * (1-coeff)) + (col2.get_blue()  * coeff)
                    )
                )
    return res

def esp_progress(col_back,col_front,percent):
    log.debug("test> set progress to %d percent",percent)
    nb_leds = 7
    led_match = [0] * nb_leds
    led_colors = [Color(rgb=(0,0,0))] * nb_leds
    pos = nb_leds * float(percent) / 100
    #log.debug("pos = %f",pos)
    j_payload = {}
    j_payload["leds"] = [0] * nb_leds * 3
    for i in range(nb_leds):
        pos_location = int(pos)
        if(i < pos_location ):
            coeff = 1
        elif(i > pos_location):
            coeff = 0
        else:
            coeff = float(pos - pos_location)
        
        led_colors[i] = interpolate(col_back,col_front,coeff)
        j_payload["leds"][3*i +0] = int(led_colors[i].get_red()*255)
        j_payload["leds"][3*i +1] = int(led_colors[i].get_green()*255)
        j_payload["leds"][3*i +2] = int(led_colors[i].get_blue()*255)

    topic = "esp/rgb led/list"
    clientMQTT.publish(topic,json.dumps(j_payload))
    return

def esp_status(topic,payload):
    if(topic == "percent"):
        percent = int(payload)
        log.debug("percent> set to %d percent",percent)
        col_back = Color(rgb=(0,0,0.02))
        col_front = Color(rgb=(0,0.2,0))
        esp_progress(col_back,col_front,percent)
    elif(topic == "bed"):
        percent = int(payload)
        log.debug("bed> set to %d percent",percent)
        col_back = Color(rgb=(0,0,0.02))
        col_front = Color(rgb=(0.2,0,0))
        esp_progress(col_back,col_front,percent)
    return


def bed_temperature(payload):
    temp = float(payload)
    if(temp < 17):
        ratio = 0
    elif(temp > 30):
        ratio = 1
    else:
        ratio = float((temp - 17.0) / (30.0-17.0))
    esp_status("bed",ratio*100)
    return

g_esp_heating_value = 0

def bed_heater_switch(payload):
    sensor = json.loads(payload)
    global g_esp_heating_value
    topic = "esp/bed heater/1h"
    request = None
    if("click" in sensor):
        if(sensor["click"] == "single"):
            request = g_esp_heating_value + 4
        elif(sensor["click"] == "double"):
            request = 0
    elif("action" in sensor):
        if(sensor["action"] == "hold"):
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

#catch the heat value in case the order comes from another client
def esp_bed_heater(topic_action,payload):
    if(topic_action == "heating"):
        global g_esp_heating_value
        g_esp_heating_value = int(payload)
        log.debug("esp_heater> %d",g_esp_heating_value)
    return


def mqtt_on_message(client, userdata, msg):
    log.info("mqtt>%s",msg.topic)
    topics = msg.topic.split('/')
    #"Nodes/98/temperature"
    if(len(topics) == 3):
        if(msg.topic == "Nodes/98/temperature"):
            bed_temperature(msg.payload)
        elif((topics[0] == "esp") and (topics[1] == "bed heater")):
            esp_bed_heater(topics[2],msg.payload)
    elif(len(topics) == 2):
        if(topics[0] == "status"):
            esp_status(topics[1],msg.payload)
        elif(topics[0] == "zigbee2mqtt"):
            name = topics[1]
            if(name == "bed heater"):
                bed_heater_switch(msg.payload)
    return


def loop_forever():
    while(True):
        sleep(0.1)
        if(config["mqtt"]["enable"]):
            clientMQTT.loop()
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)
#will start a separate thread for looping
clientMQTT = mqtt_start(config,mqtt_on_message,False)

loop_forever()
