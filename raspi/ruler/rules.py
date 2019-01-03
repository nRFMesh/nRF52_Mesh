import logging as log
from time import time
import json
from functools import reduce
from vectors import Vector
import math

def static_vars(**kwargs):
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate

def safe_angle(vect,ref):
    result = 0
    try:
        result = vect.angle(ref)
    except ValueError:
        log.error("====>Handled Exception for ValueError")
        print(vect)
    return result
    
def Upstairs_Heat(input):
    v_ref_abs = Vector(math.radians(3),math.radians(81),math.radians(9))
    v_max_angle = 306
    a = json.loads(input)
    v_acc   = Vector(math.radians(a["angle_x"]),math.radians(a["angle_y"]),math.radians(a["angle_z"]))
    angle = safe_angle(v_acc,v_ref_abs)
    if(float(a["angle_x"]) < -5):
        log.debug("Upstairs_Heat>Other side a = %f"%angle)
        angle = 360 - angle
    if(type(angle) == float):
        #angle = v_acc.angle(v_closed)
        log.debug("Upstairs_Heat> =>output(%f)"%(angle))
    else:
        log.error("Upstairs_Heat>not float")
    message = {"heating" : int(100 * angle / v_max_angle)}
    return json.dumps(message)

def Bedroom_Heat_1(input):
    v_ref_abs = Vector(math.radians(8),math.radians(-76),math.radians(11))
    v_max_angle = 306
    a = json.loads(input)
    v_acc   = Vector(math.radians(a["angle_x"]),math.radians(a["angle_y"]),math.radians(a["angle_z"]))
    angle = safe_angle(v_acc,v_ref_abs)
    if(float(a["angle_x"]) < -5):
        log.debug("Bedroom_Heat_1>Other side a = %f"%angle)
        angle = 360 - angle
    if(type(angle) == float):
        #angle = v_acc.angle(v_closed)
        log.debug("Bedroom_Heat_1> =>output(%f)"%(angle))
    else:
        log.error("Bedroom_Heat_1>not float")
    message = {"heating" : int(100 * angle / v_max_angle)}
    return json.dumps(message)


@static_vars(isLightOn=False)
def Kitchen_Move(input):
    sensor = json.loads(input)
    command = None
    if(sensor["occupancy"]):
        if(sensor["illuminance"] < 30):
            command = {"state":"ON"}
            Kitchen_Move.isLightOn = True
            log.info("Kitchen_Move>switch lights on")
        else:
            command = None
            log.debug("Kitchen_Move>bright no light needed")
    else:
        if(Kitchen_Move.isLightOn):
            command = {"state":"OFF"}
            log.info("Kitchen_Move>switch lights off")
            Kitchen_Move.isLightOn = False
        else:
            command = None
            log.debug("Kitchen_Move>light is already off")
    return json.dumps(command)

    return command

def Bathroom_Hum_Status(input):
    red = 0
    green = 0
    blue = 0
    hum = float(input)

    if(hum < 60):
        green = 15
    else:
        green = 0
    if(hum < 50):
        blue = 0
    else:
        trig = (hum - 50.0) / 50.0
        blue = 200 * trig
    jColors = {"Red":red,"Green":green,"Blue":blue}
    return json.dumps(jColors)

def RGB_Tester_Burst(input):
    log.info("RGB_Tester_Burst() post")
    return "100"

def Bedroom_Light_Up(input):
    log.debug("Sleeproom_Light_Up>input(%s)",input)
    if(float(input) == 1.0):
        result = "100"
    else:
        result = None # ignored, do nothing
    return result

@static_vars(event_time=time())
def RGB_Tester_Single(input):
    result = None
    tnow = time()
    delay = tnow - RGB_Tester_Single.event_time
    if(delay > 2):
        RGB_Tester_Single.event_time = tnow
        log.info("RGB_Tester_Single() post after %f",delay)
        payload = {"Red":4,"Green":1,"Blue":3}
        result = json.dumps(payload)
    else:
        log.debug("RGB_Tester_Single() skipped sent since %f",delay)
    return result

def Button_To_Heat_Up(input):
    log.debug("Button_To_Heat_Up>input(%s)",input)
    if(float(input) == 1.0):
        result = 11
    else:
        result = None # ignored, do nothing
    return result

def Button_To_Heat_Down(input):
    log.debug("Button_To_Heat_Down>input(%s)",input)
    if(float(input) == 1.0):
        result = 12
    else:
        result = None # ignored, do nothing
    return result


def Bathroom_Window(input):
    a = json.loads(input)
    v_acc   = Vector(a["x"],a["y"],a["z"])
    v_earth = Vector(0,0,-1)
    angle = v_acc.angle(v_earth)
    log.debug("Bathroom Window>input(%s) =>output(%f)"%(input,angle))
    return angle

def Bathroom_Heating(input):
    a = json.loads(input)
    v_acc   = Vector(a["x"],a["y"],a["z"])
    v_closed = Vector(0.213,-0.998,-0.166)#zero reference
    angle = safe_angle(v_acc,v_closed)
    if(type(angle) == float):
        #angle = v_acc.angle(v_closed)
        log.debug("Bathroom Heating>input(%s) =>output(%f)"%(input,angle))
    return angle

def Bedroom_Direction_Remote(input):
    a = json.loads(input)
    list_dirs = [abs(a["x"]),abs(a["y"]),abs(a["z"])]
    max_dir = list_dirs.index(max(list_dirs))
    log.debug("Motion Event>direction(%d) [%0.3f , %0.3f , %0.3f]",max_dir,a["x"],a["y"],a["z"])
    if(max_dir == 0):
        result = 2
        log.debug("Dimmer>On")
    elif(max_dir == 1):
        result = 1
        log.debug("Dimmer>Night")
    elif(max_dir == 2):
        result = 0
        log.debug("Dimmer>Off")
    else:
        result = None
    return result


