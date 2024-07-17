import socket
import time

if __name__ == "__main__":

    # Command to trigger sample app to set new time
    command = [0x18, 0x82, 0xC0, 0x00, 0x00, 0x01, 0x04, 0xA0]

    opened_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    print ("starting time attack\n")

    # send command every x seconds. slowly speed this up... to see time drift more dramatically 
    sleep_time = 10;
    while True:

        byte_message = bytes(command)
        opened_socket.sendto(byte_message, ("127.0.0.1", 1234))
        print("Confirmed send: ", byte_message)
        time.sleep(sleep_time)
        sleep_time -= .5
