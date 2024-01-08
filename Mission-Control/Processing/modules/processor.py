from struct import pack, unpack
import time
import queue

from modules.connection_manager import ConnectionManager 
from modules.rotator import Rotator
from modules.calculations import *
from config import CALCULATION_MESSAGE_STRUCTURE

class PacketProcessor:
  def __init__(self, 
         connection_manager: ConnectionManager,
         rotator: Rotator,
         apid_to_type: dict,
         telecommand_apid: dict,
         packetid_to_type: dict,
         telemetry_message_structure) -> None:
    # Objects
    self.connection_manager = connection_manager
    self.rotator = rotator
    
    # Message information
    self.apid_to_type = apid_to_type
    self.telecommand_apid = telecommand_apid
    self.packetid_to_type = packetid_to_type
    
    # Message structures
    self.telemetry_message_structure = telemetry_message_structure
    
    # Telemetry
    self.pfc_telemetry = dict.fromkeys(telemetry_message_structure["pfc"])
    self.bfc_telemetry = dict.fromkeys(telemetry_message_structure["bfc"])
    self.rotator_telemetry = dict.fromkeys(telemetry_message_structure["rotator"])
    
    # Timing
    self.last_pfc_telemetry_epoch = 0
    self.last_bfc_telemetry_epoch = 0
    
    # Calculations
    self.pfc_calculations = dict.fromkeys(telemetry_message_structure["pfc"], 0)
    self.bfc_calculations = dict.fromkeys(telemetry_message_structure["bfc"], 0)
    self.rotator_calculations = dict.fromkeys(telemetry_message_structure["rotator"], 0)
    self.pfc_calculations_index = 1
    self.bfc_calculations_index = 1
    
    # Queues
    self.processed_packets = queue.Queue()
  
  def process_packet(self):
    # Get a packet from connection manager
    packet = self.connection_manager.received_messages.get()
    
    try:
      # If the message is a string, it is from transceiver
      if isinstance(packet, str):
        converted = self.convert_message_to_ccsds(packet)
        if converted is None:
          raise Exception("Invalid message")
        
        # Split the message into parts
        ccsds, apid, sequence_count = converted
        
        # If able update the telemetry values from the message
        self.__update_values_from_message(packet)
        
        # Queue the packet for sending to YAMCS
        self.processed_packets.put(ccsds)
        
        print("Transceiver data successfully processed")
        
      # If the message is a bytearray, it is from YAMCS
      elif isinstance(packet, bytearray):
        converted = self.convert_ccsds_to_message(packet)
        if converted is None:
          raise Exception("Invalid ccsds packet")

        # Split the message into parts
        command, data, packet_id = converted
        
        # Check if the command is meant to change something in this program
        self.__process_rotator_command(packet_id, data)
          
        # Else queue the packet for sending to transceiver
        self.processed_packets.put(command)
        
        print("YAMCS Command successfully processed")
        
      else:
        raise Exception(f"Invalid packet type: {type(packet)}")
      
      # Complete the task
      self.connection_manager.received_messages.task_done()
                    
    except Exception as e:
      self.connection_manager.received_messages.task_done()
      print(f"An error occurred while processing packet: {e}")
    
  def __isfloat(self, value: str) -> bool:
    """
    Checks if a string is a float.
    """
    try:
      float(value)
      return True
    except ValueError:
      return False
  
  def convert_message_to_ccsds(self, message: str):
    """
    Converts a message string to a CCSDS packet.
    
    Returns the ccsds packet, apid, sequence count, is telemetry message 
        
    Refrences: https://public.ccsds.org/Pubs/133x0b2c1.pdf 
    Most of the useful information about packet structure starts from page 31
    """
    # Split the message into parts and get the apid and sequence count
    try:
      message_split = message.split(",")
      apid = int(message_split[1])
      sequence_count = int(message_split[2])
    except Exception as e:
      print(f"Error spliting message: {e}. Full message: {message}")
      return None
    
    # Convert each value string to corresponding data type and convert to bytearray
    packet_data = bytearray()
    for value in message_split[3:]:
      try:
        # Integer
        if value.isdigit():    
          byte = bytearray(pack("i", int(value) & 0xFFFFFFFF))
          byte.reverse() # Reverse the byte order as msbf is required, but pack returns lsbf
          packet_data += byte
        # Float
        elif self.__isfloat(value):
          byte = bytearray(pack("f", float(value)))
          byte.reverse() # Reverse the byte order as msbf is required, but pack returns lsbf
          packet_data += byte
        # String
        else:
          # Add a newline character as it is string termination character
          value += "\n"
          byte = bytearray(value.encode("utf-8"))
          packet_data += byte
      except Exception as e:
        print(f"Error converting value {value} to bytearray: {e}. Full message: {message}")
        return None
      
    # Create full packet
    try:
      packet = self.create_primary_header(apid, sequence_count, len(packet_data))
      packet += packet_data
    except Exception as e:
      print(f"Error creating primary header: {e}. Full message: {message}")
      return None
    
    return (packet, apid, sequence_count)
  
  def __binary_to_float(self, binary):
    return unpack('!f',pack('!I', int(binary, 2)))[0]
  
  def convert_packet_data_to_string(self,packet_id, packet_data):    
    # Heater set mode
    if packet_id == 1003:
      # Read 8 bits to get the heater mode
      heater_mode = str(int(packet_data[:8], 2))
      return heater_mode
    
    # PFC and BFC Ejection
    elif packet_id == 1004 or packet_id == 2004:
      # Read 8 bits to get the ejection channel
      heater_mode = str(int(packet_data[:8], 2))
      return heater_mode
    
    # RWC set mode
    elif packet_id == 2003:
      # Read 8 bits to get the rwc mode
      rwc_mode = str(int(packet_data[:8], 2))
      # Read next 32 bits to get the rwc direction
      rwc_direction_data = packet_data[8:40]
      rwc_direction = self.__binary_to_float(rwc_direction_data)
      return f"{rwc_mode},{rwc_direction}"
    
    # Manual rotator position and manual target coordinates
    elif packet_id == 3004 or packet_id == 3006:
      # Read 32 bits to get the latitude
      latitude = str(self.__binary_to_float(packet_data[:32]))
      # Read next 32 bits to get the longitude
      longitude = str(self.__binary_to_float(packet_data[32:64]))
      # Read next 32 bits to get the altitude
      altitude = str(self.__binary_to_float(packet_data[64:96]))
      return f"{latitude},{longitude},{altitude}"
      
    # Manual angles
    elif packet_id == 3005:
      # Read 32 bits to get the azimuth
      azimuth = str(self.__binary_to_float(packet_data[:32]))
      # Read next 32 bits to get the elevation
      elevation = str(self.__binary_to_float(packet_data[32:64]))
      return f"{azimuth},{elevation}"
    
    else:
      return ""
    
  def convert_ccsds_to_message(self, packet: bytearray):
    """
    Converts a CCSDS packet to a message string.
    If the packet is invalid, returns an empty string.
    """
    try:
      # Convert packet from bytes to hexadecimal string
      packet_hex = packet.hex()
      
      # Get the binary array
      ccsds_binary = bin(int(packet_hex, 16))[2:].zfill(len(packet_hex) * 4)

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
      packet_data = ccsds_binary[64:]
      
      # Convert to usable data types
      apid = int(apid, 2)
      packet_id = int(packet_id, 2)
      packet_sequence_count = int(packet_sequence_count, 2)

      # Get data values from packet
      data_string = self.convert_packet_data_to_string(packet_id, packet_data)

      # Get current time in epoch format
      current_time = int(time.mktime(time.localtime()))
      
      # Create the message
      if apid == 10:
        message = f"rtu_pfc,{packet_id},{packet_sequence_count},{current_time}"
      elif apid == 20:
        message = f"rtu_bfc,{packet_id},{packet_sequence_count},{current_time}"
      elif apid == 30:
        message = f"rtu_rotator"
      else:
        print(f"Invalid apid: {apid}")
        raise Exception("Invalid apid")
      
      if data_string != "":
        message += f",{data_string}"
        
      message += "~"
      
      return (message, data_string, packet_id) 
    
    except Exception as e:
      print(f"Error converting packet to message: {e}. Full packet: {packet}")
      return None
  
  def __process_rotator_command(self, packet_id: int, data: str) -> None:
    try:
      # Set target PFC
      if packet_id == 3000:
        self.rotator.set_target("pfc")
        
      # Set target BFC
      elif packet_id == 3001:
        self.rotator.set_target("bfc")    
            
      # Set rotator to auto tracking mode
      elif packet_id == 3002:
        self.rotator.set_control_mode("auto")
        
      # Set rotator to auto rotator position mode
      elif packet_id == 3003:
        self.rotator.set_rotator_position_mode("auto")
        
      # Set rotator to manual rotator position mode
      elif packet_id == 3004:
        latitude, longitude, altitude = data.split(",")
        self.rotator.set_manual_rotator_position(latitude, longitude, altitude)
      
      # Set rotator to manual angles mode
      elif packet_id == 3005:
        azimuth, elevation = data.split(",")
        self.rotator.set_manual_angles(azimuth, elevation)
        
      # Set rotator to manual target coordinates mode
      elif packet_id == 3006: 
        latitude, longitude, altitude = data.split(",")
        self.rotator.set_manual_target_position(latitude, longitude, altitude)
      
    except Exception as e:
      print(f"Error processing rotator command: {e}. Full command: {packet_id}, {data}")  
  
  def create_primary_header(self, apid: int, sequence_count: int, data_length: int) -> bytearray:
    """
    Creates the primary header of a CCSDS packet.
    """
    ## Packet version number - 3 bits total
    PACKET_VERSION_NUMBER = b"000"
    
    ## Packet identification field - 13 bits total
    # Packet type (0 is telemetry, 1 is telecommand)- 1 bit
    # Secondary header flag (Always 0, as it will NOT be used) - 1 bit
    # APID (Application Process Identifier) - 11 bits
    PACKET_TYPE = b"0"
    SECONDARY_HEADER_FLAG = b"0"
    apid_binary = bin(apid)[2:].zfill(11).encode()
    packet_identification_field = PACKET_TYPE + SECONDARY_HEADER_FLAG + apid_binary
    
    ## Packet Sequence Control - 16 bits total
    # Sequence flags (Always 11, as we are sending a single undivided packet) - 2 bits
    # Packet Sequence Count (Packet index)- 14 bits
    PACKET_SEQUENCE_FLAG = b"11"
    packet_sequence_count = bin(sequence_count)[2:].zfill(14).encode()
    packet_sequence_control = PACKET_SEQUENCE_FLAG + packet_sequence_count
    
    ## Packet data length - 16 bits total
    # 16-bit field contains a length count that equals one fewer than the length of the data field
    packet_data_length = bin(data_length - 1)[2:].zfill(16).encode()
    
    # Create the primary header
    primary_header = PACKET_VERSION_NUMBER + packet_identification_field + packet_sequence_control + packet_data_length
    primary_header = bytearray(int(primary_header[i:i+8], 2) for i in range(0, len(primary_header), 8))

    return primary_header
  
  def __update_values_from_message(self, message: str) -> None:
    """
    Converts a message string to a dictonary.
    If the message is invalid, returns an empty dictonary.
    """
    try:
      # Create a dictonary with keys from the telemetry_message_structure and values from the message
      message_split = message.split(",")
      apid = int(message_split[1])
      
      message_dict = {}
      
      # PFC essential telemetry
      if apid == [key for key, value in self.apid_to_type.items() if value == "pfc_essential"][0]:
        for key, value in zip(self.telemetry_message_structure["pfc"], message_split):
          if value.isdigit():
            message_dict[key] = int(value)
          elif self.__isfloat(value):
            message_dict[key] = float(value)
          else:
            message_dict[key] = value
        
        old_pfc_telemetry_epoch = self.last_pfc_telemetry_epoch
        self.last_pfc_telemetry_epoch = int(time.mktime(time.localtime()))
        time_delta = self.last_pfc_telemetry_epoch - old_pfc_telemetry_epoch
        
        if self.pfc_telemetry["gps_latitude"] != None:
          self.pfc_calculations = calculate_flight_computer_extra_telemetry(self.pfc_telemetry, message_dict, self.rotator_telemetry, time_delta, CALCULATION_MESSAGE_STRUCTURE["pfc"])
          message = f"calculations,60,{self.pfc_calculations_index},{self.pfc_calculations['gps_vertical_speed']},{self.pfc_calculations['baro_vertical_speed']},{self.pfc_calculations['horizontal_speed']},{self.pfc_calculations['gps_total_speed']},{self.pfc_calculations['ground_distance_to_rotator']},{self.pfc_calculations['straight_line_distance_to_rotator']}"
    
          converted = self.convert_message_to_ccsds(message)
          if converted is None:
            return
          
          # Split the message into parts
          ccsds, apid, sequence_count = converted
          self.processed_packets.put(ccsds)
          self.pfc_calculations_index += 1
          
        self.pfc_telemetry = message_dict
      
      # BFC essential telemetry
      elif apid == [key for key, value in self.apid_to_type.items() if value == "bfc_essential"][0]:
        for key, value in zip(self.telemetry_message_structure["bfc"], message_split):
          if value.isdigit():
            message_dict[key] = int(value)
          elif self.__isfloat(value):
            message_dict[key] = float(value)
          else:
            message_dict[key] = value
            
        old_bfc_telemetry_epoch = self.last_bfc_telemetry_epoch
        self.last_bfc_telemetry_epoch = int(time.mktime(time.localtime()))
        time_delta = self.last_bfc_telemetry_epoch - old_bfc_telemetry_epoch
        
        if self.bfc_telemetry["gps_latitude"] != None:
          self.bfc_calculations = calculate_flight_computer_extra_telemetry(self.bfc_telemetry, message_dict, self.rotator_telemetry, time_delta, CALCULATION_MESSAGE_STRUCTURE["bfc"])
          message = f"calculations,70,{self.bfc_calculations_index},{self.bfc_calculations['gps_vertical_speed']},{self.bfc_calculations['baro_vertical_speed']},{self.bfc_calculations['horizontal_speed']},{self.bfc_calculations['gps_total_speed']},{self.bfc_calculations['ground_distance_to_rotator']},{self.bfc_calculations['straight_line_distance_to_rotator']}"
    
          converted = self.convert_message_to_ccsds(message)
          if converted is None:
            return
          
          # Split the message into parts
          ccsds, apid, sequence_count = converted
          self.processed_packets.put(ccsds)
          self.bfc_calculations_index += 1
          
        self.bfc_telemetry = message_dict
      
      # Rotator position telemetry
      elif apid == [key for key, value in self.apid_to_type.items() if value == "rotator_position"][0]:
        for key, value in zip(self.telemetry_message_structure["rotator"], message_split):
          if value.isdigit():
            message_dict[key] = int(value)
          elif self.__isfloat(value):
            message_dict[key] = float(value)
          else:
            message_dict[key] = value
        self.rotator_telemetry = message_dict
        
    except Exception as e:
      print(f"Error converting message to dictonary: {e}. Full message: {message}")
    