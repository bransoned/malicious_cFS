import socket

command = [0x18, 0x05, 0xC0, 0x00, 0x00, 0x01, 0x00, 0x23]

byte_message = bytes(command)
opened_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

for i in range(50):
    opened_socket.sendto(byte_message, ("127.0.0.1", 1234))
    print(byte_message[0])
