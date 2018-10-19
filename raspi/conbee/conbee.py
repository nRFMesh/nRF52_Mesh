import asyncio
import websockets
import json
import signal
import sys
import requests
import cfg
from mqtt import mqtt_start
import os
import logging as log
from collections import OrderedDict

def signal_handler(signal, frame):
        print('You pressed Ctrl+C!')
        sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)
print('Press Ctrl+C')

def mqtt_on_message(client, userdata, msg):
    log.error("mqtt> Error: Unexpected mqtt message")
    return

def show_sensors(sensors_map):
    ordered_map = OrderedDict()
    for i in range(len(sensors_map)):
        index = i+1
        name = sensors_map[str(index)]["name"]
        sensor_type = sensors_map[str(index)]["type"]
        print(f"{index} : {name} : {sensor_type}")
    return

def cubeevent_to_json(buttonevent,sensors_map,sid):
    res = None
    #TODO instead of first key should check for ids ending with 0x12 and 0x0C
    first_id = cfg.get_first_key_from_param("modelid","lumi.sensor_cube.aqgl01",sensors_map)
    if(sid == first_id):
        json_payload = {}
        if(buttonevent == 7000):
            json_payload["event"] = "wakeup"
        elif(buttonevent == 7007):
            json_payload["event"] = "shake"
        elif(buttonevent == 7008):
            json_payload["event"] = "drop"
        else:
            event_to   = int(buttonevent / 1000)
            event_from = int(buttonevent %   10)
            if(event_from == event_to):
                json_payload["event"] = "double_tap"
                json_payload["face"] = event_to
            elif(event_from == 0):
                json_payload["event"] = "push"
                json_payload["face"] = event_to
            else:
                json_payload["event"] = "flip"
                json_payload["from"] = event_from
                json_payload["to"] = event_to
        res = json.dumps(json_payload)
    else:
        json_payload = {}
        json_payload["rotation"] = buttonevent
        res = json.dumps(json_payload)
    #there will be an else that uses res None if it's not 0x12 and not 0x0C
    return res

def motionevent_to_json(buttonevent):
    res = None
    json_payload = {}
    if(buttonevent == 1007):
        json_payload["event"] = "shake"
    elif(buttonevent == 1008):
        json_payload["event"] = "drop"
    elif(buttonevent == 1009):
        json_payload["event"] = "tilt"
    else:
        json_payload["motion"] = buttonevent
    res = json.dumps(json_payload)
    return res

def switchevent_to_json(buttonevent):
    res = None
    json_payload = {"switch":buttonevent}
    res = json.dumps(json_payload)
    return res

def buttonevent_to_json(buttonevent,sensors_map,sid):
    res = None
    modelid = sensors_map[sid]["modelid"]
    if(modelid == "lumi.sensor_cube.aqgl01"):
        res = cubeevent_to_json(buttonevent,sensors_map,sid)
    elif(modelid == "lumi.vibration.aq1"):
        res = motionevent_to_json(buttonevent)
    elif(modelid == "lumi.sensor_86sw1"):
        res = switchevent_to_json(buttonevent)
    elif(modelid == "lumi.remote.b1acn01"):
        res = switchevent_to_json(buttonevent)
    else:
        log.error("button event modelid unknown")
    return res

def water_to_json(sensor_event):
    res = None
    json_payload = {}
    json_payload["water"] = sensor_event["state"]["water"]
    json_payload["tampered"] = sensor_event["state"]["tampered"]
    json_payload["lowbattery"] = sensor_event["state"]["lowbattery"]
    res = json.dumps(json_payload)
    return res

async def websocket_sensor_events():
    log.info("websocket_sensor_events() start")
    async with websockets.connect(config_file["conbee"]["websocket"]) as websocket:
        while(True):
            message = await websocket.recv()
            log.debug(message)
            sensor_event = json.loads(message)
            sid = sensor_event["id"]
            sname = sensors_map[sid]["name"]
            smodelid = sensors_map[sid]["modelid"]
            node_id = cfg.get_node_id_from_name(sname,nodes)
            if(not node_id):
                log.warning("%s not found in nodes.json"%(sname))
            #node_id is good now convert the Zigbee sensor event type to the simple sensor MQTT name
            stype = sensors_map[sid]["type"]

            topic_node = "Nodes/"+node_id+"/"
            topic_model = "zigbee/" + smodelid + "/" + sname
            payload_model = None
            payload_node = None
            if("state" in sensor_event):
                if(stype == "ZHATemperature"):
                    topic_node = topic_node+"temperature"
                    payload_node = float(sensor_event["state"]["temperature"])/100
                elif(stype == "ZHAHumidity"):
                    topic_node = topic_node+"humidity"
                    payload_node = float(sensor_event["state"]["humidity"])/100
                elif(stype == "ZHAPressure"):
                    topic_node = topic_node+"pressure"
                    payload_node = int(sensor_event["state"]["pressure"])
                elif(stype == "ZHASwitch"):
                    payload_model = buttonevent_to_json(sensor_event["state"]["buttonevent"],sensors_map,sid)
                elif(stype == "ZHALightLevel"):
                    topic_node = topic_node+"light"
                    payload_node = int(sensor_event["state"]["lux"])
                    payload_model = json.dumps({"light":sensor_event["state"]["lux"]})
                elif(stype == "ZHAPresence"):
                    #presence is either true or false for alive
                    payload_model = json.dumps({"presence":sensor_event["state"]["presence"]})
                elif(stype == "ZHAOpenClose"):
                    payload_model = "closed"
                    if(sensor_event["state"]["open"]):#convert from trueopen:true,false to "open,closed"
                        payload_model = "open"
                elif(stype == "ZHAWater"):
                    payload_model = water_to_json(sensor_event)
                else:
                    log.error("event type (%s) unknown"%(stype))
            elif("config" in sensor_event):
                #config should always contain battery
                topic_node = topic_node+"battery_percent"
                payload_node = sensor_event["config"]["battery"]
            if(config_file["mqtt"]["publish"]):
                if(payload_node is not None):
                    clientMQTT.publish(topic_node,payload_node)
                    log.debug("published on : %s => %s"%(topic_node,payload_node))
                if(payload_model is not None):
                    clientMQTT.publish(topic_model,payload_model)
                    log.debug("published on : %s => %s"%(topic_model,payload_model))
            else:
                log.warning("publish not enabled !!!! skipped : %s => %s"%(topic,payload))
        log.error("Done with the While loop")



nodes_config = os.getenv('NODES_CONFIG','/home/pi/nRF52_Mesh/raspi/mesh_wizard/nodes.json')
log.info("using NODES_CONFIG : %s",nodes_config)
nodes = cfg.get_local_nodes(nodes_config)

config_file = cfg.configure_log(__file__)
response = requests.get(config_file["conbee"]["rest"]+"/sensors")
sensors_map = response.json()
log.info("received config")
show_sensors(sensors_map)

#will start a separate thread for looping
clientMQTT = mqtt_start(config_file,mqtt_on_message,True)

log.info("Entering webscoket loop")
loop = asyncio.get_event_loop()
log.info("after get_event_loop()")
while(True):
    try:
        loop.run_until_complete(websocket_sensor_events())
    except websockets.exceptions.ConnectionClosed:
        log.error("conbee websocket connection lost (websockets.exceptions.ConnectionClosed)")
        log.info("will retry connection in 10 sec")
        sleep(10)
