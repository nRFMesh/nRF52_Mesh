import os
import datetime
import time
#Local imports
import cfg
import rf_uart as ser
import json
import logging as log

on_broadcast = None
on_message = None
on_cmd_response = None

nodes_config = os.getenv('NODES_CONFIG','/home/pi/IoT_Frameworks/config/nodes.json')
log.info("using NODES_CONFIG : %s",nodes_config)
nodes = cfg.get_local_nodes(nodes_config)

pid = {
    "exec_cmd"      : 0xEC,
    "ping"          : 0x01,
    "request_pid"   : 0x02,
    "chan_switch"   : 0x03,
    "reset"         : 0x04,
    "alive"         : 0x05,
    "button"        : 0x06,
    "light"         : 0x07,
    "temperature"   : 0x08,
    "heat"          : 0x09,
    "bme280"        : 0x0A,
    "rgb"           : 0x0B,
    "magnet"        : 0x0C,
    "dimmer"        : 0x0D,
    "light_rgb"     : 0x0E,
    "gesture"       : 0x0F,
    "proximity"     : 0x10,
    "humidity"      : 0x11,
    "pressure"      : 0x12,
    "acceleration"  : 0x13,
    "light_n"       : 0x14,
    "battery"       : 0x15,
    "test_rf_resp"  : 0x30
}

inv_pid = {v: k for k, v in pid.items()}

exec_cmd = {
    "set_node_id"   : 0x01,
    "get_node_id"   : 0x02,
    "set_channel"   : 0x03,
    "get_channel"   : 0x04,
    "set_tx_power"  : 0x05,
    "get_tx_power"  : 0x06,
    "set_param"     : 0x07,
    "get_param"     : 0x08,
}

set_rx = {
    "sniff" : 0x00,
    "bcast" : 0x01,
    "msg"   : 0x02,
    "resp"  : 0x03
}
mode = {
    "power_down"    : 0x01,
    "standby"       : 0x02,
    "tx_tdby2"      : 0x03,
    "rx"            : 0x04
}
inv_mode = {v: k for k, v in mode.items()}

msg = {
    "size":0,
    "payload":[]
}

def is_broadcast(hex_byte):
    return hex_byte.startswith("0x8")

def parse_pid(byte):
    return inv_pid[byte]

def parse_payload(data):
    res = ""
    if(data[2] == pid["light_n"]):
        light = int.from_bytes(bytearray(data[4:6]),'little',signed=False)
        res = "%d" %(light)
    elif(data[2] == pid["temperature"]):
        val = float(int.from_bytes(bytearray(data[4:8]),'big',signed=True)) / 100
        res = '{:02.2f}'.format(val)
    elif(data[2] == pid["humidity"]):
        val = float(int.from_bytes(bytearray(data[4:8]),'big',signed=True)) / 1024
        res = '{:02.2f}'.format(val)
    elif(data[2] == pid["pressure"]):
        val = float(int.from_bytes(bytearray(data[4:8]),'big',signed=True)) / (256*100)
        res = '{:02.2f}'.format(val)
    elif(data[2] == pid["acceleration"]):
        accel_x = float(int.from_bytes(bytearray(data[4:6]),'big',signed=True)) / 16384
        accel_y = float(int.from_bytes(bytearray(data[6:8]),'big',signed=True)) / 16384
        accel_z = float(int.from_bytes(bytearray(data[8:10]),'big',signed=True)) / 16384
        res = '(g) X {:02.2f} ; Y {:02.2f} ; Z {:02.2f}'.format(accel_x,accel_y,accel_z)
    elif(data[2] == pid["battery"]):
        bat_v = float(int.from_bytes(bytearray(data[4:6]),'big',signed=True)) / 1000
        res = 'battery {:02.3f} V'.format(bat_v)
    elif(data[2] == pid["button"]):
        if(data[4] == 0):
            res = 'release'
        else:
            res = 'press'
    if(data[2] == pid["light_rgb"]):
        light = int.from_bytes(bytearray(data[4:6]),'big',signed=False)
        red   = int.from_bytes(bytearray(data[6:8]),'big',signed=False)
        green = int.from_bytes(bytearray(data[8:10]),'big',signed=False)
        blue  = int.from_bytes(bytearray(data[10:12]),'big',signed=False)
        res = "light:%d , red:%d , green:%d , blue:%d" % (light,red,green,blue)
    return res

