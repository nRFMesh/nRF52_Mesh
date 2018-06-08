import os
import datetime
import time
#Local imports
import cfg
import rf_uart as ser
import json
import logging as log

on_broadcast = None
on_cmd_response = None

nodes = cfg.get_local_nodes(os.environ['NODES_CONFIG'])

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
    "set_power"     : 0x05,
    "get_power"     : 0x06,
    "config"        : 0x32
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

def parse_control(byte):
    res = ""
    if(byte & 0x80):
        res = res + "Broadcast "
    else:
        res = res + "Directed "
        if(byte & 0x40):
            res = res + "Msg_Ack "
            if(byte & 0x20):
                res = res + "Message "
                if(byte & 0x10):
                    res = res + "To_Send_Ack "
                else:
                    res = res + "Do_Not_Send_Ack "
            else:
                res = res + "Acknowledge "
        else:
            res = res + "Req_Res "
            if(byte & 0x20):
                res = res + "Request "
            else:
                res = res + "Response "
    ttl = byte & 0x0F
    res = res + "ttl "+str(ttl)
    return res

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
    if(inv_pid[int(msg["id"])] == "bme280"):
        topic_t = "Nodes/"+msg["src"]+"/temperature"
        pub[topic_t] = float(msg["temp"])
        topic_h = "Nodes/"+msg["src"]+"/humidity"
        pub[topic_h] = float(msg["hum"])
        topic_p = "Nodes/"+msg["src"]+"/pressure"
        pub[topic_p] = float(msg["press"])
    if(inv_pid[int(msg["id"])] == "light"):
        topic = "Nodes/"+msg["src"]+"/light"
        pub[topic] = float(msg["light"])
    if(inv_pid[int(msg["id"])] == "acceleration"):
        topic = "jNodes/"+msg["src"]+"/acceleration"
        json_payload = {}
        json_payload["x"] = float(msg["accx"])
        json_payload["y"] = float(msg["accy"])
        json_payload["z"] = float(msg["accz"])
        pub[topic] = json.dumps(json_payload)
    return pub

def parse_rf_data(data):
    rf_data_text = parse_pid(data[2])
    rf_data_text += " : "+parse_payload(data)+" "
    if(data[2] == pid["test_rf_resp"]):
        rf_data_text += " res="+str(data[5])+" "
    rf_data_text += "(" + node_name(data[3]) + " -> "
    if(not parse_is_broadcast(data[1])):
        rf_data_text += node_name(data[4]) + ") ; "
    else:
        rf_data_text += " X) ; "
    rf_data_text +=  parse_control(data[1])
    return rf_data_text

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

def send_msg(payload):
    log.debug("tx>%s",parse_rf_data(payload))
    text_msg = "msg:0x"+''.join('%02X' % b for b in payload)+"\r\n"
    ser.send(text_msg)
    return

def serial_on_line(line):
    ldict = line2dict(line)
    if("ctrl" in ldict):
        if(is_broadcast(ldict["ctrl"])):
            on_broadcast(ldict)
    if("cmd" in ldict):
        handled = on_cmd_response(ldict)
        if(not handled):
            log.info("rf  > "+line)
    return

def run():
    ser.run()
    return

def start(config,mesh_on_broadcast,mesh_on_cmd_response):
    global on_broadcast
    global on_cmd_response
    on_broadcast = mesh_on_broadcast
    on_cmd_response = mesh_on_cmd_response
    ser.serial_start(config,serial_on_line)
    return
