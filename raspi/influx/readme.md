
# influx queries examples

    SELECT "temperature" FROM "node91" WHERE $timeFilter


    SELECT mean("battery") FROM "node78" WHERE $timeFilter GROUP BY time(3h)
    SELECT mean("battery") FROM "node78" WHERE $timeFilter GROUP BY time(24h)


    Show Field Keys
    drop MEASUREMENT "motion button"
    