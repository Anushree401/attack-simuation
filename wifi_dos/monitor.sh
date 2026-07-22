#!/bin/sh

# Configuration
# Replace 'vtnet1' with the interface connected to your testing network if different
INTERFACE="vtnet1"
INTERVAL=2 # Update interval in seconds

echo "================================================================="
echo "      OPNsense Router Performance & Metric Monitor Script"
echo "      Tracking Interface: $INTERFACE | Interval: ${INTERVAL}s"
echo "================================================================="
echo ""

# Format header for the terminal output
printf "%-10s | %-8s | %-8s | %-10s | %-10s | %-8s\n" \
       "Timestamp" "CPU %" "Intr %" "In Packets" "Out Packets" "FW States"
echo "--------------------------------------------------------------------------------"

while true; do
    # 1. Get Current Timestamp
    TIMESTAMP=$(date +"%H:%M:%S")

    # 2. Extract CPU Idle percentage and calculate CPU Usage
    # FreeBSD top command output syntax used here
    CPU_IDLE=$(top -b -d 1 | grep "CPU:" | awk -F'%' '{print $4}' | awk '{print $NF}')
    if [ -z "$CPU_IDLE" ]; then
        CPU_USAGE="N/A"
    else
        CPU_USAGE=$(echo "100 - $CPU_IDLE" | bc 2>/dev/null)
        [ -z "$CPU_USAGE" ] && CPU_USAGE="N/A"
    fi

    # 3. Extract Interrupt Overhead (high rate packets cause high interrupts)
    INTR_USAGE=$(top -b -d 1 | grep "CPU:" | awk -F'%' '{print $3}' | awk '{print $NF}')
    [ -z "$INTR_USAGE" ] && INTR_USAGE="0.0"

    # 4. Grab Cumulative Interface Packet Counters
    # Uses netstat to get total inbound/outbound packets for the target interface
    NETSTAT_OUT=$(netstat -I "$INTERFACE" -b -d -w 1 2>/dev/null | tail -n 1)
    IN_PKTS=$(echo "$NETSTAT_OUT" | awk '{print $5}')
    OUT_PKTS=$(echo "$NETSTAT_OUT" | awk '{print $8}')

    # Default to 0 if netstat output couldn't be parsed properly
    [ -z "$IN_PKTS" ] && IN_PKTS="0"
    [ -z "$OUT_PKTS" ] && OUT_PKTS="0"

    # 5. Get Active Packet Filter (pf) States
    FW_STATES=$(pfctl -si 2>/dev/null | grep "current entries" | awk '{print $3}')
    [ -z "$FW_STATES" ] && FW_STATES="N/A"

    # 6. Print Formatted Live Row
    printf "%-10s | %-7s%% | %-7s%% | %-10s | %-10s | %-8s\n" \
           "$TIMESTAMP" "$CPU_USAGE" "$INTR_USAGE" "$IN_PKTS" "$OUT_PKTS" "$FW_STATES"

    sleep "$INTERVAL"
done
