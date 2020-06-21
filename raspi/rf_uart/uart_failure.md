# Exception occurred
serial.serialutil.SerialException

# Assumptions
* /etc/rc.local started for no reason trying to add another instance on top of existing one

# Resolution
* Won't be investigated, right way is to use a service or other

# Log

Error log
```
[2020-06-21 04:36:47] Retrieving data from sensor "Flora1" ...
rf  > 110 kitchen tag : bme280
rf  > 76 bedroom tag : alive
('loading: %s', '/home/pi/nRF52_Mesh/raspi/rf_uart/config.json')
uart> /dev/ttyUSB1
Traceback (most recent call last):
  File "/home/pi/nRF52_Mesh/raspi/rf_uart/mesh_controller.py", line 213, in <module>
    loop_forever()
  File "/home/pi/nRF52_Mesh/raspi/rf_uart/mesh_controller.py", line 142, in loop_forever
    mesh.run()
  File "/home/pi/nRF52_Mesh/raspi/rf_uart/mesh.py", line 220, in run
    ser.run()
  File "/home/pi/nRF52_Mesh/raspi/rf_uart/rf_uart.py", line 10, in run
    line = ser.readline().decode("utf-8")
  File "/usr/lib/python2.7/dist-packages/serial/serialposix.py", line 490, in read
    'device reports readiness to read but returned no data '
serial.serialutil.SerialException: device reports readiness to read but returned no data (device disconnected or multiple access on port?)
1592707012: Socket error on client hci_client_mano, disconnecting.
[2020-06-21 04:39:58] Retrying ...
bed_light_button> taken
[2020-06-21 04:43:10] Failed to retrieve data from Mi Flora sensor "Flora1" (C4:7C:8D:6A:B2:9D), success rate: 0%
```
