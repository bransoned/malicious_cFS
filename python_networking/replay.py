import socket

if __name__ == "__main__":
    opened_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    while True:

        print ("Which command do you want to send\n")
        print ("1: Enable/disable SB commands phoning home")
        print ("2: Find all pipes on the SB")

        cmd_prompt = int(input("\nChoose a number: "))

        match cmd_prompt:
            case 1:
                command = [0x18, 0x82, 0xC0, 0x00, 0x00, 0x01, 0x05, 0xA0]
            case 2:
                command = [0x18, 0x82, 0xC0, 0x00, 0x00, 0x01, 0x04, 0xA0]
            case _:
                print ("Invalid selection")
                exit()

        byte_message = bytes(command)
        opened_socket.sendto(byte_message, ("127.0.0.1", 1234))
        print("Confirmed send: ", byte_message)
