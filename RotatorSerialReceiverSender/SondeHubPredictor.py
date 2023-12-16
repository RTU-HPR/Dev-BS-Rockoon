import requests
from datetime import datetime, timezone
import time

# Get current time
local_time = datetime.now(timezone.utc).astimezone()
print(local_time.isoformat())


# Function that sends request to the Tawhiri API
def SendPredictionRequest(
    profile,
    launch_datetime,
    launch_latitude,
    launch_longitude,
    launch_altitude=None,
    ascent_rate=None,
    burst_altitude=None,
    descent_rate=None,
):
    API_url = "https://api.v2.sondehub.org/tawhiri"

    # Construct for request parameters
    params = {
        "profile": profile,  # For descent simulation standard_profile must be used
        "launch_datetime": launch_datetime,  # Time of launch in RFC3339 timestamp format
        "launch_latitude": launch_latitude,  # Launch latitude
        "launch_longitude": launch_longitude,  # Launch longitude
        "launch_altitude": launch_altitude,  # Launch altitude
        "ascent_rate": ascent_rate,
        "burst_altitude": burst_altitude,  # Altitude at which baloon bursts
        "descent_rate": descent_rate,
    }

    prediction_points = []

    # Send the GET request
    response = requests.get(API_url, params=params)
    # Check if the response status code is 200 (OK)
    if response.status_code == 200:
        # Parse and the prediction
        prediction_fragment = response.json().get("prediction", [])

        for stage in prediction_fragment:
            for point in stage.get("trajectory", []):
                altitude = point.get("altitude", "")
                datetime = point.get("datetime", "")
                latitude = point.get("latitude", "")
                longitude = point.get("longitude", "")
                prediction_points.append([altitude, datetime, latitude, longitude])
    else:
        # Print the error message if the status code is not 200
        print(f"Error: {response.status_code} - {response.text}")

    return prediction_points


# Function that displays estimated trajectory on Google Maps
def GetEstimatedTrajectory(
    LaunchLatitude,
    LaunchLongitude,
    LaunchAltitude,
    ascent_rate,
    burst_altitude,
    descent_rate,
):
    return SendPredictionRequest(
        "standard_profile",
        local_time.isoformat(),
        LaunchLatitude,
        LaunchLongitude,
        launch_altitude=LaunchAltitude,
        ascent_rate=ascent_rate,
        burst_altitude=burst_altitude,
        descent_rate=descent_rate,
    )


"""
If want to simulate falling from specific altitude and not burst altitude, set launch_altitude to the current altitude
and add set burst_altitude the launch_altitude + some random constant (for example 10 meters) so that the API request 
can work properly.
Also set launch_datetime, launch_latitude, launch_longitude accordingly to the current data

When implimenting with actualy sensor readings, time.sleep() function must be used, to not spam API requests
"""
# Some random data for testing

# LaunchLatitude = 57.32794638705446
# LaunchLongitude = 25.321676805013627
# LaunchAltitude = 0
# AscentRate = 5.0  # meters per second
# BurstAltitude = 27001
# DescentRate = 10.0  # meters per second

# GetEstimatedTrajectory(
#     LaunchLatitude,
#     LaunchLongitude,
#     LaunchAltitude,
#     AscentRate,
#     BurstAltitude,
#     DescentRate,
# )
