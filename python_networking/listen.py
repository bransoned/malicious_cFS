import socket

def receive_data():
    local_ip = "127.0.0.1"  # IP address of the local machine
    local_port = 8888       # Port to listen on

    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((local_ip, local_port))

    print(f"Listening on {local_ip}:{local_port}")

    while True:
        data, addr = sock.recvfrom(1024)  # Buffer size is 1024 bytes

        try:
            decoded_string = data.decode('utf-8')
            print(f"Received string: {decoded_string} from {addr}")

        except UnicodeDecodeError:
            hex_string = "0x" + data.hex()
            print(f"Received hex message: {hex_string} from {addr}")

if __name__ == "__main__":
    receive_data()
