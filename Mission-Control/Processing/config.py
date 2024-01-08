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
  # PFC
  100: "pfc_essential",
  101: "pfc_full",
  102: "pfc_info_error",
  103: "pfc_format",
  104: "pfc_heater",
  105: "pfc_ejection",
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
  "rotator": 30
}

# Command packet ids
PACKETID_TO_TYPE = {
  # PFC
  1000: "pfc_complete_data_request",
  1001: "pfc_info_error_request",
  1002: "pfc_format_storage_request", 
  1003: "pfc_heater_set_mode_request",
  1004: "pfc_ejection_request",
  # BFC
  2000: "bfc_complete_data_request",
  2001: "bfc_info_error_request",
  2002: "bfc_format_storage_request",
  2003: "bfc_rwc_set_mode_request",
  2004: "bfc_ejection_request",
  # Rotator
  3000: "rotator_target_pfc_request",
  3001: "rotator_target_bfc_request",
  3002: "rotator_auto_tracking_request",
  3003: "rotator_auto_rotator_position_request",
  3004: "rotator_manual_rotator_position_request",
  3005: "rotator_manual_angles_request",
  3006: "rotator_manual_target_coordinates_request",
}

# Message structures
TELEMETRY_MESSAGE_STRUCTURE = {
  "pfc": ["callsign",
          "apid",
          "index",
          "gps_latitude",
          "gps_longitude",
          "gps_altitude",
          "baro_altitude",
          "gps_satellites",
          "time_since_last_gps_fix",
          "time_based_on_gps",
          "info_error_in_queue",
          "rssi",
          "snr"
          ],
  "bfc": ["callsign",
          "apid",
          "index",
          "gps_latitude",
          "gps_longitude",
          "gps_altitude",
          "baro_altitude",
          "gps_satellites",
          "time_since_last_gps_fix",
          "time_based_on_gps",
          "info_error_in_queue",
          "rssi",
          "snr"
          ],
  "rotator": ["callsign",
              "apid",
              "index", 
              "latitude",
              "longitude",
              "altitude"]
}

# CONNECTIONS
YAMCS_TM_ADDRESS = ('localhost', 10015)
YAMCS_TC_ADDRESS = ('localhost', 10025)
TRANSCEIVER_TM_ADDRESS = (str(local_ip_address), 10035)
TRANSCEIVER_TC_ADDRESS = ('192.168.251.159', 10045) # Rotator heltec
# TRANSCEIVER_TC_ADDRESS = ('192.168.251.61', 10045) # Some other heltec

CYCLE_TIME = 24 # Seconds
MAP_SERVER_PORT = 8080