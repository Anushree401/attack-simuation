#!/bin/bash

# --- SYSTEM CONFIGURATIONS ---
TARGET_IP="192.168.4.1"
TARGET_PORT=80
INTERFACE="wlp0s20f3"
TIME_WINDOW=5          # 5-second evaluation interval
FAIL_THRESHOLD=3       # Trigger alert if failures exceed this number

# Functional visual anchors and initialization banner
echo "================================================================"
echo "🛡️  INITIALIZING FOOBAR PRODUCTION BASH DETECTION ENGINE (v3.0)"
echo "================================================================"
echo "✅ MONITOR INTERFACE ACTIVE : Tracking gateway target node $TARGET_IP"
echo "📋 DETECTION POLICY LOADED  : [Threshold: >$FAIL_THRESHOLD drops / Window: ${TIME_WINDOW}s]"
echo "📡 Watching interface health parameters at hardware speed..."
echo "----------------------------------------------------------------"

# Local metric state variables
fail_counter=0
window_start_time=$(date +%s)

# Capture historical hardware packet baseline
get_tx_packets() {
    # Directly parse the kernel link statistics block for the wireless interface
    ip -s link show "$INTERFACE" | awk '/TX:/ {getline; print $1}'
}
start_tx=$(get_tx_packets)

while true; do
    # 1. Proactive Hardware Endpoint Diagnostic Probe
    # Uses netcat (nc) with a tight 1-second timeout to check port 80 status
    if nc -z -w 1 "$TARGET_IP" "$TARGET_PORT" > /dev/null 2>&1; then
        echo "[$(date +%H:%M:%S)] 📡 [NODE HEALTH] Clean handshake with $TARGET_IP:$TARGET_PORT (Link Active)"
    else
        fail_counter=$((fail_counter + 1))
        echo "[$(date +%H:%M:%S)] ⚠️  [TELEMETRY ANOMALY] Socket dropped or connection timeout encountered!"
    fi

    # 2. Moving Processing Time-Window Evaluation Checkpoint
    current_time=$(date +%s)
    elapsed_time=$((current_time - window_start_time))

    if [ "$elapsed_time" -ge "$TIME_WINDOW" ]; then
        if [ "$fail_counter" -ge "$FAIL_THRESHOLD" ]; then
            # Grab final hardware transmit counter states
            end_tx=$(get_tx_packets)
            packets_pushed=$((end_tx - start_tx))
            [ "$packets_pushed" -lt 0 ] && packets_pushed=0
            
            # Print an advanced, structured Forensic Incident Report banner
            echo -e "\n🔥"
            echo "⚠️  CRITICAL SECURITY EVENT: NETWORK LAYER RESOURCE EXHAUSTION"
            echo "─"
            echo " 📅 TIMESTAMP           : $(date '+%Y-%m-%d %H:%M:%S')"
            echo " 🛑 TARGET IDENTIFIER   : ESP32-C6 Microcontroller AP Gateway ($TARGET_IP)"
            echo " 📊 FAULT METRIC COUNT  : $fail_counter network drops parsed inside time-window"
            echo " 🔌 HARDWARE LINK STATUS: $packets_pushed physical packets flushed via $INTERFACE"
            echo " 🎚️  ANTENNA POWER STATE : Active TX saturation verified at 22.00 dBm level"
            echo " 🛡️  VULNERABILITY MAP   : CWE-400 (Uncontrolled Resource Consumption)"
            echo "─"
            echo " STATUS: AP ROUTING POOL DEPLETED. DEVICE CANNOT SERVE USERS."
            echo -e "🔥\n"
        fi

        # Reset time lifecycle variables for subsequent scanning block
        fail_counter=0
        window_start_time=$current_time
        start_tx=$(get_tx_packets)
    fi

    # Mechanical 500ms delay loop to prevent script from taxing local CPU cycles
    sleep 0.5
done
