#!/usr/bin/env python3
import sys
import time
from collections import Counter
from scapy.all import sniff, Dot11, Dot11Deauth, Dot11Disas, Dot11Beacon

# --- CONFIGURABLE ALARM THRESHOLDS ---
DEAUTH_THRESHOLD = 30   # Alerts if more than 30 Deauth frames hit in 10 seconds
BEACON_THRESHOLD = 150  # Alerts if massive unexpected Beacons flood the channel

class WirelessDetectionEngine:
    def __init__(self, interface):
        self.interface = interface
        self.window_start = time.time()
        self.window_duration = 10.0  # 10-second metric aggregation loops
        
        # Tracking buckets
        self.deauth_count = 0
        self.disassoc_count = 0
        self.beacon_sources = Counter()

    def process_packet(self, pkt):
        # Verify the packet contains 802.11 Layer-2 geometry flags
        if not pkt.haslayer(Dot11):
            return

        current_time = time.time()
        
        # Check if our 10-second calculation window has elapsed
        if current_time - self.window_start >= self.window_duration:
            self.evaluate_window_metrics()
            # Reset counters for the next window
            self.deauth_count = 0
            self.disassoc_count = 0
            self.beacon_sources.clear()
            self.window_start = current_time

        # --- PARSE PACKET TYPE MAC OVERRIDES ---
        # 1. Catch Deauthentication Frames (Subtype 12 / 0xC0)
        if pkt.haslayer(Dot11Deauth):
            self.deauth_count += 1
            
        # 2. Catch Disassociation Frames (Subtype 10 / 0xA0)
        elif pkt.haslayer(Dot11Disas):
            self.disassoc_count += 1
            
        # 3. Catch Beacon Management Frames (Subtype 8 / 0x80)
        elif pkt.haslayer(Dot11Beacon):
            ap_mac = pkt[Dot11].addr2 # Source Address (BSSID)
            if ap_mac:
                self.beacon_sources[ap_mac] += 1

    def evaluate_window_metrics(self):
        print(f"\n[i] Aggregation Window Closed. Analyzing telemetry...")
        print(f"    |-- Deauth Count   : {self.deauth_count}")
        print(f"    |-- Disassoc Count : {self.disassoc_count}")
        print(f"    |-- Unique Beacons : {len(self.beacon_sources)} active Transmitters")

        # --- RULE DETECTION PARSER ENGINE ---
        # Deauth Flood Indicator Analysis
        if self.deauth_count > DEAUTH_THRESHOLD:
            print("\033[91m" + "="*60 + "\033[0m")
            print(f"\033[91m[ALERT] CRITICAL WIRELESS DOS TRACKED!")
            print(f"        -> Reason  : Active Deauthentication Frame Flood.")
            print(f"        -> Volume  : Logged {self.deauth_count} packets in 10s window.")
            print("\033[91m" + "="*60 + "\033[0m")

        # Disassociation Flood Indicator Analysis
        if self.disassoc_count > DEAUTH_THRESHOLD:
            print("\033[91m" + "="*60 + "\033[0m")
            print(f"\033[91m[ALERT] SYSTEM ANOMALY TRACKED!")
            print(f"        -> Reason  : Active Disassociation Frame Flood.")
            print(f"        -> Volume  : Logged {self.disassoc_count} packets in 10s window.")
            print("\033[91m" + "="*60 + "\033[0m")

        # Beacon Flood Indicator Analysis
        total_beacons = sum(self.beacon_sources.values())
        if total_beacons > BEACON_THRESHOLD and len(self.beacon_sources) > 5:
            print("\033[91m" + "="*60 + "\033[0m")
            print(f"\033[91m[ALERT] HIGH DENSITY BEACON INTERFERENCE DETECTED!")
            print(f"        -> Reason  : Beacon Flood (Fake Networks Spawned).")
            print(f"        -> Volume  : Catching {total_beacons} total frames across multiple unique random MACs.")
            print("\033[91m" + "="*60 + "\033[0m")

    def run(self):
        print(f"[*] Starting Wireless DoS Detection Engine on interface: {self.interface}")
        print("[*] Monitoring airwave frequencies. Press Ctrl+C to stop.")
        # Sniff loop binds low-level scapy sockets directly onto the monitor interface
        sniff(iface=self.interface, prn=self.process_packet, store=0)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: sudo python3 {sys.argv[0]} <monitor_interface>")
        sys.exit(1)
        
    monitor_iface = sys.argv[1]
    engine = WirelessDetectionEngine(monitor_iface)
    engine.run()

