import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
import influxdb
import requests
import datetime
import logging as log
import cfg
from time import sleep
import socket
import json

# -------------------- mqtt events -------------------- 
def on_connect(lclient, userdata, flags, rc):
    log.info("mqtt connected with result code "+str(rc))
    lclient.subscribe("Nodes/#")
    lclient.subscribe("zigbee2mqtt/#")

def on_message(client, userdata, msg):
    topic_parts = msg.topic.split('/')
    try:
        post = None
        if( (len(topic_parts) == 3) and (topic_parts[0] == "Nodes") ):
            nodeid = topic_parts[1]
            sensor = topic_parts[2]
            measurement = "node"+nodeid
            value = float(str(msg.payload))
            post = [
                {
                    "measurement": measurement,
                    "time": datetime.datetime.utcnow(),
                    "fields": {
                        sensor: value
                    }
                }
            ]
        elif( (len(topic_parts) == 2) and (topic_parts[0] == "zigbee2mqtt") ):
            sensor = topic_parts[1]
            fields = json.loads(msg.payload)
            
            if("pressure" in fields):
                fields["pressure"] = int(fields["pressure"]) #force pressure to int
            if("voltage" in fields):
                fields["voltage"] = float(fields["voltage"])/1000 #convert voltage from milivolts to Volts
            post = [
                {
                    "measurement": sensor,
                    "time": datetime.datetime.utcnow(),
                    "fields": fields
                }
            ]
        if(post != None):
            try:
                clientDB.write_points(post)
                log.debug(msg.topic+" "+str(msg.payload)+" posted")
            except requests.exceptions.ConnectionError:
                log.error("ConnectionError sample skipped "+msg.topic)
            except influxdb.exceptions.InfluxDBServerError:
                log.error("InfluxDBServerError sample skipped "+msg.topic)
            except influxdb.exceptions.InfluxDBClientError as e:
                log.error("InfluxDBClientError with "+msg.topic+" : " +str(msg.payload)+" >>> "+str(e) )
    except ValueError:
        log.error(" ValueError with : "+msg.topic+" "+str(msg.payload))

def mqtt_connect_retries():
    connected = False
    while(not connected):
        try:
            clientMQTT.connect(config["mqtt"]["host"], config["mqtt"]["port"], config["mqtt"]["keepalive"])
            connected = True
        except socket.error:
            log.error("socket.error will try a reconnection in 10 s")
        sleep(10)
    return

# -------------------- main -------------------- 
config = cfg.configure_log(__file__)

# -------------------- influxDB client -------------------- 
clientDB = InfluxDBClient(    config["influxdb"]["host"], 
                            config["influxdb"]["port"], 
                            'root', 'root', 
                            config["influxdb"]["db"])



#clientDB.create_database(config["influxdb"]["db"])
#print("database created")
#clientDB.write_points(post)
#result = clientDB.query('select temperature from node15;')
#print("Query Result: {0}".format(result))

# -------------------- Mqtt Client -------------------- 
cid = config["influxdb"]["mqtt_client_id"] +"_"+socket.gethostname()
clientMQTT = mqtt.Client(client_id=cid)
clientMQTT.on_connect = on_connect
clientMQTT.on_message = on_message

mqtt_connect_retries()

clientMQTT.loop_forever()
