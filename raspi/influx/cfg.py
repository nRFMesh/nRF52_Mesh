import sys,os
import json

# -------------------- config -------------------- 
def get_local_json(loca_file_name):
    dirname = os.path.dirname(sys.argv[0])
    if(len(dirname) == 0):
        dirname = "."
    config_file = dirname+'/'+loca_file_name
    config = json.load(open(config_file))
    return config
