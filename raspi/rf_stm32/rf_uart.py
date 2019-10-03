import serial
import binascii

ser = serial.Serial()
on_line_function = None

def run():
    res = None
    try:
        line = ser.readline().decode("utf-8", errors='ignore')
        if(len(line)):
            line = line.replace('\r','')
            line = line.replace('\n','')
            on_line_function(line)
    except OSError as e:
        print("Handled exception: %s",str(e))
    #TODO handle UnicodeDecodeError error
    return res

def send(data):
    ser.write(data.encode())
    return

def serial_start(config,serial_on_line):
    global on_line_function
    on_line_function = serial_on_line
    global ser
    ser = serial.Serial(config["serial"]["port"],
                        config["serial"]["baud"],
                        timeout=0.1)
    print("uart> %s"%(ser.name))
    return ser
