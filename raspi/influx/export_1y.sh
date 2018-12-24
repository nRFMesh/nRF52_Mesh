influx -database 'meshNodes' -execute 'SELECT "temperature" FROM "node78" WHERE time > now() - 1y' -format 'csv' > /home/pi/share/$(date +%F)_influx_backup.csv
