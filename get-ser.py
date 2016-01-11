import serial 
import time 

ser = serial.Serial()
ser.port = "/dev/ttyUSB0"
ser.baudrate = 1200
ser.timeout = 0 

try: 
    ser.open()
    print "serial port is opened"

    str_list = []
    while True: 
        time.sleep(0.002)
        next_char = ser.read(ser.inWaiting())
        if next_char: 
            str_list.append(next_char)
        else: 
            if len(str_list) > 7:
				if int('0x'+str_list.pop() , 16) == 1 and int('0x'+str_list[5] , 16) == 4
					data = str_list[0:4];
					data_efe = ":".join("{:02x}".format(ord(c)) for c in data)
					print data_efe
					
except Exception as e :
    print "Exception: ", e.message 
    ser.close()

