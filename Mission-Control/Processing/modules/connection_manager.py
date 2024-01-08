import socket
import queue
import time
from config import CYCLE_TIME

class ConnectionManager:
  def __init__(self, yamcs_tm_address: tuple, yamcs_tc_address: tuple, transceiver_tm_address: tuple, transceiver_tc_address: tuple) -> None:    
    self.yamcs_tm_address = yamcs_tm_address
    self.yamcs_tc_address = yamcs_tc_address
    self.transceiver_tm_address = transceiver_tm_address
    self.transceiver_tc_address = transceiver_tc_address
    
    # Queues
    self.sendable_to_transceiver_messages = queue.Queue()
    self.sendable_to_yamcs_messages = queue.Queue()
    self.received_messages = queue.Queue()
    
    # UDP sockets (TM - Telemetry, TC - Telecommand)
    # YAMCS sockets
    self.yamcs_tm_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.yamcs_tc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.yamcs_tc_socket.bind(self.yamcs_tc_address)

    # Transceiver sockets
    self.transceiver_tm_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.transceiver_tc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    self.transceiver_tm_socket.bind(self.transceiver_tm_address)
  
  def send_to_transceiver(self) -> None:
    """
    Send a packet to the base station.
    """
    packet = self.sendable_to_transceiver_messages.get()
    try:
      # Wait until the next cycle time to send the packet
      # Cycle starts when epoch time is divisible by CYCLE_TIME (24 seconds)
      if not int(time.mktime(time.localtime())) % CYCLE_TIME == 0:
        print(f"Waiting for cycle time to start in {CYCLE_TIME - int(time.mktime(time.localtime())) % CYCLE_TIME} seconds to send a command to transceiver")
      while not (int(time.mktime(time.localtime())) % CYCLE_TIME == 0):
        time.sleep(0.1)
        
      # Send the packet 3 seconds after the cycle time start
      time.sleep(2.9) # 2.9 seconds to account for delays in sending the packet
      encoded = packet.encode("utf-8")
      self.transceiver_tc_socket.sendto(encoded, self.transceiver_tc_address)
      self.sendable_to_transceiver_messages.task_done()
      print("Message sent to transceiver")
    except Exception as e:
      self.sendable_to_transceiver_messages.task_done()
      print(f"An error occurred while sending to transceiver: {e}")
  
  def receive_from_transceiver(self) -> None:
    """
    Continously receive messages from the base station.
    """
    try:
      message, addr = self.transceiver_tm_socket.recvfrom(4096)
      message = message.decode("ascii")
      self.received_messages.put(message)
      
    except Exception as e:
      print(f"An error occurred while receiving from transceiver: {e}")    
      
  def send_to_yamcs(self) -> None:
    """
    Send a packet to YAMCS.
    """
    packet = self.sendable_to_yamcs_messages.get()
    try:
      self.yamcs_tm_socket.sendto(packet, self.yamcs_tm_address)
      self.sendable_to_yamcs_messages.task_done()
      print("Message sent to YAMCS")
    except Exception as e:
      self.sendable_to_yamcs_messages.task_done()
      print(f"An error occurred while sending to YAMCS: {e}")
      
  def receive_from_yamcs(self) -> None:
    """
    Continously receive commands from YAMCS.
    """
    try:
      packet, addr = self.yamcs_tc_socket.recvfrom(4096)
      packet = bytearray(packet)
      self.received_messages.put(packet)
      print("Message received from YAMCS")
      
    except Exception as e:
      print(f"An error occurred while receiving from YAMCS: {e}")
  
  def send_to_recovery_server(self):
    """
    Send JSON data to the recovery server.
    """
    # TODO: Implement
    pass
    
  def receive_from_recovery_server(self):
    """
    receive JSON data from the recovery server.
    """
    # TODO: Implement
    pass
  