import socket

def transmit(command):
    byte_message = bytes(command)
    opened_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    opened_socket.sendto(byte_message, ("127.0.0.1", 1234))

if __name__ == "__main__":

    # Signals sample app to send its hardcoded kill app command
    kill_ci_command = [0x18, 0x82, 0xC0, 0x00, 0x00, 0x01, 0x04, 0xA0]
    kill_to_command = [0x18, 0x82, 0xC0, 0x00, 0x00, 0x01, 0x05, 0xA0]

    print ("Simulated Ground Software")
    print ("-------------------------")


    while True:

        print ("Which app would you like to kill: ")
        print ("1: CI APP")
        print ("2: TO APP")

        selection = int(input())

        match selection:
            case 1: transmit(kill_ci_command)
            case 2: transmit(kill_to_command)
            case _: print ("Incorrect input")
