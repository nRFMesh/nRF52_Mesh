import os
import pylink
import cfg
import sys

import inspect, os

currdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
#nodes = cfg.get_local_nodes(os.environ['NODES_CONFIG'])
nodes = cfg.get_json(currdir+"\\..\\applications\\nodes.json")


#  command example
#  python uicr.py "../applications/uicr_map.json"
if(len(sys.argv) > 1):
    uicr_cfongi_file = sys.argv[1]
    print("arg0:%s"%(uicr_cfongi_file))
else:
    uicr_cfongi_file = "../applications/uicr_map.json"

#second command line argument to read or write
if(len(sys.argv) > 2):
    mode = sys.argv[2]
    print("arg1:%s"%(mode))
else:
    mode = "-x"

uicr = cfg.get_json(uicr_cfongi_file)
jlink = pylink.JLink()
jlink.open(os.environ['SEG_JLEDU'])
node_id = 0
node = {}

def get_node_id_from_uid(uid):
    res =""
    for key,val in nodes.items():
        if "uid" in val:
            if val["uid"] == uid:
                res = key
    return res

def get_uid_32():
    device_id = jlink.memory_read32(0x100000A4, 2)
    ids1 = "{0:#0{1}X}".format(device_id[0],10)
    ids2 = "{0:#0{1}X}".format(device_id[1],10)
    print("device id : %s %s" % (ids1,ids2) )
    return

def get_uid():
    d = jlink.memory_read8(0x10000060, 8)
    res = "0x%02X %02X %02X %02X %02X %02X %02X %02X"%(d[3],d[2],d[1],d[0],d[7],d[6],d[5],d[4])
    return res

def read_uicr_customer(reg_name):
    reg_nb = int(reg_name.lstrip("CUSTOMER_"))
    address = 0x10001080 + 4*reg_nb
    return jlink.memory_read32(address,1)[0]

def write_uicr_customer(reg_name,val):
    pre_val = read_uicr_customer(reg_name)
    test_val = int(val,0)
    if(pre_val == test_val):
        print("%s already has 0x%04X, new write not required"%(reg_name,test_val))
    else:
        reg_nb = int(reg_name.lstrip("CUSTOMER_"))
        address = 0x10001080 + 4*reg_nb
        if((type(val) is str) or (type(val) is unicode)):
                words_list = [int(val,0)]
        else:
            words_list = [val]
        jlink.memory_write32(address,words_list)
    return

def start():
    jlink.set_tif(pylink.enums.JLinkInterfaces.SWD)
    jlink.connect('NRF52832_XXAA', verbose=True)
    print('ARM Id: %d' % jlink.core_id())
    print('CPU Id: %d' % jlink.core_cpu())
    print('Core Name: %s' % jlink.core_name())
    print('Device Family: %d' % jlink.device_family())
    return

def read_id():
    global node_id
    global node
    uid = get_uid()
    print("device uid : %s" % uid )
    node_id = get_node_id_from_uid(uid)
    if(node_id):
        print("node mesh id : %s" % node_id )
        node = nodes[node_id]
        print("name : %s" % node["name"] )
        return True
    else:
        print("uid unknown")
        return  False

def read_config():
    global node_id
    global node
    for param,reg in uicr.items():
        if param == "mesh_id":
            val = read_uicr_customer(reg)
            print("reg %s => (mesh_id target/db :%d / %s)"%(reg,val,node_id))
        else:
            val = read_uicr_customer(reg)
            if(param in node):
                print("reg %s => (%s target/db :%d / %s)"%(reg,param,val,node[param]))
            else:
                print("reg %s => (%s target/db :%d / N.A)"%(reg,param,val))
    return

def write_config():
    global node_id
    global node
    for param,reg in uicr.items():
        if param == "mesh_id":
            print("reg %s <= (mesh_id:%s)"%(reg,node_id))
            write_uicr_customer(            reg,node_id)
        else:
            if(param in node):
                print("reg %s <= (%s:%s)"%(reg,param,node[param]))
                write_uicr_customer(            reg,node[param])
            else:
                print("reg %s X (%s: N.A)"%(reg,param))

    test_pass = True

    for param,reg in uicr.items():
        if param == "mesh_id":
            test_val = read_uicr_customer(reg)
            if(test_val != int(node_id)):
                test_pass = False
        else:
            test_val = read_uicr_customer(reg)
            if(param in node):
                if(test_val != int(node[param],0)):
                    test_pass = False
            #else: do nothing as test is kept pass if param not in db node(id)
            
    if(not test_pass):
        print("Verification failed")
        read_config()
    else:
        print("write verified")
    return

if(mode == "-r"):
    start()
    if(read_id()):
        read_config()
    else:
        print("node id unavailable")
elif(mode == "-w"):
    start()
    read_id()
    write_config()
#else do nothing
