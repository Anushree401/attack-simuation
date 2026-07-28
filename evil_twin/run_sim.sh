#!/bin/bash

# --- CONFIGURATION ---
INTERFACE="wlp0s20f3" 
AIRGEDDON_DIR="/tmp/airgeddon"

# Ensure the script is run with root privileges
if [ "$EUID" -ne 0 ]; then
  echo "[-] Error: This script must be run as root. Use: sudo $0"
  exit 1
fi

echo "[*] Initializing automated environment setup..."

# 1. Install standard system utility dependencies automatically
echo "[*] Checking and installing core system packages..."
apt update && apt install git wireless-tools net-tools tmux iw -y
if [ $? -ne 0 ]; then
    echo "[-] Error: Failed to install required utility packages. Check network connection."
    exit 1
fi

# 2. Download the automation framework natively if not present
if [ ! -d "$AIRGEDDON_DIR" ]; then
    echo "[*] Downloading automated framework code from repository..."
    git clone --depth 1 https://github.com "$AIRGEDDON_DIR"
    if [ $? -ne 0 ]; then
        echo "[-] Error: Git clone failed. Verify internet connection and try again."
        exit 1
    fi
fi

# 3. Release interface control from the system network manager
echo "[*] Releasing $INTERFACE from NetworkManager locks..."
nmcli device set "$INTERFACE" managed no

# 4. Clear conflicting network sockets to free up ports
echo "[*] Clearing conflicting network ports..."
systemctl stop systemd-resolved 2>/dev/null
systemctl stop dnsmasq 2>/dev/null
killall dnsmasq 2>/dev/null
fuser -k 53/udp 67/udp 80/tcp 2>/dev/null

# 5. Verify interface presence and reset state
if ! ip link show "$INTERFACE" &> /dev/null; then
    echo "[-] Error: Interface $INTERFACE not found. Check 'iw dev' output."
    nmcli device set "$INTERFACE" managed yes
    systemctl start systemd-resolved 2>/dev/null
    exit 1
fi

echo "[*] Resetting wireless interface state..."
ip link set "$INTERFACE" down
iw "$INTERFACE" set type managed
ip link set "$INTERFACE" up

echo "[+] Environment preparation complete."
echo "[*] Launching dynamic automation framework..."
echo "--------------------------------------------------"
echo "NEXT STEPS IN MENU:"
echo "1. Select interface: Choose the number for $INTERFACE"
echo "2. Select Option 2 to put the interface into Monitor Mode."
echo "3. Select Option 9 to open the Evil Twin Attacks Menu."
echo "--------------------------------------------------"
sleep 3

# 6. Execute the interactive framework
bash "${AIRGEDDON_DIR}/airgeddon.sh"

# --- CLEANUP BLOCK ---
# Runs automatically when you exit the framework or press Ctrl+C
echo ""
echo "[-] Restoring system defaults and network interfaces..."
ip link set "$INTERFACE" down
iw "$INTERFACE" set type managed
ip link set "$INTERFACE" up
nmcli device set "$INTERFACE" managed yes

# Restart standard everyday networking services
systemctl start systemd-resolved 2>/dev/null
systemctl start dnsmasq 2>/dev/null

echo "[+] Cleanup complete. Your laptop's standard Wi-Fi has been restored."
