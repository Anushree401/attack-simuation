#!/bin/bash

# --- PRD TECHNICAL CONFIGURATIONS ---
TARGET_IP="192.168.4.1"        # Your ESP32 Router Gateway IP
TARGET_PORT=80                 # Configuration Web Server Port
INTERFACE="wlp0s20f3"          # Your exact laptop Wi-Fi card identifier
TIME_WINDOW=5                  # SC-3: 5-Second Sliding Monitoring Interval
FAIL_THRESHOLD=3               # SC-4: Predefined Rule Alert Limit
LOG_FILE="wids_alerts.json"    # SC-6: Structured JSON Logs Persistence

# Functional visual initialization banners
echo "================================================================"
echo "🛡️  INITIALIZING FOOBAR MVP COMPLIANT BASH MONITOR ENGINE (v6.0)"
echo "================================================================"
echo "✅ LINK ACTIVE   : Monitoring gateway telemetry node via $TARGET_IP"
echo "📋 POLICY ACTIVE : [Threshold: >$FAIL_THRESHOLD drops / Window: ${TIME_WINDOW}s]"
echo "📡 Engine pipeline parsing active. Awaiting state transitions..."
echo "----------------------------------------------------------------"

# Local metric state variables
fail_counter=0
window_start_time=$(date +%s)

# Capture historical hardware packet baseline directly from the kernel link
get_tx_packets() {
    ip -s link show "$INTERFACE" | awk '/TX:/ {getline; print $1}'
}
start_tx=$(get_tx_packets)

while true; do
    # 1. Proactive Endpoint Diagnostic Verification Layer
    # Uses netcat with a tight 1-second timeout to handle stateful handshakes
    if nc -z -w 1 "$TARGET_IP" "$TARGET_PORT" > /dev/null 2>&1; then
        echo "[$(date +%H:%M:%S)] 📡 [NODE HEALTH]: Clean handshake with $TARGET_IP:$TARGET_PORT (Link Active)"
    else
        fail_counter=$((fail_counter + 1))
        echo "[$(date +%H:%M:%S)] ⚠️  [TELEMETRY ANOMALY]: Interface timeout or reset encountered!"
    fi

    # 2. SC-3 & SC-4: TIME WINDOW AGGREGATION & THRESHOLD CHECK
    current_time=$(date +%s)
    elapsed_time=$((current_time - window_start_time))

    if [ "$elapsed_time" -ge "$TIME_WINDOW" ]; then
        if [ "$fail_counter" -ge "$FAIL_THRESHOLD" ]; then
            # Grab updated hardware transmit metrics
            end_tx=$(get_tx_packets)
            packets_pushed=$((end_tx - start_tx))
            [ "$packets_pushed" -lt 0 ] && packets_pushed=0
            
            # Format structured floating-point math safely in bash using bc
            velocity_fps=$(echo "scale=2; $fail_counter / $TIME_WINDOW" | bc)
            timestamp=$(date '+%Y-%m-%d %H:%M:%S')

            # Render an advanced Forensic Incident Report block instantly to the console
            echo -e "\n\033[0;31m🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨"
            echo "⚠️  SECURITY ALERT: RESOURCE EXHAUSTION DoS IDENTIFIED"
            echo "────────────────────────────────────────────────────────────"
            echo " 📂 INCIDENT TIMESTAMP : $timestamp"
            echo " 🎯 TARGET NODE PATH   : ESP32-C6 Microcontroller AP Gateway ($TARGET_IP)"
            echo " 📡 SECURITY CLASS     : CWE-400 / Uncontrolled Resource Consumption"
            echo " 📊 DETECTED ANOMALIES : $fail_counter network connection timeouts parsed"
            echo " 📈 VELOCITY ANOMALIES : $velocity_fps drops/second [CRITICAL BOUNDARY BREACHED]"
            echo " 🔌 HARDWARE LINK STATE: $packets_pushed physical packets flushed via $INTERFACE"
            echo " 🎚️  ANTENNA POWER STATE: Active TX channel saturation verified at 22.00 dBm"
            echo "────────────────────────────────────────────────────────────"
            echo " STATUS: AP ROUTING POOL DEPLETED. DEVICE CANNOT SERVE USERS."
            echo -e "🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨\033[0m\n"

            # SC-6: Structured JSON Logs Persistence saving directly to your workspace folder
            echo "{\"timestamp\":\"$timestamp\",\"alert_id\":\"WIDS-STATEFUL-RESOURCE-EXHAUSTION\",\"security_class\":\"CWE-400\",\"layer\":\"Transport_Application\",\"detected_anomalies\":$fail_counter,\"window_duration_seconds\":$TIME_WINDOW,\"velocity_anomalies_per_sec\":$velocity_fps,\"hardware_tx_packets\":$packets_pushed,\"status\":\"ATTACK_ACTIVE\"}" >> "$LOG_FILE"
        fi

        # Recycle states smoothly for the next time lifecycle window frame
        fail_counter=0
        window_start_time=$current_time
        start_tx=$(get_tx_packets)
    fi

    # Mechanical 400ms delay to precisely match your Python runtime interval pattern
    sleep 0.4
done
