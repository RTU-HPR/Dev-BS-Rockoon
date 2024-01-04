RECEIVE_HEADERS = {
  "pfc": {"telemetry": "rtu_hpr_pfc_telemetry",
          "status": "rtu_hpr_pfc_status",
          "info": "rtu_hpr_pfc_info",
          "error": "rtu_hpr_pfc_error",
          "ack": "rtu_hpr_pfc_ack",
  },
  "bfc": {"telemetry": "rtu_hpr_bfc_telemetry",
          "status": "rtu_hpr_bfc_status",
          "info": "rtu_hpr_bfc_info",
          "error": "rtu_hpr_bfc_error",
          "ack": "rtu_hpr_bfc_ack",
  },
  "rotator": {"position": "rtu_hpr_rotator_position"}
}

REQUEST_HEADERS = {
  "pfc": {"status": "rtu_hpr_pfc_status_request",
          "data": "rtu_hpr_pfc_data_request",
          "mosfet_1": "rtu_hpr_pfc_mosfet_1_request",
          "mosfet_2": "rtu_hpr_pfc_mosfet_2_request",
  },
  "bfc": {"status": "rtu_hpr_bfc_status_request",
          "data": "rtu_hpr_bfc_data_request",
  },
  "rotator": {"target_pfc": "rtu_hpr_rotator_target_pfc_request",
              "target_bfc": "rtu_hpr_rotator_target_bfc_request",
              "rotator_auto": "rtu_hpr_rotator_auto_request",
              "rotator_auto_rotator_position": "rtu_hpr_rotator_auto_rotator_position_request",
              "rotator_manual_rotator_position": "rtu_hpr_rotator_manual_rotator_position_request",
              "rotator_manual_angles": "rtu_hpr_rotator_manual_angles_request",
              "rotator_manual_target_coordinates": "rtu_hpr_rotator_manual_coordinates_request",
  },
}

# APID can be from 0 to 255
# Telemetry package apids
APID_TO_TYPE = {
  # Rotator
  50: "rotator_position",
  # PFC
  100: "pfc_essential",
  101: "pfc_full",
  102: "pfc_info",
  103: "pfc_error",
  104: "pfc_format",
  105: "pfc_ejection_1",
  106: "pfc_ejection_2",
  # BFC
  200: "bfc_essential",
  201: "bfc_full",
  202: "bfc_info",
  203: "bfc_error",
  204: "bfc_format",
  205: "bfc_rwc_mode",
  206: "bfc_ejection_1",
  207: "bfc_ejection_2",  
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
  1001: "pfc_get_info_error",
  1002: "pfc_format_storage_request", 
  1003: "pfc_heater_set_mode_request",
  1004: "pfc_ejection_request",
  # BFC
  2000: "bfc_complete_data_request",
  2001: "bfc_get_info_error",
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



HEADER_TO_APID = {
  # PFC
  RECEIVE_HEADERS["pfc"]["telemetry"]: 100,
  RECEIVE_HEADERS["pfc"]["status"]: 101,
  RECEIVE_HEADERS["pfc"]["info"]: 102,
  RECEIVE_HEADERS["pfc"]["error"]: 103,
  RECEIVE_HEADERS["pfc"]["ack"]: 104,
  # BFC
  RECEIVE_HEADERS["bfc"]["telemetry"]: 200,
  RECEIVE_HEADERS["bfc"]["status"]: 201,
  RECEIVE_HEADERS["bfc"]["info"]: 202,
  RECEIVE_HEADERS["bfc"]["error"]: 203,
  RECEIVE_HEADERS["bfc"]["ack"]: 204,
  # ROTATOR
  RECEIVE_HEADERS["rotator"]["position"]: 300,
}

PACKETID_TO_HEADER = {
  # PFC
  1000: REQUEST_HEADERS["pfc"]["status"],
  1001: REQUEST_HEADERS["pfc"]["data"],
  1002: REQUEST_HEADERS["pfc"]["mosfet_1"],
  1003: REQUEST_HEADERS["pfc"]["mosfet_2"],
  # BFC
  2000: REQUEST_HEADERS["bfc"]["status"],
  2001: REQUEST_HEADERS["bfc"]["data"],
  # SOFTWARE
  3000: REQUEST_HEADERS["software"]["target_pfc"],
  3001: REQUEST_HEADERS["software"]["target_bfc"],
  3002: REQUEST_HEADERS["software"]["rotator_auto"],
  3003: REQUEST_HEADERS["software"]["rotator_auto_rotator_position"],
  3004: REQUEST_HEADERS["software"]["rotator_manual_rotator_position"],
  3005: REQUEST_HEADERS["software"]["rotator_manual_angles"],
  3006: REQUEST_HEADERS["software"]["rotator_manual_target_coordinates"],
}


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
  "rotator": REQUIRED_MESSAGE_FIELDS + ["latitude", "longitude", "altitude"]
}

# Don't add any extra fields to these
STATUS_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["status"]
INFO_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["info"]
ERROR_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["error"]
ACK_MESSAGE_STRUCTURE = REQUIRED_MESSAGE_FIELDS + ["ack"]

# CONNECTIONS
YAMCS_TM_ADDRESS = ('localhost', 10015)
YAMCS_TC_ADDRESS = ('localhost', 10025)
# TRANSCEIVER_TM_ADDRESS = ('192.168.251.232', 10035)
# TRANSCEIVER_TC_ADDRESS = ('192.168.251.61', 10045)

# Find your local IPv4 address with ipconfig (Windows)
# Each network has its own IP address
# TODO: Make this automatic
TRANSCEIVER_TM_ADDRESS = ('192.168.1.51', 10035)
TRANSCEIVER_TC_ADDRESS = ('192.168.1.51', 10045)
