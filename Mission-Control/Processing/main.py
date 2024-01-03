from time import sleep

from modules.processor import PacketProcessor
from modules.connection_manager import ConnectionManager
from modules.sondehub import SondeHubUploader
from modules.router import Router
from modules.map import Map
from modules.thread_manager import ThreadManager
import config

if __name__ == '__main__':
  # Create objects
  connection_manager = ConnectionManager(
    config.YAMCS_TM_ADDRESS,
    config.YAMCS_TC_ADDRESS,
    config.TRANSCEIVER_TM_ADDRESS,
    config.TRANSCEIVER_TC_ADDRESS
  )
  
  processor = PacketProcessor(
    connection_manager,
    config.HEADERS,
    config.HEADER_TO_APID,
    config.PACKETID_TO_HEADER,
    config.TELECOMMAND_APID,
    config.TELEMETRY_MESSAGE_STRUCTURE,
    config.STATUS_MESSAGE_STRUCTURE,
    config.INFO_MESSAGE_STRUCTURE,
    config.ERROR_MESSAGE_STRUCTURE
  )
  router = Router(processor, connection_manager)
  sondehub_uploader = SondeHubUploader()
  map = Map(map_server_port=11000)
  thread_manager = ThreadManager(connection_manager, processor, router, sondehub_uploader, map)
  
  # Start threads
  thread_manager.start_map_server_thread()
  thread_manager.start_receive_from_transceiver_thread()
  thread_manager.start_receive_from_yamcs_thread()
  thread_manager.start_packet_sending_thread()
  thread_manager.start_packet_processing_thread()
  thread_manager.start_router_thread()
  
  while True:
    try:
      print("All threads:")
      for thread in thread_manager.active_threads:
        if not thread.is_alive():
          thread_manager.active_threads.remove(thread)
          continue
        print(f"* Thread name: {thread.name}")
        
      print()
      sleep(1)
    except KeyboardInterrupt:
      print("Exiting...")
      exit(0)