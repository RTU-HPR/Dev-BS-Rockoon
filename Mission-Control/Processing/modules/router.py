from modules.processor import PacketProcessor
from modules.connection_manager import ConnectionManager
from modules.rotator import Rotator
from modules.map import Map
from time import sleep

class Router:
  def __init__(self, processor: PacketProcessor, connection: ConnectionManager, rotator: Rotator, map: Map) -> None:
    self.processor = processor
    self.connection = connection
    self.rotator = rotator
    self.map = map
  
  def send_data_to_map(self):
    # If the balloon has a valid position and the coordinates are not already in the list, add them
    if self.processor.bfc_telemetry["gps_latitude"] != 0 and self.processor.bfc_telemetry["gps_longitude"] != 0:
      if isinstance(self.processor.bfc_telemetry["gps_latitude"], float) and isinstance(self.processor.bfc_telemetry["gps_longitude"], float):
        if [self.processor.bfc_telemetry["gps_latitude"], self.processor.bfc_telemetry["gps_longitude"]] not in self.map.ballon_coordinates:
          self.map.ballon_coordinates.append([self.processor.bfc_telemetry["gps_latitude"], self.processor.bfc_telemetry["gps_longitude"]])
          self.map.map_update_required = True    

    # If the payload has a valid position and the coordinates are not already in the list, add them
    if self.processor.pfc_telemetry["gps_latitude"] != 0 and self.processor.pfc_telemetry["gps_longitude"] != 0:
      if isinstance(self.processor.pfc_telemetry["gps_latitude"], float) and isinstance(self.processor.pfc_telemetry["gps_longitude"], float):
        if [self.processor.pfc_telemetry["gps_latitude"], self.processor.pfc_telemetry["gps_longitude"]] not in self.map.payload_coordinates:
          self.map.payload_coordinates.append([self.processor.pfc_telemetry["gps_latitude"], self.processor.pfc_telemetry["gps_longitude"]])
          self.map.map_update_required = True    
        
    # If the rotator has a valid position and the coordinates are not the same as the last ones in the list, change them
    if [self.processor.rotator_telemetry["latitude"], self.processor.rotator_telemetry["longitude"]] not in self.map.rotator_coordinates:
      if isinstance(self.processor.rotator_telemetry["latitude"], float) and isinstance(self.processor.rotator_telemetry["longitude"], float):
        self.map.rotator_coordinates = [[self.processor.rotator_telemetry["latitude"], self.processor.rotator_telemetry["longitude"]]]
        self.map.map_update_required = True  
        
    sleep(0.1)  
  
  def send_processed_data(self):
    try:
      packet = self.processor.processed_packets.get()
      if isinstance(packet, str):
        self.connection.sendable_to_transceiver_messages.put(packet)
      elif isinstance(packet, bytearray):
        self.connection.sendable_to_yamcs_messages.put(packet)
      else:
        raise TypeError("Packet must be a string or bytearray")  
      self.processor.processed_packets.task_done()
    except Exception as e:
      print(f"An error occurred while sending processed data: {e}")
    
  def send_rotator_command_to_transceiver(self):
    if self.rotator.rotator_last_command != self.rotator.rotator_command:
      converted = self.processor.convert_message_to_ccsds(self.rotator.rotator_command)
      if converted is None:
        return
      
      # Split the message into parts
      ccsds, apid, sequence_count = converted
      self.processor.processed_packets.put(ccsds)
      self.rotator.rotator_command_index += 1
      
      self.connection.sendable_to_transceiver_messages.put(self.rotator.rotator_command)
      self.rotator.rotator_last_command = self.rotator.rotator_command
      
      print(f"Rotator command sent. Angles: {self.rotator.rotator_command.split(',')[3:]}")
    sleep(0.1)
    
  def update_rotator_data(self):
    # Update the rotator position
    self.rotator.set_auto_rotator_position(self.processor.rotator_telemetry["latitude"],
                                            self.processor.rotator_telemetry["longitude"],
                                            self.processor.rotator_telemetry["altitude"])

    # If the rotator is in auto tracking mode, update the rotator target position
    if self.rotator.rotator_target == "pfc":
      self.rotator.set_auto_target_position(self.processor.pfc_telemetry["gps_latitude"],
                                              self.processor.pfc_telemetry["gps_longitude"],
                                              self.processor.pfc_telemetry["gps_altitude"])
        
    elif self.rotator.rotator_target == "bfc":
      self.rotator.set_auto_target_position(self.processor.bfc_telemetry["gps_latitude"],
                                              self.processor.bfc_telemetry["gps_longitude"],
                                              self.processor.bfc_telemetry["gps_altitude"])
    sleep(0.1)
  
    
  def send_data_to_logging(self):
    pass
  
  def send_data_to_sondehub(self):
    pass
  
  def send_data_to_recovery_server(self):
    pass