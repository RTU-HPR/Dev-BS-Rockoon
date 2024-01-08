from threading import Thread, Event
from tracemalloc import start

from modules.connection_manager import ConnectionManager
from modules.processor import PacketProcessor
from modules.router import Router
from modules.sondehub import SondeHubUploader
from modules.map import Map

class ThreadManager:
  def __init__(self, connection_manager: ConnectionManager, packet_processor: PacketProcessor, router: Router, sondehub_uploader: SondeHubUploader, map: Map) -> None:
    self.connection_manager = connection_manager
    self.packet_processor = packet_processor
    self.router = router
    self.sondehub_uploader = sondehub_uploader
    self.map = map
    
    self.active_threads = []
    self.stop_event = Event()
    
  # THREAD STARTERS
  def start_receive_from_transceiver_thread(self):
    thread = Thread(target=self.receive_from_transceiver_thread, name="Transceiver Receiver")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_receive_from_yamcs_thread(self):
    thread = Thread(target=self.receive_from_yamcs_thread, name="YAMCS Receiver")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_send_to_transceiver_thread(self):
    thread = Thread(target=self.send_to_transceiver_thread, name="Transceiver Sender")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
  
  def start_send_to_yamcs_thread(self):
    thread = Thread(target=self.send_to_yamcs_thread, name="YAMCS Sender")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
  
  def start_packet_processing_thread(self):
    thread = Thread(target=self.packet_processing_thread, name="Packet Processor")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_send_processed_data_thread(self):
    thread = Thread(target=self.send_processed_data_thread, name="Processed Data Sender")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_map_server_thread(self):
    thread = Thread(target=self.map_server_thread, name="Map Server")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_map_update_thread(self):
    thread = Thread(target=self.map_update_thread, name="Map Updater")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_rotator_command_to_transceiver_thread(self):
    thread = Thread(target=self.rotator_command_to_transceiver_thread, name="Rotator Command Sender")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_rotator_data_update_thread(self):
    thread = Thread(target=self.rotator_data_update_thread, name="Rotator Data Updater")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_send_heartbeat_to_transceiver_thread(self):
    thread = Thread(target=self.send_heartbeat_to_transceiver_thread, name="Heartbeat Sender")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
  
  # THREAD FUNCTIONS
  def receive_from_transceiver_thread(self):
    while not self.stop_event.is_set():
      self.connection_manager.receive_from_transceiver()
      
  def receive_from_yamcs_thread(self):
    while not self.stop_event.is_set():
      self.connection_manager.receive_from_yamcs()
  
  def map_server_thread(self):
    while not self.stop_event.is_set():
      self.map.run_server()  
      
  def map_update_thread(self):
    while not self.stop_event.is_set():
      self.map.update_map()  
  
  def packet_processing_thread(self):
    while not self.stop_event.is_set():
      self.packet_processor.process_packet()
      
  def send_processed_data_thread(self):
    while not self.stop_event.is_set():
      self.router.send_processed_data()
  
  def send_to_transceiver_thread(self):
    while not self.stop_event.is_set():
      self.connection_manager.send_to_transceiver()
      
  def send_to_yamcs_thread(self):
    while not self.stop_event.is_set():
      self.connection_manager.send_to_yamcs()
      
  def rotator_command_to_transceiver_thread(self):
    while not self.stop_event.is_set():
      self.router.send_rotator_command_to_transceiver()
  
  def rotator_data_update_thread(self):
    while not self.stop_event.is_set():
      self.router.update_rotator_data()
  
  def send_heartbeat_to_transceiver_thread(self):
    while not self.stop_event.is_set():
      self.connection_manager.send_heartbeat_to_transceiver()
  