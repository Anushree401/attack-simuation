# Security & Attack Simulation Lab

> [!CAUTION]
> **LEGAL & ETHICAL NOTICE — AUTHORIZED LABORATORY USE ONLY**
> 
> This repository is maintained exclusively for **educational research, system resilience testing, and hands-on learning** within isolated lab environments. 
> 
> **STRICT PROHIBITIONS & DISCLAIMER:**
> - **DO NOT** use these scripts or tools against target hosts, networks, or devices without explicit written authorization.
> - **DISCLAIMER OF LIABILITY:** The author assumes **no liability or responsibility** for misuse, network disruption, or legal consequences resulting from the execution of code in this repository.

---

## About This Repository

This repository serves as a personal laboratory playground to experiment with security attack simulations, study low-level network behaviors, monitor kernel telemetry, and learn how defensive controls mitigate distinct security vectors.

By simulating various network and application attack patterns in controlled environments, this project aims to provide practical insight into system metrics, socket state queues, resource exhaustion, and security mitigation techniques.

---

## Simulated Attack Modules

### 1. Denial of Service (DoS) Simulation ([`dos/`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/dos))

**Attack Concept:**  
A **Denial of Service (DoS)** attack aims to render a target machine, service, or network infrastructure unavailable to legitimate users by overwhelming it with an excessive volume of traffic or resource-intensive requests. DoS attacks typically exploit specific protocol behaviors (such as uncompleted TCP handshakes in SYN floods), consume network interface bandwidth (UDP datagram floods), or trigger high CPU software interrupt (`SoftIRQ`) overhead (ICMP ping floods). Studying DoS dynamics in a lab allows security engineers to analyze telemetry metrics, observe kernel queue bottlenecks, and test defensive countermeasures such as SYN cookies, firewall rate-limiting, and eBPF/XDP packet filtering.

> 📖 **Detailed Guide:** For setup, compilation, and telemetry metrics, refer to [DOS_SIMULATION.md](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/DOS_SIMULATION.md).

### 2. Wireless DoS (802.11) Simulation ([`wifi_dos/`](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/wifi_dos))

**Attack Concept:**  
A **Wireless Denial of Service (Wi-Fi DoS)** attack targets IEEE 802.11 Layer 2 management and control frame mechanisms to disrupt wireless link state associations or saturate hardware scanning interrupts. Because unencrypted 802.11 management frames (such as Deauthentication and Disassociation) can be spoofed in legacy Wi-Fi standards, client devices can be involuntarily disconnected from Access Points. Additionally, injecting vast numbers of fake BSSID Beacon frames overwhelms client wireless drivers during scanning procedures. Studying 802.11 DoS dynamics in an isolated lab highlights the security vulnerabilities of unauthenticated wireless management frames and demonstrates the necessity of defensive technologies such as IEEE 802.11w Protected Management Frames (PMF) and WPA3 authentication standards.

> 📖 **Detailed Guide:** For hardware requirements, lab orchestration, and wireless telemetry metrics, refer to [WIFI_DOS_SIMULATION.md](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/WIFI_DOS_SIMULATION.md).

---

## Repository Structure Overview

```text
.
├── README.md               # Main repository overview and module sitemap
├── DOS_SIMULATION.md       # Detailed guide for the DoS telemetry & benchmark suite
├── WIFI_DOS_SIMULATION.md  # Detailed guide for the Wireless 802.11 DoS simulation suite
├── dos/                    # Denial of Service simulation module
│   ├── flood_engine.cpp    # Raw socket multi-threaded traffic generation engine
│   ├── monitor.sh          # Real-time CLI kernel telemetry dashboard
│   └── flood_engine_guide.txt
└── wifi_dos/               # Wireless DoS (802.11) simulation module
    ├── wifi_flood.cpp      # Low-level 802.11 raw packet injection engine (libpcap)
    ├── run_wifi_lab.sh     # Interactive wireless lab orchestrator & monitor mode control
    ├── monitor.sh          # Real-time router & system performance telemetry dashboard
    └── wifi_flood_guide.txt # Operational reference guide for 802.11 simulation vectors
```

---

## Roadmap & Future Modules

This lab will continue to expand with additional simulation modules and learning experiments:

- [x] **Denial of Service (DoS)** (`dos/` / [DOS_SIMULATION.md](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/DOS_SIMULATION.md)) - TCP SYN, UDP, and ICMP benchmark suite with live telemetry monitoring.
- [x] **Wireless DoS (802.11)** (`wifi_dos/` / [WIFI_DOS_SIMULATION.md](file:///media/anushreebalaji/skynet/Psnal/SIDE%20GIGS/PROJECTS/SECURITY/attack_scripts/WIFI_DOS_SIMULATION.md)) - 802.11 Deauthentication, Disassociation, and Beacon flooding lab suite with hardware monitor mode orchestration.
