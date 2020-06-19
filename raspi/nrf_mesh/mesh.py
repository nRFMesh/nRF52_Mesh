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

config = cfg.configure_log(__file__)
nodes_config_file = '/home/pi/nRF52_Mesh/nodes.json'
log.info("mesh> using local : %s",nodes_config_file)
nodes = cfg.get_local_nodes(nodes_config_file)

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
    "text"          : 0x16,
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

def int8_signed(byte):
    val = int(byte)
    if(val > 127):
        return (256-val) * (-1)
    else:
        return val

def publish(msg):
    pub = {}
    if(msg["src"] in nodes):
        src_name = nodes[msg["src"]]["name"]
    else:#keep number as identifier
        src_name = msg["src"]
    if(config["mesh"]["whitelist"] != []):
        if(src_name not in config["mesh"]["whitelist"]):
            log.debug(f"{src_name} not whitelisted - discarded")
            return
    if(config["mesh"]["blacklist"] != []):
        if(src_name in config["mesh"]["blacklist"]):
            log.debug(f"{src_name} balcklisted - discarded")
            return
    topic = src_name
    json_payload = {}
    if("rssi" in msg):
        json_payload["rssi"] = int(msg["rssi"])
    if(inv_pid[int(msg["pid"])] == "alive"):
        json_payload["alive_count"] = int(msg["alive"])
        nb_rx = int(msg["nb"])
        for i in range(nb_rx):
            rx_i = "rx"+str(i+1)
            json_payload["path"] = []
            txpow_rssi_nid = msg[rx_i].split(',')
            path_entry = {}
            path_entry["tx_power"]  = int8_signed(txpow_rssi_nid[0])
            path_entry["rssi"]      = txpow_rssi_nid[1]
            last_rssi                       = txpow_rssi_nid[1]
            path_entry["nodeid"]    = txpow_rssi_nid[2]
            json_payload["path"].append(path_entry)
        json_payload["rssi"] = int(last_rssi)
        pub[topic] = json_payload
    elif(inv_pid[int(msg["pid"])] == "bme280"):
        if("temp" in msg):
            json_payload["temperature"] = float(msg["temp"])
            json_payload["humidity"] = float(msg["hum"])
            json_payload["pressure"] = float(msg["press"])
    elif(inv_pid[int(msg["pid"])] == "temperature"):
        if("temp" in msg):
            json_payload["temperature"] = float(msg["temp"])
    elif(inv_pid[int(msg["pid"])] == "light"):
        if("light" in msg):
            json_payload["light"] = float(msg["light"])
    elif(inv_pid[int(msg["pid"])] == "battery"):
        json_payload["battery"] = float(msg["battery"])
    elif(inv_pid[int(msg["pid"])] == "acceleration"):
        if("accx" in msg):  #check accx is enough as some have size error logs
            json_payload = {}
            json_payload["x"] = float(msg["accx"])
            json_payload["y"] = float(msg["accy"])
            json_payload["z"] = float(msg["accz"])
            pub[topic] = json_payload
    elif(inv_pid[int(msg["pid"])] == "button"):
        if(int(msg["button"]) == 1):
            json_payload["button"] = "down"
        else:
            json_payload["button"] = "up"
    elif(inv_pid[int(msg["pid"])] == "reset"):
        json_payload["reset"] = float(msg["reset"])
    pub[topic] = json_payload
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
    log.debug("mesh> uart> "+line)
    ldict = line2dict(line)
    if("ctrl" in ldict):
        if(is_broadcast(ldict["ctrl"])):
            on_broadcast(ldict)
        else:
            if("cmd" in ldict):
                on_cmd_response(ldict,True)
                log.info("mesh> remote cmd resp > "+line)
            else:
                on_message(ldict)
            #log.info("msg > "+line)
    elif("cmd" in ldict):
        on_cmd_response(ldict,False)
        log.info("mesh> cmd resp > "+line)
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

def stop():
    ser.serial_stop()
    return
