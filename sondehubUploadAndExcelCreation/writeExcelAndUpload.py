#DISCLAIMER - ANYONE RUNNING THIS NEEDS TO SET THEIR OWN PATHS FOR THINGS
#THESE ONES ONLY WORK ON MY PC

#all imports for everything
# Sondehub lib that we'll be using to download and access the data
from sondehub.amateur import Uploader
#libaries needed for working with data
import pandas as pd
import time
import datetime
#libraries for excel and serial communications
import serial
import csv
import random
#so these are preset coordinates
longitude = 56.946285
latitude = 24.105078
altitude = 100
t = 3
#fieldnames for excel
fieldnames = ["longitude", "latitude", "altitude"]

arduino_port = "COM4" #serial port of Arduino
baud = 115200#115200 #arduino uno runs at 9600 baud
fileName="data.csv" #name of the CSV file generated

ser = serial.Serial(arduino_port, baud)
ser.flushInput()
print("Connected to Arduino port:" + arduino_port)


#this here creates up a csv file and writes it's fieldnames. cool.
with open('C:/Users/user/Downloads/sondehub_jupyter/data.csv', 'w') as csv_file:
    csv_writer = csv.DictWriter(csv_file,fieldnames=fieldnames, delimiter=",")
    csv_writer.writeheader()
    print("Created file")
#the following sets up sondehub balloon.
uploader = Uploader("NOT_MYCALL")

uploader = Uploader(
    "NOT_MYCALL",
    uploader_position=[56.946285, 24.105078, 100], # [latitude, longitude, altitude]
    uploader_radio="My Radio Information", # Radio information - Optional
    uploader_antenna="My antenna Information" # Antenna information - Optional
)

uploader.upload_station_position(
    "NOT_MYCALL",
    [56.946285, 24.105078, 100], # [latitude, longitude, altitude]
)

x = datetime.datetime.utcnow()
    
time_now=str(x)[0:10]+"T"+x.strftime("%X")+"Z" 
#time_now=str(x)[0:10]+" "+x.strftime("%X")
# year month days hours minutes miliseconds

uploader.add_telemetry(
    "VIP-ROCKOON", # Your payload callsign
    time_now, #"2022-05-28T00:07:05Z", # Time 
    longitude,
    latitude,
    altitude,
)
time.sleep(t)
#this bit fills in data    
while True:

    #opens file in append mode and sets parameters for the writer
    with open('C:/Users/user/Downloads/sondehub_jupyter/data.csv', 'a') as csv_file:
        csv_writer = csv.writer(csv_file, delimiter=",")
        #tells which data goes to which colummn
        getData=ser.readline()
        data = str(getData[0:len(getData)-2].decode("utf-8"))
        #writes the stuff
        csv_writer.writerow(data.split(","))
        print(data)
        time.sleep(1)
    #uploads telemetry to sondehub, same telemetry that is being saved to excel.

#delay between updates. currently set to one second so as not to spam the serial(hurts to look at)

    data = pd.read_csv('c:/Users/user/Downloads/sondehub_jupyter/data.csv')
    last_value = data.iloc[-1] 
    uploader.add_telemetry(
        "VIP-ROCKOON", # Your payload callsign
        time_now, #"2022-05-28T00:07:05Z", # Time 
        float(last_value['longitude']), 
        float(last_value['latitude']), 
        int(last_value['altitude']),
    )
    time.sleep(t)
