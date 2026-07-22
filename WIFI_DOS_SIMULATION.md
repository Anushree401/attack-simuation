# Wireless Denial of Service (Wi-Fi DoS) Telemetry & Resilience Simulation

> [!CAUTION]
> **LEGAL & ETHICAL NOTICE — AUTHORIZED LABORATORY USE ONLY**
> 
> This software and documentation are created exclusively for **educational research, academic study, and authorized wireless network resilience benchmarking** within controlled, isolated lab environments.
> 
> **STRICT PROHIBITIONS & DISCLAIMER OF RESPONSIBILITY:**
> - **DO NOT** execute these scripts, raw packet injectors, or orchestration scripts against any wireless network, Access Point (AP), client device, or radio spectrum that you do not personally own or have explicit written authorization to test.
> - **DO NOT** deploy or use this software to disrupt wireless communications, conduct unauthorized Wi-Fi deauthentication attacks, or violate applicable wireless communications laws and regulations (e.g., FCC regulations).
> - **DISCLAIMER OF LIABILITY:** The author/creator of this project assumes **NO RESPONSIBILITY OR LIABILITY** for any misuse, illegal activity, operational disruption, signal interference, or legal consequences resulting from the use or distribution of this code.

---

## Overview

The `wifi_dos/` module provides a laboratory benchmark environment to study IEEE 802.11 Layer 2 wireless management frame dynamics, hardware monitor mode behavior, packet injection throughput, and wireless network resilience under synthetic load. By analyzing how client wireless interfaces and Access Points (APs) react to management and control frame saturation, security researchers can evaluate wireless defenses such as IEEE 802.11w Protected Management Frames (PMF) and WPA3 security standards.

---

## Module Structure

| File / Component | Type | Description |
| :--- | :--- | :--- |
| [`wifi_dos/wifi_flood.cpp`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/wifi_dos/wifi_flood.cpp) | C++ Source | Low-level 802.11 raw packet injection engine utilizing `libpcap` for Deauthentication, Disassociation, and Beacon flooding. |
| [`wifi_dos/run_wifi_lab.sh`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/wifi_dos/run_wifi_lab.sh) | Shell Script | Automated lab orchestrator managing interface discovery, monitor mode toggling, daemon isolation (`NetworkManager`), and system recovery. |
| [`wifi_dos/monitor.sh`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/wifi_dos/monitor.sh) | Shell Script | Real-time telemetry monitoring dashboard for tracking CPU usage, hardware interrupt overhead (`Intr %`), throughput, and packet filter states (`pfctl`). |
| [`wifi_dos/wifi_flood_guide.txt`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/wifi_dos/wifi_flood_guide.txt) | Documentation | Operational CLI reference guide listing vector syntax, symptom analysis, and mitigation mechanisms. |

---

## Requirements & Compilation

### Prerequisites
* **Operating System:** Linux OS (Ubuntu / Debian / Arch / Kali) with kernel support for raw 802.11 socket injection.
* **Hardware:** Physical Wi-Fi network interface card (NIC) supporting IEEE 802.11 Monitor Mode and Frame Injection.
* **Privileges:** Root / `sudo` privileges (required for raw packet socket operations and hardware interface mode switching).
* **Dependencies:** `libpcap-dev`, `g++`, `iw`, `iproute2`, `systemd` (`NetworkManager` / `wpa_supplicant`).

### Building the Wireless Injection Engine
To compile the wireless raw frame engine inside the `wifi_dos/` directory:

```bash
cd wifi_dos
g++ -O3 wifi_flood.cpp -o wifi_engine -lpcap
```

---

## Lab Orchestration & Usage

### 1. Interactive Wireless Lab Suite
The `run_wifi_lab.sh` script automates interface detection, stops conflicting networking daemons, switches the Wi-Fi card to Monitor Mode, and presents an interactive vector menu:

```bash
cd wifi_dos
sudo ./run_wifi_lab.sh
```

### 2. Emergency / Manual Network Restoration
If the lab terminates or is interrupted, restore the physical Wi-Fi interface back to Managed Mode and restart networking daemons using:

```bash
sudo ./run_wifi_lab.sh --restore
```

---

## Wireless Vectors, Telemetry & Defensive Observations

The `wifi_dos` module benchmarks three distinct 802.11 Layer 2 simulation vectors:

### 1. Deauthentication Frame Flooding
* **Vector Mechanics:** Generates spoofed IEEE 802.11 Deauthentication management frames (Reason Code 7: *Class 3 frame received from nonassociated STA*) targeting the client-AP association pair.
* **Observed Symptom:** Instant loss of Wi-Fi connection on targeted client devices.
* **Defensive Countermeasure:** Deploy **IEEE 802.11w Protected Management Frames (PMF)**. PMF adds cryptographic MAC headers to management frames, causing unauthenticated deauthentication frames to be ignored by clients and APs.

### 2. Disassociation Frame Flooding
* **Vector Mechanics:** Sends spoofed Disassociation notification frames stating the client is tearing down its link state.
* **Observed Symptom:** Client enters a perpetual connection loop, forcing constant 4-way handshake re-authentication attempts.
* **Defensive Countermeasure:** Mandate **WPA3-SAE (Simultaneous Authentication of Equals)** authentication standards, which strictly mandate 802.11w PMF protection across all connected endpoints.

### 3. Beacon Frame Flooding
* **Vector Mechanics:** Injects thousands of fake Access Point (AP) SSID advertisement beacon frames into the wireless channel.
* **Observed Symptom:** Client-side wireless interface driver parses thousands of incoming BSSIDs as hardware scanning interrupts, causing local Wi-Fi selection menus to freeze or crash.
* **Defensive Countermeasure:** Implement **Client-Side Signal Strength (RSSI) Threshold Filtering** and deploy Wireless Intrusion Detection/Prevention Systems (WIDS/WIPS) to detect high-density beacon anomalies.
