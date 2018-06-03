# What you'll find [here](https://github.com/nRFMesh/nRF52_Mesh)
* nRF52 boards schematics and applications using the nRF SDK 15
    * Low power Custom Sensor Tag with BME280 and MAX44009 (currently @22 uA)
    * Reprogrammed USB dongle from the market with 3d printed pogo-pins adapter (status alive)
* A light weight Mesh Protocol connecting all the devices using a custom RF protocol (without softdevice)
    * Sleepy nodes (low power) and router nodes (always listening)
    * single layer ultra simple rpotocol. App into mac with unique ids to small ids mapping
    * An alternative to the Bluetooth Mesh and Zigbee Thread IPV6 world
* Tools and environemt
    * devices database management and automated parameters flashing
    * [HomeSmartMesh](https://github.com/HomeSmartMesh/IoT_Frameworks) compatible environment where a raspberry pi collects the data into a time series [influx](https://www.influxdata.com/time-series-database/) Data base and serve a [Graphana](https://grafana.com/grafana) web interface

# Installation
* GNU Tools ARM Embedded version 6 2017-q2-update, referenced from the SDK Makefile.windows
* Python [jlink wrapper](https://github.com/square/pylink). Used in Makefile and tools of this repo, more on [pylink.readthedocs](http://pylink.readthedocs.io/en/latest/pylink.html). Note that it has to be used with a 32 bit python version referenced in the tools scripts as **C:\Python27\python.exe**. wrapper scripts already availabe in the tools directory

<img src="images/pylink.png" width="360">

* System variable **NODES_CONFIG** should point on a json configuration file, see an example at [node.json](https://raw.githubusercontent.com/HomeSmartMesh/IoT_Frameworks/master/config/nodes.json) or the screenshot below

<img src="images/nodes.json.png" width="400">

The parameters here are used by external tools to have a consistent undersanding of the mesh network. From simple id to name translation, to location display according to coordinates down to user data flashing.

## Cool defines and data configuration
### sdk config tool
Once in the application directory just use ```make conf``` to call a cmsis [configuration wizard](http://helmpcb.com/software/cmsis-configuration-wizard), as provided in the SDK. Note that it was here extended to make the user drivers shared and configurable as well, e.g. the I²C frequency of the **Application** in the screenshot

<img src="images/cmsis-wizard.png" width="400">

### Automated mesh devices configuration
 User data flashing is done with Pylink which reads in [uicr.py](tools\uicr.py) the registers of the attached device, look it up in the **NODES_CONFIG** file, retrives which parameters should be flashed, the mapping of parameters to CUSTOMER_X registers come from "uicr_map.json".

## simple board switch
The repo contais a directory for boards declaration "boards/" and a directory for applications "applications/". Although every application targets a particular board, it is possible to use any application for any other board. In the makefile a sinlge line has to be edited

    USED_BOARD := BOARD_NRF52_SENSOR_TAG

# Doxygen Documentation
* install [Doxygen](www.doxygen.org/)
* run

    gen_doc.bat

Yes this is only wrapping ```doxygen Doxyfile``` but just take it as a workspace local alias
# nRF52 Sensor Tag
This board is based on modules, it is very simple to solder and allows selection of any other I²C sensor modules.

## Design
### Module
<img src="boards/nrf52_sensortag/images/module_size.png" width="300">

### Schematics
Here a screenshot of the schematics which design files are also available in the [board subdirectory](boards/nrf52_sensortag/pcb)

<img src="boards/nrf52_sensortag/images/schematics_pinout.png" width="400">

## Mounted PCBs
It is possible to mount either CR2032 or CR2477

<img src="boards/nrf52_sensortag/images/mounted.png" width="600">

It is based on a market available nRF52832 module seen below


## Low Power configuration
|Flags to clear|
--- |
| NRFX_UARTE_ENABLED |
| NRFX_UART_ENABLED | 
| UART_ENABLED | 
| UART0_ENABLED |
| NRF_FPRINTF_ENABLED |
| NRF_LOG_BACKEND_UART_ENABLED |
| NRF_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED |
| NRF_LOG_ENABLED |
* removed nrf_drv_uart.c from Makefile
* Required nRF52832 Errata [89] TWI: Static 400 uA current while using GPIOTE


## Low Power measures
| Mode | Current |
--- | --- |
| RTC + RAM | 9.6 uA |
| // + Sensors | 22 uA |
| // without TWI Woraround | 470 uA |
| Uart Log | 500 uA |
| Uart Log + HF | 700 uA |


# nRF52 Dongle
## Software
### RF to Serial Gateway
The node connected through serial port will report messages in text format
1. Local Events

Upon startup such message is sent

    nodeid:75;channel:10;reset:1

2. Listening

Upon activation of the uicr config ```is_listening``` the node starts acting as a gatewy that will report messages through serial in text mode

    rssi:-50;id:73;ctrl:0x81;src:73;alive:68;tx_power:0

3. commands requests and responses

It is possible to reconfigure the Node on runtime and manage manage a mesh network by sending messages

    cmd:0x0102
    cmd:0x<command id><arguments>

| Command id | Function | Parameters |
--- | --- | --- |
| 0x01 | Set RF Channel | 0:2400 MHz - 100:2500 MHz|
| 0x02 | Get RF Channel | |
| 0x03 | Set Tx Power | 0x04: +4dBm - 0xD8: -40dBm|
| 0x04 | Get Tx Power | |
| 0x05 | Set Bitrate | tbd |
| 0x06 | Get Bitrate |
| 0x07 | Set CRC | 0 : No CRC - 1 : 8bit - 2 : 16 bit|
| 0x08 | Get CRC |  |




### Switch between serial SERIAL and LOG over UART
run

    make conf

then switch both flags :
* nRF_Log/NRF_LOG_BACKEND_UART_ENABLED
* Application/APP_SERIAL_ENABLED
don't forget to save

### Serial mode
* NRF_SERIAL_MODE_IRQ flag is not used by the driver and has equivalent function as NRF_SERIAL_MODE_DMA

* ser_send() must be used with a variable in memory as DMA cannot read from Code in Flash

## Hardware

Why reinvent the wheel ? When it comes to a Server interface as a dongle, we can reuse a usb dongle from the market that includes a **2104** serial to usb converter. Keyword search on shopping websites : **nRF52832 USB dongle**. Aka "nRF52832-YJ-17017-USB-UART"

<img src="boards/nrf52_dongle/images/dongle.png" width="200">
<br/>

## From the inside

<img src="boards/nrf52_dongle/images/components.png" width="200" title="Components">
<img src="boards/nrf52_dongle/images/back_swdio.png" width="200" title="SWDIO pins">
<br/>

## pinout
| nRF52 | pin |
--- | --- |
| Rx | P0.05 |
| Tx | P0.06 |
| CTS | P0.07 |
| RTS | P0.08 |
| LED1 | P0.28 |
| LED2 | P0.29 |


## Needle adapter
Making a needle adapter is made easier with 3d printing. The used pogo pin is seen below

<img src="boards/nrf52_dongle/images/pogo-pin 0.5mm x 16.35mm.png" width="200" title="Pogo pin">

The adapter model still in preparation can be found [here](https://a360.co/2IcKZK9). It should look something like this:

<img src="boards/nrf52_dongle/images/adapter.png" width="300" title="Adapter">
<br/>
<img src="boards/nrf52_dongle/images/pogo-pin adapter.jpg" width="300" title="Adapter">
