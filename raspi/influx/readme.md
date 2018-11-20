
# influx queries examples

    SELECT "temperature" FROM "node91" WHERE $timeFilter


    SELECT mean("battery") FROM "node78" WHERE $timeFilter GROUP BY time(3h)
    SELECT mean("battery") FROM "node78" WHERE $timeFilter GROUP BY time(24h)


    Show Field Keys
    drop MEASUREMENT "motion button"

# Install on the Raspberry pi
sources:
- https://gist.github.com/boseji/bb71910d43283a1b84ab200bcce43c26
- https://docs.influxdata.com/influxdb/v1.4/introduction/getting_started/
- https://github.com/influxdata/influxdb/blob/master/QUERIES.md

```
sudo apt install apt-transport-https
echo "deb https://repos.influxdata.com/debian stretch stable" | sudo tee /etc/apt/sources.list.d/influxdb.list
sudo apt-get update

sudo apt-get install influxdb

```

### config
Default
```
sudo nano /etc/influxdb/influxdb.conf
```
directly from the repo
```
INFLUXDB_CONFIG_PATH=/home/pi/IoT_Frameworks/config/influxdb/influxdb.conf
```

### start the service
```
sudo systemctl start influxdb
```

### help reminder, see getting started link for more
```
influx -precision rfc3339
CREATE DATABASE mydb,
SHOW DATABASES
SHOW SERIES on raspiStatus
USE mydb

DROP DATABASE mydb

cpu,host=serverA,region=us_west value=0.64

SHOW FIELD KEYS FROM "cpu_temp"
SHOW TAG KEYS FROM "cpu_temp"
SHOW TAG VALUES FROM "cpu_temp" WITH KEY="host"

SELECT * FROM "cpu_temp" WHERE "host" = 'ioserv'

SELECT "power" FROM "node30" WHERE time > '2017-12-24T12:33:00Z' AND time < '2017-12-24T15:34:10Z'
DELETE FROM "node37" WHERE time > '2017-12-24T12:33:00Z' AND time < '2017-12-24T15:34:10Z'
```
### Nodes posts
```
post = [
    {
        "measurement": "node6",
        "time": datetime.datetime.utcnow(),
        "fields": {
            "temperature": value
        }
    }
]
```
### Grafana Queries
```
SELECT "temperature" FROM "node6" WHERE $timeFilter
```
### Raspi status posts
```
posts = [
    {
        "measurement": "cpu_load",
        "time": tnow,
        "tags":{
            "host":hostname
        },
        "fields": {
            "value": float(rasp.getCPU_Avg1min())
        }
    }
]
```

### Raspi queries
```
SELECT "value" FROM "cpu_load"
```
