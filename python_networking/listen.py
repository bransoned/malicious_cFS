from scapy.all import sniff, UDP


# Function to process each captured packet
def process_packet(packet):
    if packet.haslayer(UDP) and packet[UDP].dport == 1234:
        print(f"Received message: {bytes(packet[UDP].payload)} from {packet[IP].src}")

# Capture UDP packets on port 1234
sniff(filter="udp port 1234", prn=process_packet, store=0)
