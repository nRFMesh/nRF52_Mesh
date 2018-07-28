#!/bin/bash

#first time install :
#pip install -r requirements.txt
#python3 -m pip install -r requirements.txt

#services
mosquitto -c /etc/mosquitto/mosquitto.conf &
sudo systemctl start grafana-server.service

#scripts
python rf_uart/mesh_controller.py -c 2 -f l &
python wemo/wemo_client.py &
python milight/milight_gateway.py &
python3 ruler/ruler.py &
#this script would fail if influxdb is not yet available, it is therefore delayed
sleep 15 && python influx/influx_client.py &

