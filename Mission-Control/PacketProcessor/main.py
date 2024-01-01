import struct
from time import sleep
import socket
from threading import Thread
import sys
import time

# CCSDS packet refrence: https://public.ccsds.org/Pubs/133x0b2e1.pdf 
# Most of the useful information about packet structure starts from page 31

# Config
# 11 bit integer (Values from 0 to 2047) APID is used four our packets
# First digit is the system identifier (1 for PFC, 2 for BFC)
# Second digit is the receive or transmit identifier (0 for receive, 1 for transmit)
# Third digit is the packet identifier
# WiFi Received message prefixes and their APIDs
pfc_prefix_apid = {
  "rtu_hpr_pfc": 100,
  "rtu_hpr_pfc_status": 101,
  "rtu_hpr_pfc_info": 102,
  "rtu_hpr_pfc_error": 103,
}

bfc_prefix_apid = {
  "rtu_hpr_bfc": 200,
  "rtu_hpr_bfc_status": 201,
  "rtu_hpr_bfc_info": 202,
  "rtu_hpr_bfc_error": 203,
}

# YAMCS Received message packet id and message prefixes
# All telecommands have the same APID, but have different packet ids
pfc_telecommand_apid = 110
pfc_packetid_prefix = {
  1000: "rtu_hpr_pfc_status_request",
  1001: "rtu_hpr_pfc_data_request",
  1002: "rtu_hpr_pfc_ranging_request",
  1003: "rtu_hpr_pfc_mosfet_1_request",
  1004: "rtu_hpr_pfc_mosfet_2_request",
}

bfc_telecommand_apid = 210
bfc_packetid_prefix = {
  2000: "rtu_hpr_bfc_status_request",
  2001: "rtu_hpr_bfc_data_request",
}

