import serial
from datetime import datetime, timedelta
from time import sleep
import csv
import folium
from flask import Flask, render_template
from flask_socketio import SocketIO
import threading
from threading import Lock
import pandas as pd
import math
import numpy as np

from SondeHubPredictor import GetEstimatedTrajectory

app = Flask(__name__)
app.config["SECRET_KEY"] = "secret!"
app.config["TEMPLATES_AUTO_RELOAD"] = True
socketio = SocketIO(app)

# Define the serial port and baud rate
serial_port = "COM14"
baud_rate = 115200
thread_running = False

# csv file name
file_name = "ROTATOR_DATA_"

data_ser = ""
callsign = ""
id = ""
time = ""
gps_lat = ""
gps_lng = ""
gps_alt = ""
temperature = ""
satellites = ""
pressure = ""
speed = ""
baro_alt = ""
rotator_lat = ""
rotator_lng = ""
rotator_alt = ""
rssi = ""
snr = ""
distance = ""

m = None

lock = Lock()
# Balloon flight coordinates
balloon_coordinates = []

# Requests
data_request = False
ping_request = False
detach_request = False

custom_angle_request = False
custom_azimuth = 0
custom_elevation = 0

custom_coordinate_request = False
custom_latitude = 0
custom_longitude = 0
custom_altitude = 0

data_send_off_request = False
data_send_on_request = False
buzzer_on_request = False
buzzer_off_request = False

# API
# Last API request time
last_api_request_time = datetime.now()

# Was a prediction requested?
prediction_request = False

# Prediction results
prediction_results = []

def haversine_np(lon1, lat1, lon2, lat2):
    """
    Calculate the great circle distance between two points
    on the earth (specified in decimal degrees)
    
    All args must be of equal length.    
    
    """
    lon1, lat1, lon2, lat2 = map(np.radians, [lon1, lat1, lon2, lat2])
    
    dlon = lon2 - lon1
    dlat = lat2 - lat1
    
    a = np.sin(dlat/2.0)**2 + np.cos(lat1) * np.cos(lat2) * np.sin(dlon/2.0)**2
    
    c = 2 * np.arcsin(np.sqrt(a))
    km = 6378.137 * c
    return km

@app.route("/")
def index():
    return render_template("home.html")


@app.route("/map")
def showMap():
    return render_template("map.html")


@socketio.on("prediction_request")
def handle_prediction_request():
    global prediction_request, last_api_request_time
    seconds = (datetime.now() - last_api_request_time).total_seconds()

    if not prediction_request and seconds > 1:
        lock.acquire()
        print("A prediction has been requested!")
        prediction_request = True
        last_api_request_time = datetime.now()
        lock.release()


@socketio.on("data_request")
def handle_data_request():
    print("A data request has been requested!")
    global data_request
    data_request = True


@socketio.on("ping_request")
def handle_ping_request():
    print("A ping request has been requested!")
    global ping_request
    ping_request = True

@socketio.on("data_send_off_request")
def handle_data_send_off_request():
    print("A data send off request has been requested!")
    global data_send_off_request
    data_send_off_request = True
    
@socketio.on("data_send_on_request")
def handle_data_send_on_request():
    print("A data send on request has been requested!")
    global data_send_on_request
    data_send_on_request = True
    
@socketio.on("buzzer_off_request")
def handle_buzzer_off_request():
    print("A buzzer off request has been requested!")
    global buzzer_on_request 
    buzzer_on_request = True
    
@socketio.on("buzzer_on_request")
def handle_buzzer_on_request():
    print("A buzzer off request has been requested!")
    global buzzer_off_request
    buzzer_off_request = True

@socketio.on("detach_request")
def handle_detach_request():
    print("A detach request has been requested!")
    global detach_request
    detach_request = True


@socketio.on("custom_angles")
def handle_custom_angle(azimuth, elevation):
    print("Custom angles have been requested!")
    global custom_angle_request, custom_azimuth, custom_elevation
    custom_azimuth = float(azimuth)
    custom_elevation = float(elevation)
    custom_angle_request = True
    
    
@socketio.on("custom_coordinates")
def handle_custom_coordinates(latitude, longitude, altitude):
    print("Custom coordinates have been requested!")
    global custom_coordinate_request, custom_latitude, custom_longitude, custom_altitude
    custom_latitude = float(latitude)
    custom_longitude = float(longitude)
    custom_altitude = float(altitude)
    custom_coordinate_request = True
    

@socketio.on("get_data")
def handle_get_data():
    try:
        distance = str(haversine_np(round(float(gps_lng), 6), round(float(gps_lat), 6), round(float(rotator_lng), 6), round(float(rotator_lat), 6)))
    except:
        distance = "Not yet calculated"
    socketio.emit(
        "send_data",
        (gps_lat, gps_lng, gps_alt, baro_alt, speed, temperature, rssi, snr, data_ser, distance),
    )


