from modules.processor import PacketProcessor
from modules.connection_manager import ConnectionManager
from modules.rotator import Rotator

class Router:
  def __init__(self, processor: PacketProcessor, connection: ConnectionManager, rotator: Rotator) -> None:
    self.processor = processor
    self.connection = connection
    self.rotator = rotator
  
  def send_data_to_logging(self):
    pass
  
  def send_data_to_map(self):
    pass
  
  def send_data_to_sondehub(self):
    pass
  
  def send_data_to_recovery_server(self):
    pass
  
  def send_data_to_yamcs(self):
    pass
  
  def send_command_to_transceiver(self):
    packet = self.processor.processed_packets.get()
    self.connection.sendable_messages.put(packet)
    self.processor.processed_packets.task_done()
    
  def send_rotator_command_to_transceiver(self):
    if self.rotator.rotator_last_command != self.rotator.rotator_command:
      self.connection.sendable_messages.put(self.rotator.rotator_command)
      self.rotator.rotator_last_command = self.rotator.rotator_command
      print(f"Rotator command sent: {self.rotator.rotator_command}")
  
  def update_rotator_data(self):
    # If the rotator is in auto position mode, update the rotator position
    if self.rotator.rotator_position_mode == "auto":
      self.rotator.set_auto_rotator_position(self.processor.rotator_telemetry["latitude"],
                                             self.processor.rotator_telemetry["longitude"],
                                             self.processor.rotator_telemetry["altitude"])

    # If the rotator is in auto tracking mode, update the rotator target position
    if self.rotator.rotator_target == "pfc":
      if self.processor.pfc_telemetry["gps_latitude"] != 0 and self.processor.pfc_telemetry["gps_longitude"] != 0:
        self.rotator.set_auto_target_position(self.processor.pfc_telemetry["gps_latitude"],
                                              self.processor.pfc_telemetry["gps_longitude"],
                                              self.processor.pfc_telemetry["gps_altitude"])
        
    elif self.rotator.rotator_target == "bfc":
      if self.processor.bfc_telemetry["gps_latitude"] != 0 and self.processor.bfc_telemetry["gps_longitude"] != 0:
        self.rotator.set_auto_target_position(self.processor.bfc_telemetry["gps_latitude"],
                                              self.processor.bfc_telemetry["gps_longitude"],
                                              self.processor.bfc_telemetry["gps_altitude"])