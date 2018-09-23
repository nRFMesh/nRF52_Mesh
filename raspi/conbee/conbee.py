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

def signal_handler(signal, frame):
        print('You pressed Ctrl+C!')
        sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)
print('Press Ctrl+C')

def mqtt_on_message(client, userdata, msg):
    log.error("mqtt> Error: Unexpected mqtt message")
    return

def show_sensors(sensors_map):
    for key,sensor in sensors_map.items():
        name = sensor["name"]
        sensor_type = sensor["type"]
        print(f"{key} : {name} : {sensor_type}")
    return

def buttonevent_to_json(buttonevent,sensors_map,sid):
    res = None
    modelid = sensors_map[sid]["modelid"]
    if(modelid == "lumi.sensor_cube.aqgl01"):
        #TODO find if the sid is the first in sensors_map then it's events, or if second then it's gyro rotation analog value
        json_payload = {}
        json_payload["event"] = buttonevent
        res = json.dumps(json_payload)
        print("button => cube")
    elif(modelid == "lumi.vibration.aq1"):
        json_payload = {"motion":"high"}
        res = json.dumps(json_payload)
        print("button => vibration")
    else:
        print("button to nothing")
    return res

async def websocket_sensor_events():
    async with websockets.connect(config_file["websocket"]["url"]) as websocket:
        #name = input("What's your name? ")
        #await websocket.send(name)
        #print(f"> {name}")
        while(True):
            message = await websocket.recv()
            log.debug(message)
            sensor_event = json.loads(message)
            sid = sensor_event["id"]
            sname = sensors_map[sid]["name"]
            node_id = cfg.get_node_id_from_name(sname,nodes)
            if(not node_id):
                log.error("%s not found in nodes.json"%(sname))
                continue
            #node_id is good now convert the Zigbee sensor event type to the simple sensor MQTT name
            stype = sensors_map[sid]["type"]
            payload = None
            if("state" in sensor_event):
                if(stype == "ZHATemperature"):
                    topic = "Nodes/"+node_id+"/"+"temperature"
                    payload = float(sensor_event["state"]["temperature"])/100
                elif(stype == "ZHAHumidity"):
                    topic = "Nodes/"+node_id+"/"+"humidity"
                    payload = float(sensor_event["state"]["humidity"])/100
                elif(stype == "ZHAPressure"):
                    topic = "Nodes/"+node_id+"/"+"pressure"
                    payload = int(sensor_event["state"]["pressure"])
                elif(stype == "ZHASwitch"):
                    topic = "jNodes/"+node_id+"/"+"button"
                    #payload = sensor_event["state"]["buttonevent"]
                    payload = buttonevent_to_json(sensor_event["state"]["buttonevent"],sensors_map,sid)
                else:
                    log.error("event type (%s) unknown"%(stype))
            elif("config" in sensor_event):
                #config should always contain battery
                topic = "Nodes/"+node_id+"/"+"battery_percent"
                payload = sensor_event["config"]["battery"]
            if(payload):
                clientMQTT.publish(topic,payload)
                log.debug("published on : %s => %s"%(topic,payload))



config_file = cfg.configure_log(__file__)

nodes_config = os.getenv('NODES_CONFIG','/home/pi/nRF52_Mesh/raspi/mesh_wizard/nodes.json')
log.info("using NODES_CONFIG : %s",nodes_config)
nodes = cfg.get_local_nodes(nodes_config)

#config_file = cfg.get_local_json()
response = requests.get(config_file["gateway"]["url"]+"/sensors")
sensors_map = response.json()
log.info("received config")
show_sensors(sensors_map)

#will start a separate thread for looping
clientMQTT = mqtt_start(config_file,mqtt_on_message)

log.info("Entering webscoket loop")
loop = asyncio.get_event_loop()
loop.run_until_complete(websocket_sensor_events())