# Open a new log file
i = 1
while True:
    # Find a file index that does not yet exist
    try:
        file = open(file_name + str(i) + ".csv", "r")
        file.close()
        i += 1
    except:
        # Save the file name to be used
        file_name = file_name + str(i) + ".csv"
        # Write header for csv file
        with open(file_name, "a", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(
                [
                    "callsign",
                    "id",
                    "time",
                    "gps_lat",
                    "gps_lng",
                    "gps_alt",
                    "temperature",
                    "satellites",
                    "pressure",
                    "speed",
                    "baro_alt",
                    "rotator_lat",
                    "rotator_lng",
                    "rotator_alt",
                    "rssi",
                    "snr",
                ]
            )
        break


def read_serial_data():
    global balloon_coordinates, prediction_request, last_api_request_time, prediction_results, m, data_request, ping_request, detach_request, data_send_on_request, data_send_off_request, buzzer_on_request, buzzer_off_request, custom_angle_request, custom_coordinate_request
    while True:
        try:
            print(f"Connecting to {serial_port}")
            ser = serial.Serial(serial_port, baud_rate)
            while True:
                data_ser = ""
                # Read any available data
                if ser.in_waiting > 0:
                    data_ser = ser.readline().decode("utf-8").strip()
                    if "rtu_vip_bfc" in data_ser:
                        print(f"{serial_port} received message: {data_ser}")
                    else:
                        data_ser = ""

                # Send requests
                if data_request:
                    ser.write("BFC_DATA_REQUEST".encode())
                    print("Serial write done! BFC_DATA_REQUEST")
                    data_request = False
                    sleep(0.05)
                if ping_request:
                    ser.write("BFC_PING".encode())
                    print("Serial write done! BFC_PING")
                    ping_request = False
                    sleep(0.05)
                if detach_request:
                    ser.write("BFC_ATKABE".encode())
                    print("Serial write done! BFC_ATKABE")
                    detach_request = False
                    sleep(0.05)
                if data_send_on_request:
                    ser.write("BFC_DATA_SEND_ON".encode())
                    print("Serial write done! BFC_DATA_SEND_ON")
                    data_send_on_request = False
                    sleep(0.05)
                if data_send_off_request:
                    ser.write("BFC_DATA_SEND_OFF".encode())
                    print("Serial write done! BFC_DATA_SEND_OFF")
                    data_send_off_request = False
                    sleep(0.05)
                if buzzer_on_request:
                    ser.write("BFC_BUZZER_ON".encode())
                    print("Serial write done! BFC_BUZZER_ON")
                    buzzer_on_request = False
                    sleep(0.05)
                if buzzer_off_request:
                    ser.write("BFC_BUZZER_OFF".encode())
                    print("Serial write done! BFC_BUZZER_OFF")
                    buzzer_off_request = False
                    sleep(0.05)
                
                if custom_angle_request:
                    msg = "A" + str(int(custom_azimuth)) + "," + str(int(custom_elevation))
                    ser.write(msg.encode())
                    print(f"Serial write done! Custom angles - AZ: {int(custom_azimuth)} | ELEV: {int(custom_elevation)}")
                    custom_angle_request = False
                    sleep(0.05)
                if custom_coordinate_request:
                    msg = "C" + str(round(float(custom_latitude), 6)) + "," + str(round(float(custom_longitude), 6)) + "," + str(round(float(custom_altitude), 2))
                    ser.write(msg.encode())
                    print(f"Serial write done! Custom coordinates - LAT: {round(float(custom_latitude), 6)} | LNG: {round(float(custom_latitude), 6)} | ALT: {round(float(custom_altitude), 2)}")
                    custom_coordinate_request = False
                    sleep(0.05)

                # data_ser = "rtu_vip_bfc,3,18:04:43,55.123456,24.987654,123.45,25.67,10,123000,10.5,123.45,15.123456,14.987654,20.52,-50,-2"
                # print(data_ser)
                if data_ser:
                    data = data_ser.split(",")
                    # Split ukhas message + rotator gps position to variables
                    try:
                        (
                            callsign,
                            id,
                            time,
                            gps_lat,
                            gps_lng,
                            gps_alt,
                            temperature,
                            satellites,
                            pressure,
                            speed,
                            baro_alt,
                            rotator_lat,
                            rotator_lng,
                            rotator_alt,
                            rssi,
                            snr,
                        ) = data
                    except:
                        print("Failed spliting message to variables")
                        continue

                    gps_lat = float(gps_lat)
                    gps_lng = float(gps_lng)
                    gps_alt = float(gps_alt)

                    # Create a map with data
                    if gps_lat != 0 and gps_lng != 0:
                        balloon_coordinates.append((gps_lat, gps_lng))

                    # If no gps coordinates yet available, default to Cēsis airfield
                    if balloon_coordinates[-1][0] == 0:
                        # Create a map
                        m = folium.Map((57.320578, 25.321794), zoom_start=10)

                        # Start position
                        folium.Marker(
                            location=(57.320578, 25.321794),
                            tooltip="Starting position",
                            icon=folium.Icon(color="green"),
                        ).add_to(m)

                        # Save the map
                        m.save("templates/map.html")
                        socketio.emit("update_map")
                        sleep(0.1)

                    else:
                        # Create a map
                        m = folium.Map(balloon_coordinates[-1])
                        # Start position
                        folium.Marker(
                            location=balloon_coordinates[0],
                            popup=f"{balloon_coordinates[-1], balloon_coordinates[-1]}",
                            tooltip="Starting position",
                            icon=folium.Icon(color="green"),
                        ).add_to(m)

                        # Last balloon position
                        folium.Marker(
                            location=balloon_coordinates[-1],
                            popup=f"{balloon_coordinates[-1], balloon_coordinates[-1]}",
                            tooltip="Last balloon position",
                            icon=folium.Icon(color="blue"),
                        ).add_to(m)

                        # Balloon trajectory
                        folium.PolyLine(
                            locations=balloon_coordinates,
                            color="#0048ff",
                            weight=5,
                            tooltip="Balloon trajectory",
                        ).add_to(m)

                        # If a prediction was requested and cooldown time has passed
                        seconds = (
                            datetime.now() - last_api_request_time
                        ).total_seconds()
                        if prediction_request and seconds > 1:
                            prediction_request = False
                            last_api_request_time = datetime.now()
                            if balloon_coordinates[-1][0] != 0:
                                prediction_results = GetEstimatedTrajectory(
                                    gps_lat,
                                    gps_lng,
                                    gps_alt,
                                    ascent_rate=6,
                                    burst_altitude=gps_alt + 10,
                                    descent_rate=6,
                                )
                                print(prediction_results)
                                print("Prediction received successfully!")

                        # If prediction result isn't empty, add it to the map
                        if prediction_results:
                            # Prediction start position
                            folium.Marker(
                                location=(
                                    prediction_results[0][-2],
                                    prediction_results[0][-1],
                                ),
                                tooltip="Prediction burst position",
                                popup=f"{prediction_results[0][-2], prediction_results[0][-1]}",
                                icon=folium.Icon(color="red"),
                            ).add_to(m)

                            # Last Prediction position
                            folium.Marker(
                                location=(
                                    prediction_results[-1][-2],
                                    prediction_results[-1][-1],
                                ),
                                popup=f"{prediction_results[-1][-2], prediction_results[-1][-1]}",
                                tooltip="Prediction landing position",
                                icon=folium.Icon(color="blue"),
                            ).add_to(m)

                            # Prediction trajectory
                            folium.PolyLine(
                                locations=[[x[-2], x[-1]] for x in prediction_results],
                                color="#FF0000",
                                weight=3,
                                tooltip="Prediction trajectory",
                            ).add_to(m)

                        # Calculate most extreme south west and north east coordinates
                        all_coordinates = balloon_coordinates + [
                            [x[-2], x[-1]] for x in prediction_results
                        ]
                        df = pd.DataFrame(all_coordinates, columns=["Lat", "Long"])
                        sw = df[["Lat", "Long"]].min().values.tolist()
                        ne = df[["Lat", "Long"]].max().values.tolist()

                        m.fit_bounds([sw, ne])

                        # Save the map
                        m.save("templates/map.html")
                        socketio.emit("update_map")

                    try:
                        distance = str(round(haversine_np(round(float(gps_lng), 6), round(float(gps_lat), 6), round(float(rotator_lng), 6), round(float(rotator_lat), 6)), 4))
                    except:
                        distance = "Not yet calculated"
                    socketio.emit(
                        "send_data",
                        (
                            gps_lat,
                            gps_lng,
                            gps_alt,
                            baro_alt,
                            speed,
                            temperature,
                            rssi,
                            snr,
                            data_ser,
                            distance
                        ),
                    )

                    # Log message to file
                    with open(file_name, "a", newline="") as f:
                        # print("Saving to file")
                        writer = csv.writer(f)
                        writer.writerow(data)

        except serial.SerialException as err:
            print(f"Connection failed with error: {err.args[0]}")
            print("Reconnecting...")
            sleep(1)
        except Exception as err:
            print(f"Something went wrong{err.args}")


# main socketio thread
if __name__ == "__main__":
    if not thread_running:
        # Start the serial thread
        serial_thread = threading.Thread(target=read_serial_data, daemon=True)
        serial_thread.start()
        thread_running = True

    # DEBUG MUST BE FALSE
    socketio.run(app, debug=False)
