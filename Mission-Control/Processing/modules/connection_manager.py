import socket
import queue

class ConnectionManager:
  def __init__(self, yamcs_tm_address: tuple, yamcs_tc_address: tuple, transceiver_tm_address: tuple, transceiver_tc_address: tuple) -> None:    
    self.yamcs_tm_address = yamcs_tm_address
    self.yamcs_tc_address = yamcs_tc_address
    self.transceiver_tm_address = transceiver_tm_address
    self.transceiver_tc_address = transceiver_tc_address
    
    # Queues
    self.sendable_messages = queue.Queue()
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
    
  def send_packet(self) -> None:
    """
    Wait for sendable packets in queue and send them to the appropriate destination.
    """
    while True:
      source, packet = self.sendable_messages.get()
      print(f"Sending packet from {source}: {packet}")
      try:
        if source == "yamcs":
          self.__send_to_transceiver(packet)
        elif source == "transceiver":
          self.__send_to_yamcs(packet)
        self.sendable_messages.task_done()
      except Exception as e:
        self.sendable_messages.task_done()
        print(f"An error occurred while sending a packet: {e}")
  
  def __send_to_transceiver(self, packet: str) -> None:
    """
    Send a packet to the base station.
    """
    try:
      encoded = packet.encode("utf-8")
      self.transceiver_tc_socket.sendto(encoded, self.transceiver_tc_address)
    except Exception as e:
      print(f"An error occurred while sending to transceiver: {e}")
  
  def receive_from_transceiver(self):
    """
    Continously receive messages from the base station.
    """
    while True:
      try:
        message, addr = self.transceiver_tm_socket.recvfrom(4096)
        message = message.decode("ascii")
        self.received_messages.put(("transceiver", message))
        
      except Exception as e:
        print(f"An error occurred while receiving from transceiver: {e}")    
      
  def __send_to_yamcs(self, packet: bytearray) -> None:
    """
    Send a packet to YAMCS.
    """
    try:
      self.yamcs_tm_socket.sendto(packet, self.yamcs_tm_address)
    except Exception as e:
      print(f"An error occurred while sending to YAMCS: {e}")
      
  def receive_from_yamcs(self) -> None:
    """
    Continously receive commands from YAMCS.
    """
    while True:
      try:
        packet, addr = self.yamcs_tc_socket.recvfrom(4096)
        packet = bytearray(packet)
        self.received_messages.put(("yamcs", packet))
        
      except Exception as e:
        print(f"An error occurred while receiving from YAMCS: {e}")
  
  def __send_to_recovery_server(self):
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
  