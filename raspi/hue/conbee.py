import asyncio
import websockets
import json
import signal
import sys
import requests
import cfg
from mqtt import mqtt_start

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

async def websocket_sensor_events():
    async with websockets.connect(config_file["websocket"]["url"]) as websocket:
        #name = input("What's your name? ")
        #await websocket.send(name)
        #print(f"> {name}")
        while(True):
            message = await websocket.recv()
            sensor_event = json.loads(message)
            print(sensor_event)
            sid = sensor_event["id"]
            sname = sensors_map[sid]["name"]
            stype = sensors_map[sid]["type"]
            state_name = None
            if(stype == "ZHATemperature"):
                state_name = "temperature"
            elif(stype == "ZHAHumidity"):
                state_name = "humidity"
            elif(stype == "ZHAPressure"):
                state_name = "pressure"
            print(f"Sensor Event,  id: {sid} , name: {sname} , type: {stype}")
            if(state_name):
                value = sensors_map[sid]["state"][state_name]
                print(f"value = {value}")
                #topic = "Nodes/"+node_id+"/"+state_name
                #clientMQTT.publish(topic,payload)
            else:
                print("state event unknown")


config_file = cfg.get_local_json()
response = requests.get(config_file["gateway"]["url"]+"/sensors")
sensors_map = response.json()
print("received config")
show_sensors(sensors_map)

#will start a separate thread for looping
clientMQTT = mqtt_start(config_file,mqtt_on_message)

print("Entering webscoket loop")
loop = asyncio.get_event_loop()
loop.run_until_complete(websocket_sensor_events())
