influx -database 'meshNodes' -execute 'SELECT "temperature" FROM "node78" WHERE time > 2017-01-01 and time < 2017-12-12T23:59:59Z' -format 'csv' > /home/pi/share/$(date +%F)_influx_backup.csv


