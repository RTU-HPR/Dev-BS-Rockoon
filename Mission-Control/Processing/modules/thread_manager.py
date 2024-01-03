from concurrent.futures import thread
from threading import Thread

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
    
  def start_receive_from_transceiver_thread(self):
    thread = Thread(target=self.connection_manager.receive_from_transceiver, name="Transceiver Receiver")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_receive_from_yamcs_thread(self):
    thread = Thread(target=self.connection_manager.receive_from_yamcs, name="YAMCS Receiver")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
  
  def start_map_server_thread(self):
    thread = Thread(target=self.map.run_server, name="Map Server")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_packet_processing_thread(self):
    thread = Thread(target=self.packet_processor.process_packet, name="Packet Processor")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
    
  def start_packet_sending_thread(self):
    thread = Thread(target=self.connection_manager.send_packet, name="Packet Sender")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)
  
  def start_router_thread(self):
    thread = Thread(target=self.router.send_command_to_transceiver, name="Router")
    thread.daemon = True
    thread.start()
    self.active_threads.append(thread)