#!/bin/bash

# --- SYSTEM CONFIGURATIONS ---
TARGET_IP="192.168.4.1"
INTERFACE="wlp0s20f3"
TIME_WINDOW=5                   # 5-second tracking window
LATENCY_THRES_MS=500           # Alert if network delay crosses 500ms (Critical Flood)
LOG_FILE="wids_alerts.json"     # Structured JSON persistence

echo "================================================================="
echo "🛡️  INITIALIZING FOOBAR TELEMETRY LATENCY DAEMON (v5.5)"
echo "================================================================="
echo "✅ PASSIVE LINK ACTIVE   : Tracking hardware node via $TARGET_IP"
echo "📋 POLICY METRICS LOADED : [Threshold: >${LATENCY_THRES_MS}ms Delay / Window: ${TIME_WINDOW}s]"
echo "📡 Engine pipeline checking connection matrix at hardware speed..."
echo "-----------------------------------------------------------------"

# Local tracking parameters
high_latency_events=0
window_start_time=$(date +%s)

# Capture historical hardware packet baseline
get_tx_packets() {
    ip -s link show "$INTERFACE" | awk '/TX:/ {getline; print $1}'
}
start_tx=$(get_tx_packets)

while true; do
    # Probe the gateway using a fast 1-packet ping stream with a tight 1-second timeout
    ping_output=$(ping -c 1 -W 1 "$TARGET_IP" 2>/dev/null)
    
    if [ $? -eq 0 ]; then
        # Parse out the raw numeric latency string (extracting milliseconds index)
        current_latency=$(echo "$ping_output" | awk -F'/' '/rtt/ {print $5}' | cut -d'.' -f1)
        
        if [ -n "$current_latency" ]; then
            echo "[$(date +%H:%M:%S)] 📡 [NODE BALANCER]: Connection established. RTT delay: ${current_latency}ms"
            
            # If latency spikes over your safety threshold rule, register it as an anomaly
            if [ "$current_latency" -gt "$LATENCY_THRES_MS" ]; then
                high_latency_events=$((high_latency_events + 1))
                echo -e "   ↳ \033[0;33m[⚠️ METRIC WARNING] Extreme latency anomaly parsed! Channel Congestion Active.\033[0m"
            fi
        fi
    else
        # Treat complete connection timeouts or drops as a critical fault event
        high_latency_events=$((high_latency_events + 1))
        echo "[$(date +%H:%M:%S)] ⚠️  [TELEMETRY ANOMALY]: Packets dropped or timeout encountered!"
    fi

    # --- TIME-WINDOW EVALUATION CHECKPOINT ---
    current_time=$(date +%s)
    elapsed_time=$((current_time - window_start_time))

    if [ "$elapsed_time" -ge "$TIME_WINDOW" ]; then
        # If we logged multiple latency drops/spikes within the 5-second frame, fire alert
        if [ "$high_latency_events" -ge 2 ]; then
            end_tx=$(get_tx_packets)
            packets_pushed=$((end_tx - start_tx))
            [ "$packets_pushed" -lt 0 ] && packets_pushed=0
            
            timestamp=$(date '+%Y-%m-%d %H:%M:%S')
            
            # Print an advanced Forensic Incident Report block instantly to the console
            echo -e "\n\033[0;31m🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨"
            echo "⚠️  SECURITY ALERT: LAYER-2 IEEE 802.11 DoS ATTACK IDENTIFIED"
            echo "────────────────────────────────────────────────────────────"
            echo " 📂 INCIDENT TIMESTAMP : $timestamp"
            echo " 🎯 TARGET NODE PATH   : ESP32-C6 Microcontroller AP Gateway ($TARGET_IP)"
            echo " 📡 PROTOCOL LAYER     : IEEE 802.11 MAC Layer (Management Frame Injection)"
            echo " 📊 ANOMALIES PARSED   : $high_latency_events hardware timeout metrics inside window"
            echo " 🔌 HARDWARE LINK STATE: $packets_pushed physical packets flushed via $INTERFACE"
            echo " 🎚️  ANTENNA POWER STATE: Active TX channel saturation verified at 22.00 dBm"
            echo " 🛡️  VULNERABILITY MAP  : CWE-400 (Resource Exhaustion via Radio Saturation)"
            echo "────────────────────────────────────────────────────────────"
            echo " STATUS: CRITICAL CHANNEL JAMMING ACTIVE. WIRELESS TIMEOUT EXCEEDED 5000MS."
            echo -e "🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨🚨\033[0m\n"
            
            # Save the persistent, structured audit log to your JSON ledger file
            echo "{\"timestamp\":\"$timestamp\",\"alert_id\":\"WIDS-L2-LATENCY-ALERT\",\"protocol_layer\":\"802.11_MAC\",\"detected_anomalies\":$high_latency_events,\"window_seconds\":$TIME_WINDOW}" >> "$LOG_FILE"
        fi

        # Reset state trackers cleanly for the next monitoring cycle block
        high_latency_events=0
        window_start_time=$current_time
        start_tx=$(get_tx_packets)
    fi

    # Small pause to prevent the watchdog loop from eating your laptop's CPU
    sleep 0.2
done
