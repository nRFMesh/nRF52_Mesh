import sys,os
import json
import socket
from collections import OrderedDict

# -------------------- config -------------------- 
def get_local_json():
    """fetches the config.json file in the local directory
       if config_hostname.json is found it is used over the default one
    """
    config = None
    dirname = os.path.dirname(sys.argv[0])
    if(len(dirname) == 0):
        dirname = "."
    config_file = dirname+'/'+"config_"+socket.gethostname()+".json"
    if(os.path.isfile(config_file)):
        #print("loading: ",config_file)
        config = json.load(open(config_file))
    else:
        config_file = dirname+'/'+"config.json"
        if(os.path.isfile(config_file)):
            #print("loading: %s",config_file)
            config = json.load(open(config_file))
        else:
            print("Fatal error 'config.json' not found")
    return config

# -------------------- config -------------------- 
def get_json(nodes_file):
    nodes = json.load(open(nodes_file),object_pairs_hook=OrderedDict)
    return nodes
