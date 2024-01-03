from modules.processor import PacketProcessor
from modules.connection_manager import ConnectionManager
class Router:
  def __init__(self, processor: PacketProcessor, connection: ConnectionManager) -> None:
    self.processor = processor
    self.connection = connection
  
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
    source, packet = self.processor.processed_packets.get()
    self.connection.sendable_messages.put((source, packet))
    self.processor.processed_packets.task_done()
  
  def send_command_to_rotator(self):
    pass