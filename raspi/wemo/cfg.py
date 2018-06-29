import sys,os
import json
import logging as log

# -------------------- config -------------------- 
def get_local_json(loca_file_name):
    dirname = os.path.dirname(sys.argv[0])
    if(len(dirname) == 0):
        dirname = "."
    config_file = dirname+'/'+loca_file_name
    config = json.load(open(config_file))
    return config


def configure_log(config):
    log_level_map = {
        "Debug"     :10,
        "Info"      :20,
        "Warning"   :30,
        "Error"     :40,
        "Critical"  :50
    }
    log.basicConfig(    filename=config["logfile"],
                        level=log_level_map[config["level"]],
                        format='%(asctime)s %(name)s %(levelname)-8s %(message)s',
                        datefmt='%d %H:%M:%S'
                        )
    log.getLogger('').addHandler(log.StreamHandler())
    log.info("log started @ level:%s",config["level"])
    return
