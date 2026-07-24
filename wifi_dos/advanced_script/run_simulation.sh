#!/bin/bash

# Ensure the script runs with root privileges
if [ "$EUID" -ne 0 ]; then
  echo "[-] Please run as root (sudo)."
  exit 1
fi

INTERFACE=$1
MODE=$2
SOURCE_FILE="wifi_flood.cpp"
OUTPUT_BIN="./wifi_flood"

# Verify basic arguments
if [ -z "$INTERFACE" ] || [ -z "$MODE" ]; then
    echo "Usage: sudo $0 <interface> <mode> [additional_args]"
    echo "Modes: deauth, disassoc, beacon"
    echo "Example: sudo $0 wlan0 deauth FF:FF:FF:FF:FF:FF 11:22:33:44:55:66"
    exit 1
fi

# Step 1: Install dependencies and compile the source if needed
if [ ! -f "$OUTPUT_BIN" ]; then
    echo "[*] Compiling $SOURCE_FILE..."
	if ! command -v g++ &> /dev/null || [ ! -f /usr/include/pcap.h ]; then
    		echo "[*] Installing required build dependencies..."
    		apt-get update && apt-get install -y build-essential libpcap-dev
	fi    
    g++ -O2 "$SOURCE_FILE" -o "$OUTPUT_BIN" -lpcap
    if [ $? -ne 0 ]; then
        echo "[-] Compilation failed. Ensure $SOURCE_FILE exists and contains valid C++ code."
        exit 1
    fi
    echo "[+] Compilation successful."
fi

# Step 2: Clear conflicting network managers and enable Monitor Mode
echo "[*] Disabling network conflicts and enabling monitor mode on $INTERFACE..."
ip link set "$INTERFACE" down
iw dev "$INTERFACE" set type monitor
ip link set "$INTERFACE" up

# Verify status
CURRENT_MODE=$(iw dev "$INTERFACE" info | grep type | awk '{print $2}')
if [ "$CURRENT_MODE" != "monitor" ]; then
    echo "[-] Failed to set $INTERFACE to monitor mode. Hardware may not support injection."
    exit 1
fi
echo "[+] $INTERFACE is now in monitor mode."

# Step 3: Shift arguments and pass remaining fields to the compiled binary
shift # Remove interface
echo "[*] Executing wireless engine simulation..."
"$OUTPUT_BIN" "$INTERFACE" "$@"
