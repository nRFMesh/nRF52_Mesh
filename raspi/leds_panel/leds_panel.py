import sys
import json
from mqtt import mqtt_start
import cfg
from colour import Color
from time import sleep

panel_name = "curvy"

def interpolate(col1,col2,coeff):
    res = Color(rgb=(   (col1.get_red()   * (1-coeff)) + (col2.get_red()   * coeff),
                        (col1.get_green() * (1-coeff)) + (col2.get_green() * coeff),
                        (col1.get_blue()  * (1-coeff)) + (col2.get_blue()  * coeff)
                    )
                )
    return res

def pixels_all(color):
    topic = "esp/"+panel_name+"/all"
    panel = {}
    panel["red"]    = int(color.get_red()*255)
    panel["green"]  = int(color.get_green()*255)
    panel["blue"]   = int(color.get_blue()*255)
    clientMQTT.publish(topic,json.dumps(panel))
    return

def pixels_one(index,color):
    topic = "esp/"+panel_name+"/one"
    panel = {}
    panel["index"]    = index
    panel["red"]    = int(color.get_red()*255)
    panel["green"]  = int(color.get_green()*255)
    panel["blue"]   = int(color.get_blue()*255)
    clientMQTT.publish(topic,json.dumps(panel))
    return

def pixels_list(col_list):
    topic = "esp/"+panel_name+"/list"
    panel = {}
    rgb_list = []
    for col in col_list:
        rgb_list.append(int(col.get_red()*255))
        rgb_list.append(int(col.get_green()*255))
        rgb_list.append(int(col.get_blue()*255))
    panel["leds"] = rgb_list
    clientMQTT.publish(topic,json.dumps(panel))
    return

def panel_off():
    topic = "esp/"+panel_name+"/panel"
    panel = {}
    panel["action"] = "off"
    clientMQTT.publish(topic,json.dumps(panel))
    return

def panel_flash(color, duration_ms):
    topic = "esp/"+panel_name+"/panel"
    panel = {}
    panel["action"] = "flash"
    panel["duration_ms"] = duration_ms
    panel["r"] = int(col.get_red()*255))
    panel["g"] = int(col.get_green()*255))
    panel["b"] = int(col.get_blue()*255))
    clientMQTT.publish(topic,json.dumps(panel))
    return

def test_range():
    for i in range(5):
        pixels_one(i,Color(rgb=(0, 0, 0.1)))
        sleep(0.2)
    return

def test_list():
    my_panel = []
    for i in range(4):
        my_panel.append(Color(rgb=(0, 0, 1/255)))
    pixels_list(my_panel)
    return

def pixels_off():
    pixels_all(Color(rgb=(0, 0, 0)))
    return

def pixels_brightness(val):
    pixels_all(Color(rgb=(val, val, val)))
    return

def pixels_blink():
    pixels_brightness(1)
    sleep(0.3)
    pixels_off()
    return

# -------------------- main -------------------- 
config = cfg.get_local_json()
#will start a separate thread for looping
clientMQTT = mqtt_start(config,None,False)

#pixels_all(Color(rgb=(0, 1/255, 0)))
#test_range()
#test_list()
#pixels_blink()
#pixels_off()
#panel_off()

panel_flash(Color(rgb=(0, 0, 0)))

