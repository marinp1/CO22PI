#!/usr/bin/env python3
import serial
import json

latest_measurement = None

if __name__ == '__main__':
    ser = serial.Serial('/dev/tty.usbserial-1110', 9600, timeout=1)
    ser.reset_input_buffer()
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            # 12,4/14/2023,16:24:25,1265,26.0,23.5
            splitted = line.split(',')
            if (len(splitted) != 6):
                print(line)
            else:
                latest_measurement = {
                    "date": splitted[1],
                    "time": splitted[2],
                    "co2": splitted[3],
                    "temperature": splitted[4],
                    "humidity": splitted[5]
                }

                with open("web/latest.json", "w") as outfile:
                    json.dump(latest_measurement, outfile)
