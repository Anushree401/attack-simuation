import time
from scapy.all import sniff, Dot11Deauth, Dot11Disas

# Configurable Time-Window Aggregation & Thresholds (Section 3.2 & 4.3)
AGGREGATION_WINDOW = 5  # Time window in seconds
DOS_THRESHOLD = 30       # Trigger alert if exceeded

packet_count = 0
window_start = time.time()

def process_frame(packet):
    global packet_count, window_start
    
    # Phase: Parsing & Classification (Section 3.3)
    if packet.haslayer(Dot11Deauth) or packet.haslayer(Dot11Disas):
        packet_count += 1
        
    # Phase: Time-Window Aggregation Evaluation
    current_time = time.time()
    if current_time - window_start >= AGGREGATION_WINDOW:
        print(f"[*] Window Analytics: Captured {packet_count} management frames in last {AGGREGATION_WINDOW}s")
        
        # Phase: Threshold Evaluation & Alerting (Section 3.3)
        if packet_count > DOS_THRESHOLD:
            print(f"[!!!] ALERT: Wireless DoS Detected! Rate: {packet_count} frames/window. Threshold: {DOS_THRESHOLD}")
            # Logging module hooks can be appended here (Section 1.4)
            
        # Reset window metrics
        packet_count = 0
        window_start = current_time

print("[*] FooBar: Wireless DoS Detection Engine Initiated...")
print("[*] Passive monitoring active on interface: mon0")

# Start continuous passive packet acquisition (Section 3.5)
sniff(iface="mon0", prn=process_frame, store=0)
