#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <pcap.h>
#include <unistd.h>

const uint8_t eth_radiotap[26] = {
    0x00, 0x00, 0x1a, 0x00, 0x2f, 0x40, 0x00, 0xa0, 
    0x20, 0x08, 0x00, 0x00, 0x00, 0x02, 0x85, 0x09, 
    0xa0, 0x04, 0xb0, 0x00, 0x10, 0x00, 0x00, 0x00, 
    0x10, 0x00
};

int main() {
    // --- HARDCODED TARGETS ---
    const char* wifi_card = "wlp0s20f3";
    std::string custom_ssid = "ESP32_Lab_Simulation"; // Custom scannable beacon tag
    uint8_t air_broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* air_tx = pcap_open_live(wifi_card, BUFSIZ, 1, 1, errbuf);
    if (!air_tx) {
        std::cerr << "[-] Driver interface allocation failed: " << errbuf << std::endl;
        return 1;
    }

    std::cout << "[*] ENGINE ACTIVE -> BROADCASTING BEACON CELLS FOR: " << custom_ssid << "\n";
    uint64_t frame_count = 0;
    std::srand(std::time(nullptr));

    while (true) {
        std::vector<uint8_t> raw_frame;
        raw_frame.insert(raw_frame.end(), eth_radiotap, eth_radiotap + 26);

        uint8_t frame_control[2] = {0x80, 0x00}; // Type: Management, Subtype: Beacon
        uint8_t frame_duration[2] = {0x00, 0x00};
        
        // Randomize BSSID to populate wireless scans with dynamic cell footprints
        uint8_t dynamic_bssid[6] = {0x00, 0x11, 0x22, 0x33, (uint8_t)(rand() % 255), (uint8_t)(rand() % 255)};

        raw_frame.insert(raw_frame.end(), frame_control, frame_control + 2);
        raw_frame.insert(raw_frame.end(), frame_duration, frame_duration + 2);
        raw_frame.insert(raw_frame.end(), air_broadcast, air_broadcast + 6); 
        raw_frame.insert(raw_frame.end(), dynamic_bssid, dynamic_bssid + 6);    
        raw_frame.insert(raw_frame.end(), dynamic_bssid, dynamic_bssid + 6);    
        
        uint8_t sequence_control[2] = {0x00, 0x00};
        raw_frame.insert(raw_frame.end(), sequence_control, sequence_control + 2);

        uint8_t timestamp_capabilities[12] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x64,0x00, 0x11,0x04};
        raw_frame.insert(raw_frame.end(), timestamp_capabilities, timestamp_capabilities + 12);

        // Element ID 0: SSID parameter
        raw_frame.push_back(0x00); 
        raw_frame.push_back(custom_ssid.length()); 
        raw_frame.insert(raw_frame.end(), custom_ssid.begin(), custom_ssid.end());

        // Element ID 1: Supported Rates baseline
        uint8_t connection_rates[10] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
        raw_frame.insert(raw_frame.end(), connection_rates, connection_rates + 10);

        pcap_sendpacket(air_tx, raw_frame.data(), raw_frame.size());
        frame_count++;

        if (frame_count % 500 == 0) {
            std::cout << "\r[+] Broadcast benchmark: " << frame_count << " beacons deployed..." << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    pcap_close(air_tx);
    return 0;
}
