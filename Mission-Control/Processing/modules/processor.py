from struct import pack
from time import localtime
import queue

from modules.connection_manager import ConnectionManager 
from modules.rotator import Rotator

class PacketProcessor:
  def __init__(self, 
               connection_manager: ConnectionManager,
               rotator: Rotator,
               receive_headers: dict,
               request_headers: dict,
               header_to_apid: dict,
               packetid_to_header: dict,
               telecommand_apid: dict,
               telemetry_message_structure: dict,
               status_message_structure: list,
               info_message_structure: list,
               error_message_structure: list) -> None:
    # Objects
    self.connection_manager = connection_manager
    self.rotator = rotator
    
    # Message information
    self.receive_headers = receive_headers
    self.request_headers = request_headers
    self.header_to_apid = header_to_apid
    self.packetid_to_header = packetid_to_header
    self.telecommand_apid = telecommand_apid
    
    # Message structures
    self.telemetry_message_structure = telemetry_message_structure
    self.status_message_structure = status_message_structure
    self.info_message_structure = info_message_structure
    self.error_message_structure = error_message_structure
    
    # Telemetry
    self.pfc_telemetry = {"gps_latitude": 0, "gps_longitude": 0, "gps_altitude": 0}
    self.bfc_telemetry = {"gps_latitude": 0, "gps_longitude": 0, "gps_altitude": 0}
    self.rotator_telemetry = {"latitude": 0, "longitude": 0, "altitude": 0}
    
    # Queues
    self.processed_packets = queue.Queue()
  
  def process_packet(self):
    while True:
      # Get a packet from connection manager
      packet = self.connection_manager.received_messages.get()
      
      try:
        # If the message is a string, it is from transceiver
        if isinstance(packet, str):
          converted = self.__convert_message_to_ccsds(packet)
          if converted is None:
            raise Exception("Invalid message")
          
          # Split the message into parts
          ccsds, apid, sequence_count, is_telemetry_message = converted
          
          # If the message is a telemetry message, update the telemetry values from the original message
          if is_telemetry_message:
            self.__update_values_from_message(packet)
          
          # Queue the packet for sending to YAMCS
          self.processed_packets.put(ccsds)
          
        # If the message is a bytearray, it is from YAMCS
        elif isinstance(packet, bytearray):
          converted = self.__convert_ccsds_to_message(packet)
          if converted is None:
            raise Exception("Invalid ccsds packet")

          # Split the message into parts
          command, header, data, apid = converted
          
          # Check if the command is meant to change something in this program
          if apid == self.telecommand_apid["software"]:
            self.__process_software_command(header, data)
            
          # Else queue the packet for sending to transceiver
          else:
            self.processed_packets.put(command)
        else:
          raise Exception(f"Invalid packet type: {type(packet)}")
        
        # Complete the task
        self.connection_manager.received_messages.task_done()
                      
      except Exception as e:
        self.connection_manager.received_messages.task_done()
        print(f"An error occurred while processing packet: {e}")
    
  
  def __convert_message_to_ccsds(self, message: str):
    """
    Converts a message string to a CCSDS packet.
    
    Returns the ccsds packet, apid, sequence count, is telemetry message 
        
    Refrences: https://public.ccsds.org/Pubs/133x0b2c1.pdf 
    Most of the useful information about packet structure starts from page 31
    """
    # Split the message into parts and get the apid and sequence count
    try:
      message_split = message.split(",")
      apid = self.header_to_apid[message_split[0]]
      sequence_count = int(message_split[1])
    except Exception as e:
      print(f"Error spliting message: {e}. Full message: {message}")
      return None
    
    # Convert each value string to corresponding data type and convert to bytearray
    packet_data = bytearray()
    
    # Process the time string the same for all messages
    # Make sure that the time string is in HH:MM:SS format
    time_parts = message_split[2].split(":")
    time_parts = [part.zfill(2) for part in time_parts]
    time_string_padded = ":".join(time_parts)
    byte = bytearray(time_string_padded.encode("utf-8"))
    packet_data += byte
    
    # Flags
    is_telemetry_message = False
    
    # Telemetry/rotator message data fields will have integers/floats, while status/info/error messages will only have strings
    if "telemetry" in message_split[0] or "rotator" in message_split[0]:
      is_telemetry_message = True
      for value in message_split[3:]:
        try:
          # Integer
          if value.isdigit():    
            byte = bytearray(pack("i", int(value)))
            byte.reverse() # Reverse the byte order as msbf is required, but pack returns lsbf
            packet_data += byte
          # Float
          elif "." in value:
            byte = bytearray(pack("f", float(value)))
            byte.reverse() # Reverse the byte order as msbf is required, but pack returns lsbf
            packet_data += byte
          else:
            raise Exception("Invalid value in telemetry message")
        except Exception as e:
          print(f"Error converting value {value} to bytearray: {e}. Full message: {message}")
          return None
    else:
      try:
        byte = bytearray(message[3].encode("utf-8"))
        packet_data += byte
      except Exception as e:
        print(f"Error converting value {message[3]} to bytearray: {e}. Full message: {message}")
        return None
      
    # Create full packet
    try:
      packet = self.__create_primary_header(apid, sequence_count, len(packet_data))
      packet += packet_data
    except Exception as e:
      print(f"Error creating primary header: {e}. Full message: {message}")
      return None
    
    return (packet, apid, sequence_count, is_telemetry_message)  
  
  
  def __convert_ccsds_to_message(self, packet: bytearray):
    """
    Converts a CCSDS packet to a message string.
    If the packet is invalid, returns an empty string.
    """
    try:
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
      packet_data = ccsds_binary[64:]

      # Convert to usable data types
      apid = int(apid, 2)
      packet_id = int(packet_id, 2)
      packet_sequence_count = int(packet_sequence_count, 2)

      # Get the message header from the packet id
      message_header = self.packetid_to_header[packet_id]

      # Get current time in HH:MM:SS format
      current_time = ":".join([str(x).zfill(2) for x in list(localtime())[3:6]])      
      
      # We could also parse and add the packet data to message, but we don't need it for now
      data = packet_data
      # Create the message
      message = f"{message_header},{packet_sequence_count},{current_time}"
      if data != "":
        message += f",{data}"
      
      return (message, message_header, data, apid) 
    
    except Exception as e:
      print(f"Error converting packet to message: {e}. Full packet: {packet}")
      return None
  
  
  def __process_software_command(self, header: str, data: str) -> None:
    try:
      # Rotator control
      if header == self.request_headers["software"]["target_pfc"] or header == self.request_headers["software"]["target_bfc"]:
        self.rotator.set_target(data)
        
      elif header == self.request_headers["software"]["rotator_auto"]:
        self.rotator.set_control_mode("auto")
        
      elif header == self.request_headers["software"]["rotator_auto_rotator_position"]:
        self.rotator.set_rotator_position_mode("auto")
        
      elif header == self.request_headers["software"]["rotator_manual_rotator_position"]:
        latitude, longitude, altitude = data.split(",")
        self.rotator.set_manual_rotator_position(latitude, longitude, altitude)
        
      elif header == self.request_headers["software"]["rotator_manual_angles"]:
        azimuth, elevation = data.split(",")
        self.rotator.set_manual_angles(azimuth, elevation)
        
      elif header == self.request_headers["software"]["rotator_manual_target_coordinates"]:
        latitude, longitude, altitude = data.split(",")
        self.rotator.set_manual_target_position(latitude, longitude, altitude)
      
    except Exception as e:
      print(f"Error processing software command: {e}. Full command: {header}, {data}")
    
  
  def __create_primary_header(self, apid: int, sequence_count: int, data_length: int) -> bytearray:
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
      message_dict = {}
      
      if message_split[0] == self.receive_headers["pfc"]["telemetry"]:
        for key, value in zip(self.telemetry_message_structure["pfc"], message_split):
          if value.isdigit():
            message_dict[key] = int(value)
          elif "." in value:
            message_dict[key] = float(value)
          else:
            message_dict[key] = value
        self.pfc_telemetry = message_dict
      
      elif message_split[0] == self.receive_headers["bfc"]["telemetry"]:
        for key, value in zip(self.telemetry_message_structure["bfc"], message_split):
          if value.isdigit():
            message_dict[key] = int(value)
          elif "." in value:
            message_dict[key] = float(value)
          else:
            message_dict[key] = value
        self.bfc_telemetry = message_dict
      
      elif message_split[0] == self.receive_headers["rotator"]["position"]:
        for key, value in zip(self.telemetry_message_structure["rotator"], message_split):
          if value.isdigit():
            message_dict[key] = int(value)
          elif "." in value:
            message_dict[key] = float(value)
          else:
            message_dict[key] = value
        self.rotator_telemetry = message_dict
      else:
        raise Exception("Invalid message header")
    except Exception as e:
      print(f"Error converting message to dictonary: {e}. Full message: {message}")
    