# Denial of Service (DoS) Telemetry & Resilience Simulation

> [!CAUTION]
> **LEGAL & ETHICAL NOTICE — AUTHORIZED LABORATORY USE ONLY**
> 
> This software is created exclusively for **educational, academic research, and authorized network resilience benchmarking** within controlled, isolated lab environments.
> 
> **STRICT PROHIBITIONS & DISCLAIMER OF RESPONSIBILITY:**
> - **DO NOT** execute these scripts or binaries against any target, host, device, or network infrastructure that you do not personally own or have explicit, written authorization to test.
> - **DO NOT** deploy or use this software to perform unauthorized Denial of Service (DoS/DDoS) attacks, disrupt network availability, or violate applicable computer crime laws.
> - **DISCLAIMER OF LIABILITY:** The author/creator of this project assumes **NO RESPONSIBILITY OR LIABILITY** for any misuse, illegal activity, financial loss, system damage, or legal consequences resulting from the use or distribution of this code.

---

## Overview

The `dos/` module provides a laboratory benchmark environment to observe system telemetry, socket queue behavior, and kernel performance under heavy synthetic network load. By monitoring how network interfaces and kernel subsystems behave under stress, network administrators and security researchers can evaluate defensive controls (such as SYN cookies, firewall rate-limits, and queue management).

---

## Module Structure

| File / Component | Type | Description |
| :--- | :--- | :--- |
| [`dos/flood_engine.cpp`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/dos/flood_engine.cpp) | C++ Source | Multi-threaded raw socket traffic generation engine for TCP, UDP, and ICMP benchmarks. |
| [`dos/monitor.sh`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/dos/monitor.sh) | Shell Script | Real-time CLI telemetry dashboard tracking socket state (`SYN-RECV`), packet rates, and CPU SoftIRQ load. |
| [`dos/flood_engine_guide.txt`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/dos/flood_engine_guide.txt) | Documentation | Operational guide and usage documentation for the DoS telemetry suite. |

---

## Requirements & Compilation

### Prerequisites
* Linux OS (Ubuntu / Debian recommended)
* `g++` compiler with C++11 support and POSIX threads (`pthread`)
* Root / `sudo` privileges (required for raw socket operations and socket queue state inspection)
* Dependencies: `mpstat` (from `sysstat`), `ss`, `ip`, `awk`

### Building the Traffic Engine
To compile the benchmark engine inside the `dos/` folder:

```bash
cd dos
g++ -O3 -pthread flood_engine.cpp -o flood_engine
```

---

## Telemetry Metrics & Defensive Observations

The `dos/monitor.sh` dashboard captures live performance indicators to analyze host resilience:

### 1. TCP Handshake Queue (`SYN-RECV`)
* **Metric**: Semi-open TCP connection count observed via `ss -ant`.
* **Behavior**: Tracks backlog saturation during TCP handshake floods.
* **Defensive Strategy**: Enable Linux SYN Cookies (`sysctl -w net.ipv4.tcp_syncookies=1`) to prevent connection table exhaustion.

### 2. UDP Bandwidth & Interface Drops
* **Metric**: Interface RX drops via `/sys/class/net/<interface>/statistics/rx_dropped`.
* **Behavior**: Measures packet loss when datagram rate exceeds socket buffer or ring buffer capacity.
* **Defensive Strategy**: Apply early packet filtering using `iptables`/`nftables` or kernel eBPF/XDP hooks.

### 3. ICMP Processing & CPU SoftIRQ Load
* **Metric**: CPU Software Interrupt (`SoftIRQ`) time via `mpstat` and kernel counters in `/proc/net/snmp`.
* **Behavior**: Monitors CPU load caused by per-packet kernel interrupt processing.
* **Defensive Strategy**: Configure kernel ICMP rate limits (`net.ipv4.icmp_ratelimit`) or drop ICMP echo requests at the perimeter.