def parse_is_broadcast(byte):
    return (byte & 0x80)

def node_name(byte):
    res ="Unknown"
    if(str(byte) in nodes):
        res = nodes[str(byte)]["name"]
    return res

def publish(msg):
    pub = {}
    if("rssi" in msg):
        topic = "Nodes/"+msg["src"]+"/rssi"
        pub[topic] = int(msg["rssi"])
    if(inv_pid[int(msg["pid"])] == "alive"):
        #Publish both : database count
        topic = "Nodes/"+msg["src"]+"/alive"
        pub[topic] = int(msg["alive"])
        #and then publish json structure with more info
        topic = "jNodes/"+msg["src"]+"/alive"
        json_payload = {}
        json_payload["count"] = int(msg["alive"])
        nb_rx = int(msg["nb"])
        for i in range(nb_rx):
            rx_i = "rx"+str(i+1)
            json_payload[rx_i] = {}
            txpow_rssi_nid = msg[rx_i].split(',')
            json_payload[rx_i]["tx_power"]  = txpow_rssi_nid[0]
            json_payload[rx_i]["rssi"]      = txpow_rssi_nid[1]
            json_payload[rx_i]["nodeid"]    = txpow_rssi_nid[2]
        pub[topic] = json.dumps(json_payload)
    elif(inv_pid[int(msg["pid"])] == "bme280"):
        if("temp" in msg):
            topic_t = "Nodes/"+msg["src"]+"/temperature"
            pub[topic_t] = float(msg["temp"])
            topic_h = "Nodes/"+msg["src"]+"/humidity"
            pub[topic_h] = float(msg["hum"])
            topic_p = "Nodes/"+msg["src"]+"/pressure"
            pub[topic_p] = float(msg["press"])
    elif(inv_pid[int(msg["pid"])] == "light"):
        if("light" in msg):
            topic = "Nodes/"+msg["src"]+"/light"
            pub[topic] = float(msg["light"])
    elif(inv_pid[int(msg["pid"])] == "battery"):
        topic = "Nodes/"+msg["src"]+"/battery"
        pub[topic] = float(msg["battery"])
    elif(inv_pid[int(msg["pid"])] == "acceleration"):
        topic = "jNodes/"+msg["src"]+"/acceleration"
        if("accx" in msg):  #check accx is enough as some have size error logs
            json_payload = {}
            json_payload["x"] = float(msg["accx"])
            json_payload["y"] = float(msg["accy"])
            json_payload["z"] = float(msg["accz"])
            pub[topic] = json.dumps(json_payload)
    elif(inv_pid[int(msg["pid"])] == "button"):
        topic = "Nodes/"+msg["src"]+"/button"
        pub[topic] = int(msg["button"])
    elif(inv_pid[int(msg["pid"])] == "reset"):
        topic = "Nodes/"+msg["src"]+"/reset"
        pub[topic] = float(msg["reset"])
    return pub

def line2dict(line):
    res = {}
    entries = line.split(';')
    for entry in entries:
        kv = entry.split(':')
        if(len(kv)==2):
            res[kv[0]] = kv[1]
    return res

def command(cmd,params=[]):
    cmd_list = [exec_cmd[cmd]]+params
    text_cmd = "cmd:0x" + ''.join('%02X' % b for b in cmd_list)+"\r\n"
    ser.send(text_cmd)
    return

def send(payload):
    #print("payload:",payload)
    text_msg = "msg:0x"+''.join('%02X' % b for b in payload)+"\r\n"
    ser.send(text_msg)
    return

def serial_on_line(line):
    #print(line)
    ldict = line2dict(line)
    if("ctrl" in ldict):
        if(is_broadcast(ldict["ctrl"])):
            on_broadcast(ldict)
        else:
            if("cmd" in ldict):
                on_cmd_response(ldict,True)
                log.info("remote cmd resp > "+line)
            else:
                on_message(ldict)
            #log.info("msg > "+line)
    elif("cmd" in ldict):
        on_cmd_response(ldict,False)
        log.info("cmd resp > "+line)
    return

def run():
    ser.run()
    return

def start(config,mesh_on_broadcast,mesh_on_message,mesh_on_cmd_response):
    global on_broadcast
    global on_message
    global on_cmd_response
    on_broadcast = mesh_on_broadcast
    on_cmd_response = mesh_on_cmd_response
    on_message = mesh_on_message
    ser.serial_start(config,serial_on_line)
    return
