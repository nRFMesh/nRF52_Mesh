#!/bin/bash
echo "Backing up the influx Databse"
influxd backup /home/pi/share/$(date +%F)_influx_backup
influxd backup -database meshNodes -since 2017-12-01T00:00:00Z /home/pi/share/$(date +%F)_influx_backup
influxd backup -database raspiStatus -since 2017-12-01T00:00:00Z /home/pi/share/$(date +%F)_influx_backup