class Basestation():
    def __init__(self):
      # Counters
      self.tc_counter = 0
      
      # Threads
      # Continously running threads
      self.yamcs_tc_thread = None
      self.wifi_tm_thread = None
      # Message processing threads
      self.message_processing_threads = []
      
      # Used UDP addresses
      self.yamcs_tm_address = ('localhost', 10015)
      self.yamcs_tc_address = ('localhost', 10025)
      self.wifi_tm_address = ('192.168.251.232', 10035)
      self.wifi_tc_address = ('192.168.251.61', 10045)
      # Find your local IPv4 address with ipconfig (Windows)
      # Each network has its own IP address
      # TODO: Make this automatic
      # self.wifi_tm_address = ('192.168.1.51', 10035)
      # self.wifi_tc_address = ('192.168.1.51', 10045)
      
      # UDP sockets (TM - Telemetry, TC - Telecommand)
      # YAMCS sockets
      self.yamcs_tm_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.yamcs_tc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.yamcs_tc_socket.bind(self.yamcs_tc_address)

      # WiFi sockets
      self.wifi_tm_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.wifi_tc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
      self.wifi_tm_socket.bind(self.wifi_tm_address)

    def receive_from_wifi(self):
      while True:
        try:
          message, addr = self.wifi_tm_socket.recvfrom(4096)
          message = message.decode("ascii")
          print(f"Received: {message}")
          # Start a new thread to process the message
          thread = Thread(target=self.process_wifi_telemetry, args=(message,))
          self.message_processing_threads.append(thread)
          self.message_processing_threads[-1].daemon = True
          self.message_processing_threads[-1].start()
          
        except Exception as e:
          print(f"An error occurred while receiving from WiFi: {e}")

    def send_to_wifi(self, packet: bytearray):
      try:
        print("Sending to WiFi")
        self.wifi_tc_socket.sendto(packet, self.wifi_tc_address)
      except Exception as e:
        print(f"An error occurred while sending to WiFi: {e}")

    def receive_from_yamcs(self):
      while True:
        packet, addr = self.yamcs_tc_socket.recvfrom(4096)
        # Start a new thread to process the message
        thread = Thread(target=self.process_yamcs_telecommand, args=(packet,))
        self.message_processing_threads.append(thread)
        self.message_processing_threads[-1].daemon = True
        self.message_processing_threads[-1].start()
        self.tc_counter += 1

    def send_to_yamcs(self, packet: bytearray):
      try:
        self.yamcs_tm_socket.sendto(packet, self.yamcs_tm_address)
      except Exception as e:
        print(f"An error occurred while sending to YAMCS: {e}")
    
    def create_primary_header(self, apid, sequence_count, data_length) -> bytearray:
      ## Packet version number - 3 bits total
      packet_version_number = b"000"
      
      ## Packet identification field - 13 bits total
      # Packet type (0 is telemetry, 1 is telecommand)- 1 bit
      # Secondary header flag (Always 0, as we won't use it) - 1 bit
      # APID (Application Process Identifier - What is this message?) - 11 bits
      packet_type = b"0"
      seconday_header_flag = b"0"
      apid_binary = bin(apid)[2:].zfill(11).encode()
      packet_identification_field = packet_type + seconday_header_flag + apid_binary
      
      ## Packet Sequence Control - 16 bits total
      # Sequence flags (Always 11, as we are sending a single undivided packed) - 2 bits
      # Packet Sequence Count (Packet index)- 14 bits
      packet_sequence_flag = b"11"
      packet_sequence_count = bin(sequence_count)[2:].zfill(14).encode()
      packet_sequence_control = packet_sequence_flag + packet_sequence_count
      
      ## Packet data length - 16 bits total
      # 16-bit field that contain a length count that equals one fewer than the length of the data field
      packet_data_length = bin(data_length - 1)[2:].zfill(16).encode()
      
      # Create the primary header
      primary_header = packet_version_number + packet_identification_field + packet_sequence_control + packet_data_length
      primary_header = bytearray(int(primary_header[i:i+8], 2) for i in range(0, len(primary_header), 8))

      return primary_header
    
    def convert_message_to_ccsds(self, message: str) -> bytearray:
      # Split the string in values 
      message_split = message.split(",")

      # Get the APID from the prefix depending on the system
      if "pfc" in message_split[0]:
        apid = pfc_prefix_apid[message_split[0]]
      elif "bfc" in message_split[0]:
        apid = bfc_prefix_apid[message_split[0]]
        
      # Get the sequence count from the message
      sequence_count = int(message_split[1])
      
      # Convert each value string to corresponding data type and convert to bytearray
      packet_data = bytearray()
      for val in message_split[1:]:
        # Float
        if "." in val:
          byte = bytearray(struct.pack("f", float(val))) # Experiment with "big" argument
          byte.reverse() # Reverse the byte order as msbf is required but struct.pack returns lsbf
          packet_data += byte
        # Integer
        elif val.isdigit():    
          byte = bytearray(struct.pack("i", int(val)))
          byte.reverse()
          packet_data += byte
        # Time string
        elif ":" in val:
          time_parts = val.split(":")
          time_parts = [part.zfill(2) for part in time_parts]
          time_string_padded = ":".join(time_parts)
          byte = bytearray(time_string_padded.encode("utf-8"))
          packet_data += byte
        # Something else
        else:
          print(f"Undefined value: {val}")
          
      # Create full packet
      packet = self.create_primary_header(apid, sequence_count, len(packet_data))
      packet += packet_data
        
      return packet

    def convert_ccsds_to_message(self, packet):
      # Convert packet from bytes to hexadecimal string
      packet_hex = packet.hex()
      
      # Get the binary array
      ccsds_binary = bin(int(packet_hex, 16))[2:].zfill(len(packet_hex) * 4)

      # We only need the apid, packet id and packet data, but we get the all of the fields, just in case we need them later
      # Secondary header field
      packet_version_number = ccsds_binary[:3]      
      
      packet_identification_field = ccsds_binary[3:16]
      packet_type = packet_identification_field[0]
      secondary_header_flag = packet_identification_field[1:2]
      apid = packet_identification_field[2:]

      packet_sequence_control = ccsds_binary[16:32]
      sequence_flags = packet_sequence_control[:2]
      packet_sequence_count = packet_sequence_control[2:]
      
      # User data field
      packet_data_length = ccsds_binary[32:48]
      packet_id = ccsds_binary[48:64]
      
      # Get packet data
      packet_data = ccsds_binary[64:]

      # Convert to usable data types
      apid = int(apid, 2)
      packet_id = int(packet_id, 2)
      packet_sequence_count = int(packet_sequence_count, 2)

      # Get the message prefix from the APID and packet id
      if apid == pfc_telecommand_apid:
        message_prefix = pfc_packetid_prefix[packet_id]
      elif apid == bfc_telecommand_apid:
        message_prefix = bfc_packetid_prefix[packet_id]

      # Get current time in HH:MM:SS format
      current_time = ":".join([str(x).zfill(2) for x in list(time.localtime())[3:6]])      
      
      # We could also parse and add the packet data to message, but we don't need it for now

      # Create the message
      message = f"{message_prefix},{packet_sequence_count},{current_time}"
      
      print(message)
      return message
    
    def process_yamcs_telecommand(self, packet):
      message = self.convert_ccsds_to_message(packet)
      self.send_to_wifi(message.encode("utf-8"))
    
    def process_wifi_telemetry(self, message: str):
      ccsds = self.convert_message_to_ccsds(message)
      self.send_to_yamcs(ccsds)
      
    def start(self):
      # Start UDP threads
      # Start listening for messages from YAMCS
      self.yamcs_tc_thread = Thread(target=self.receive_from_yamcs)
      self.yamcs_tc_thread.daemon = True
      self.yamcs_tc_thread.start()

      # Start listening for messages from WiFi
      self.wifi_tm_thread = Thread(target=self.receive_from_wifi)
      self.wifi_tm_thread.daemon = True
      self.wifi_tm_thread.start()


if __name__ == '__main__':
  basestation = Basestation()
  basestation.start()

  try:
    while True:
      sleep(0.1)
  except KeyboardInterrupt:
    print("Exiting...")
    sys.exit(0)