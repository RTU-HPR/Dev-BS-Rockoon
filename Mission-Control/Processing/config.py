# DONT CHANGE ANY APID OR PACKETID VALUES
# DONT CHANGE ANY APID OR PACKETID VALUES
# DONT CHANGE ANY APID OR PACKETID VALUES
# DONT CHANGE ANY APID OR PACKETID VALUES
# DONT CHANGE ANY APID OR PACKETID VALUES

# Get local IP address
from socket import gethostname, gethostbyname
hostname = gethostname()
local_ip_address = gethostbyname(hostname)

# APID can be from 0 to 255
# Telemetry package apids
APID_TO_TYPE = {
  # Rotator
  50: "rotator_position",
  # Calculations
  60: "pfc_calculations",
  70: "bfc_calculations",
  80: "rotator_calculations",
  # PFC
  100: "pfc_essential",
  101: "pfc_full",
  102: "pfc_info_error",
  103: "pfc_format",
  104: "pfc_ejection",
  # BFC
  200: "bfc_essential",
  201: "bfc_full",
  202: "pfc_info_error",
  203: "bfc_format",
  204: "bfc_rwc_mode",
  205: "bfc_ejection",  
}

# All telecommands have the same APID, but have different packet ids
TELECOMMAND_APID = {
  "pfc": 10,
  "bfc": 20,
  "rotator": 30,
}

# Command packet ids
PACKETID_TO_TYPE = {
  # PFC
  1000: "pfc_complete_data_request",
  1001: "pfc_info_error_request",
  1002: "pfc_format_storage_request", 
  1003: "pfc_ejection_request",
  # BFC
  2000: "bfc_complete_data_request",
  2001: "bfc_info_error_request",
  2002: "bfc_format_storage_request",
  2003: "bfc_rwc_set_mode_request",
  2004: "bfc_ejection_request",
  # Rotator
  3000: "rotator_set_target_request",
  3001: "rotator_auto_tracking_request",
  3002: "rotator_auto_rotator_position_request",
  3003: "rotator_manual_rotator_position_request",
  3004: "rotator_manual_angles_request",
  3005: "rotator_manual_target_coordinates_request",
  # Non yamcs
  3100: "rotator_angles_request",
}

# Message structures
TELEMETRY_MESSAGE_STRUCTURE = {
  "pfc": [("float", "gps_latitude"),
          ("float", "gps_longitude"),
          ("float", "gps_altitude"),
          ("float", "baro_altitude"),
          ("int", "gps_satellites"),
          ("int", "info_error_in_queue"),
          ("float", "rssi"),
          ("float", "snr")],
  
  "bfc": [("float", "gps_latitude"),
          ("float", "gps_longitude"),
          ("float", "gps_altitude"),
          ("float", "baro_altitude"),
          ("int", "gps_satellites"),
          ("int", "info_error_in_queue"),
          ("float", "rssi"),
          ("float", "snr")],
  
  "rotator": [("float", "latitude"),
              ("float", "longitude"),
              ("float", "altitude"),]
}

CALCULATION_MESSAGE_STRUCTURE = {
  "pfc": ["gps_vertical_speed",
          "baro_vertical_speed",
          "horizontal_speed",
          "gps_total_speed",
          "ground_distance_to_rotator",
          "straight_line_distance_to_rotator"],
  "bfc": ["gps_vertical_speed",
          "baro_vertical_speed",
          "horizontal_speed",
          "gps_total_speed",
          "ground_distance_to_rotator",
          "straight_line_distance_to_rotator"],
  "rotator": ["calculated_azimuth"
              "calculated_elevation"]         
}

# CONNECTIONS
YAMCS_TM_ADDRESS = ('localhost', 10015)
YAMCS_TC_ADDRESS = ('localhost', 10025)
TRANSCEIVER_TM_ADDRESS = (str(local_ip_address), 10035)
TRANSCEIVER_TC_ADDRESS = ('192.168.236.61', 10045) # Some random heltec
#TRANSCEIVER_TC_ADDRESS = ('192.168.236.159', 10045) # Heltec inside rotator box
MAP_SERVER_PORT = 9500

CYCLE_TIME = 12 # Seconds