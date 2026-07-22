#!/bin/bash

# --- CONFIGURATION & AUTO-DISCOVERY ---
TARGET_PORT=8080

INTERFACE=$(ip -o -4 route show to default | awk '{print $5}')
if [ -z "$INTERFACE" ]; then
    INTERFACE=$(ls /sys/class/net | grep -v lo | head -n 1)
fi

echo "[*] Initializing Full Protocol Telemetry Matrix on interface: $INTERFACE..."
sleep 1.5

# Baseline data collection
PREV_RX_PACKETS=$(cat /sys/class/net/"$INTERFACE"/statistics/rx_packets 2>/dev/null || echo 0)
PREV_ICMP_IN=$(awk '/Icmp:/ {print $2}' /proc/net/snmp 2>/dev/null | tail -n 1 || echo 0)
PREV_TIME=$(date +%s)

while true; do
    CURRENT_TIME=$(date +%s)
    CURRENT_RX_PACKETS=$(cat /sys/class/net/"$INTERFACE"/statistics/rx_packets 2>/dev/null || echo 0)
    CURRENT_ICMP_IN=$(awk '/Icmp:/ {print $2}' /proc/net/snmp 2>/dev/null | tail -n 1 || echo 0)

    # --- NET METRICS ---
    TIME_DELTA=$((CURRENT_TIME - PREV_TIME))
    [ "$TIME_DELTA" -le 0 ] && TIME_DELTA=1
    
    # Total PPS calculation
    PACKET_DELTA=$((CURRENT_RX_PACKETS - PREV_RX_PACKETS))
    PPS=$((PACKET_DELTA / TIME_DELTA))

    # Isolated ICMP PPS calculation
    ICMP_DELTA=$((CURRENT_ICMP_IN - PREV_ICMP_IN))
    ICMP_PPS=$((ICMP_DELTA / TIME_DELTA))

    # Protocol structural counters
    SYN_RECV=$(ss -ant | grep -i "SYN-RECV" | wc -l)
    UDP_SOCKETS=$(ss -anu | grep -c "$TARGET_PORT")
    RX_DROPPED=$(cat /sys/class/net/"$INTERFACE"/statistics/rx_dropped 2>/dev/null || echo 0)
    
    SOFT_IRQ=$(mpstat 1 1 | awk '/Average:/ {print $8}' | sed 's/,/./')
    [ -z "$SOFT_IRQ" ] && SOFT_IRQ="0.00"

    # Rotate tracking baselines
    PREV_RX_PACKETS=$CURRENT_RX_PACKETS
    PREV_ICMP_IN=$CURRENT_ICMP_IN
    PREV_TIME=$CURRENT_TIME

    # --- DYNAMIC PROTOCOL FINGERPRINTING LOGIC ---
    if [ "$PPS" -gt 2500 ]; then
        if [ "$SYN_RECV" -gt 50 ]; then
            VECTOR_TYPE="TCP SYN FLOOD"
        elif [ "$ICMP_PPS" -gt 1000 ]; then
            VECTOR_TYPE="ICMP PING FLOOD"
        elif [ "$UDP_SOCKETS" -gt 0 ] || [ "$RX_DROPPED" -gt 0 ]; then
            VECTOR_TYPE="RAW UDP FLOOD"
        else
            VECTOR_TYPE="GENERIC VOLUMETRIC FLOOD"
        fi
        STATUS_ALERT="\e[1;41;37m [CRITICAL] HOST UNDER ACTIVE $VECTOR_TYPE \e[0m"
    elif [ "$PPS" -gt 800 ]; then
        STATUS_ALERT="\e[1;33m [WARNING] ANOMALOUS TRAFFIC SPIKE \e[0m"
    else
        STATUS_ALERT="\e[1;32m [HEALTHY] TRAFFIC MOVEMENT NORMAL \e[0m"
    fi

    # --- RENDERING ENGINE ---
    clear
    echo -e "=================================================================="
    echo -e "         ADAPTIVE TELEMETRY DETECTOR ENGINE                       "
    echo -e "=================================================================="
    echo -e " Monitored NIC         : $INTERFACE"
    echo -e " Monitored Server Port : $TARGET_PORT"
    echo -e "----------------------------- NETWORK ----------------------------"
    echo -e " Aggregate Packet Rate : $PPS packets/sec"
    echo -e " ICMP (Ping) Rate      : $ICMP_PPS packets/sec"
    echo -e " Interface Dropped     : $RX_DROPPED total packets dropped"
    echo -e "------------------------- KERNEL SOCKETS ------------------------"
    echo -e " Semi-Open (SYN-RECV)  : $SYN_RECV connections"
    echo -e " Target UDP Sockets    : $UDP_SOCKETS active hits"
    echo -e "------------------------ SYSTEM RESOURCES ------------------------"
    echo -e " CPU Software IRQ Load : $SOFT_IRQ% processing capacity spent on NIC"
    echo -e "------------------------------------------------------------------"
    echo -e " ALARM STATUS          : $STATUS_ALERT"
    echo -e "=================================================================="
done
