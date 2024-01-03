HEADERS = {
  "pfc": {"telemetry": "rtu_hpr_pfc_telemetry",
          "status": "rtu_hpr_pfc_status",
          "info": "rtu_hpr_pfc_info",
          "error": "rtu_hpr_pfc_error",
  },
  "bfc": {"telemetry": "rtu_hpr_bfc_telemetry",
          "status": "rtu_hpr_bfc_status",
          "info": "rtu_hpr_bfc_info",
          "error": "rtu_hpr_bfc_error",
  },

  "rotator": {"coordinates": "rtu_hpr_rotator_coordinates",
              "angles": "rtu_hpr_rotator_angles"
              },
  "software": {"angles": "rtu_hpr_base_station_angles",
               "coordinates": "rtu_hpr_base_station_coordinates"
              }
}

HEADER_TO_APID = {
  # PFC
  HEADERS["pfc"]["telemetry"]: 100,
  HEADERS["pfc"]["status"]: 101,
  HEADERS["pfc"]["info"]: 102,
  HEADERS["pfc"]["error"]: 103,
  # BFC
  HEADERS["bfc"]["telemetry"]: 200,
  HEADERS["bfc"]["status"]: 201,
  HEADERS["bfc"]["info"]: 202,
  HEADERS["bfc"]["error"]: 203,
}

# YAMCS Received message packet id and message prefixes
# All telecommands have the same APID, but have different packet ids
TELECOMMAND_APID = {
  "pfc": 110,
  "bfc": 210,
}

PACKETID_TO_HEADER = {
  # PFC
  1000: "rtu_hpr_pfc_status_request",
  1001: "rtu_hpr_pfc_data_request",
  1002: "rtu_hpr_pfc_ranging_request",
  1003: "rtu_hpr_pfc_mosfet_1_request",
  1004: "rtu_hpr_pfc_mosfet_2_request",
  # BFC
  2000: "rtu_hpr_bfc_status_request",
  2001: "rtu_hpr_bfc_data_request",
}

YAMCS_TM_ADDRESS = ('localhost', 10015)
YAMCS_TC_ADDRESS = ('localhost', 10025)
# TRANSCEIVER_TM_ADDRESS = ('192.168.251.232', 10035)
# TRANSCEIVER_TC_ADDRESS = ('192.168.251.61', 10045)

# Find your local IPv4 address with ipconfig (Windows)
# Each network has its own IP address
# TODO: Make this automatic
TRANSCEIVER_TM_ADDRESS = ('192.168.1.51', 10035)
TRANSCEIVER_TC_ADDRESS = ('192.168.1.51', 10045)

# Message structures
REQUIRED_MESSAGE_FIELDS = ["header", "packet_id", "time"]

TELEMETRY_MESSAGE_STRUCTURE = {
  "pfc": REQUIRED_MESSAGE_FIELDS + ["gps_latitude",
                                    "gps_longitude",
                                    "gps_altitude",
                                    "outside_temperature",
                                    "gps_satellites",
                                    "outside_pressure",
                                    "gps_speed",
                                    "baro_altitude",
                                    "time_on"],
  "bfc": REQUIRED_MESSAGE_FIELDS + ["gps_latitude",
                                    "gps_longitude",
                                    "gps_altitude",
                                    "outside_temperature",
                                    "gps_satellites",
                                    "outside_pressure",
                                    "gps_speed",
                                    "baro_altitude",
                                    "time_on"],
}

# Don't add any extra fields to these
STATUS_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["status"]
INFO_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["info"]
ERROR_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["error"]