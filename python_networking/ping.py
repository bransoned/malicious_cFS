# Basis of sending a ping command once every 10 seconds
# Then compare the results of the time drift from other script

import socket
import time

ping_command = [0x18, 0x82, 0xC0, 0x00, 0x00, 0x01, 0x00, 0xA0]

def transmit(command):
    byte_message = bytes(command)
    opened_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    opened_socket.sendto(byte_message, ("127.0.0.1", 1234))

while True:
    transmit(ping_command)
    time.sleep(10)
