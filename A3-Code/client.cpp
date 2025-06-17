#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <unistd.h>

// Server and client configuration
#define SERVER_PORT 12345        // The server is listening on port 12345
#define CLIENT_PORT 54321        // Arbitrary ephemeral port for the client (example)
#define SERVER_IP "127.0.0.1"      // Assumed server IP (localhost)
#define CLIENT_IP "127.0.0.1"      // Client IP (localhost)

// Global constants for sequence numbers as per assignment and server.cpp
#define SYN_SEQ 200              // Client's initial sequence number in SYN
#define ACK_SEQ_FINAL 600        // Expected sequence number in the final ACK packet to complete handshake

// Function to create and send a raw packet
// Takes a socket, destination address, packet buffer, and packet length
void send_packet(int sock, struct sockaddr_in &dest, const char *packet, int packet_len) {
    if (sendto(sock, packet, packet_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    }
}

// Function to construct and send a packet (SYN or ACK) using raw sockets
// 'isSyn' flag indicates whether the packet is a SYN packet (true) or an ACK packet (false)
void craft_and_send_packet(int sock, struct sockaddr_in &dest, bool isSyn, uint32_t seq, uint32_t ack) {
    // Buffer to hold both IP and TCP headers.
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    // IP header pointer
    struct iphdr *ip = (struct iphdr *)packet;
    // TCP header pointer, located after the IP header
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // ----- Construct the IP header -----
    ip->ihl = 5;                   // Internet Header Length: 5 * 32-bit words
    ip->version = 4;               // IPv4
    ip->tos = 0;                   // Type of service
    ip->tot_len = htons(sizeof(packet)); // Total length = IP header + TCP header
    ip->id = htons(54321);         // Identification (arbitrary)
    ip->frag_off = 0;              // No fragmentation
    ip->ttl = 64;                  // Time-to-live
    ip->protocol = IPPROTO_TCP;    // Protocol = TCP
    ip->saddr = inet_addr(CLIENT_IP); // Source IP address (client IP)
    ip->daddr = inet_addr(SERVER_IP); // Destination IP address (server IP)

    // ----- Construct the TCP header -----
    tcp->source = htons(CLIENT_PORT);   // Source port (client port)
    tcp->dest = htons(SERVER_PORT);     // Destination port (server port)
    tcp->seq = htonl(seq);              // Sequence number (parameterized)
    tcp->ack_seq = htonl(ack);          // Acknowledgment number (parameterized)
    tcp->doff = 5;                      // Data offset: 5 * 32-bit words (20 bytes header)
    
    // Set flags: for SYN packet, set SYN flag; for ACK, clear SYN and set ACK flag
    if (isSyn) {
        tcp->syn = 1;
        tcp->ack = 0;
    } else {
        tcp->syn = 0;
        tcp->ack = 1;
    }
    tcp->fin = 0;
    tcp->rst = 0;
    tcp->psh = 0;
    
    tcp->window = htons(8192);      // Window size (arbitrary value)
    tcp->check = 0;                 // Checksum set to 0 (kernel may compute if necessary)
    
    // Send the packet using the provided socket and destination address.
    send_packet(sock, dest, packet, sizeof(packet));

    // For logging, print what type of packet was sent.
    if (isSyn)
        std::cout << "[+] Sent SYN with sequence number " << seq << std::endl;
    else
        std::cout << "[+] Sent ACK with sequence number " << seq << " and acknowledgment " << ack << std::endl;
}

int main() {
    // Create a raw socket for sending and receiving TCP packets
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket option to include our own IP header (required for raw sockets)
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // Define server address structure for sending packets
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // --------------------------
    // Step 1: Send SYN Packet
    // --------------------------
    // The client's SYN packet uses sequence number SYN_SEQ (200) and no acknowledgment.
    craft_and_send_packet(sock, server_addr, true, SYN_SEQ, 0);

    // --------------------------
    // Step 2: Receive SYN-ACK Packet
    // --------------------------
    // We now wait for a response from the server.
    // The expected SYN-ACK packet should have the SYN and ACK flags set.
    // The server code uses a sequence number of 400 in its reply and sets ack_seq to (client_seq + 1)
    char buffer[65536];
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    bool receivedSynAck = false;
    uint32_t server_seq = 0;

    while (!receivedSynAck) {
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&src_addr, &addr_len);
        if (data_size < 0) {
            perror("recvfrom() failed");
            continue;
        }

        // Parse the IP header
        struct iphdr *ip = (struct iphdr *)buffer;
        // TCP header located after the IP header (IP header length in bytes = ip->ihl * 4)
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        // Process only packets destined to our client port?
        // However, note that the server code checks for tcp->dest == SERVER_PORT.
        // Since our client sent SYN from CLIENT_PORT, the incoming SYN-ACK will have destination as CLIENT_PORT.
        // Depending on the network, you might need additional filtering.
        if (ntohs(tcp->dest) != CLIENT_PORT)
            continue;

        // Check if this is the SYN-ACK (SYN and ACK flags set)
        if (tcp->syn == 1 && tcp->ack == 1) {
            // Optionally, you could also check the acknowledgement matches (should equal SYN_SEQ+1 which is 201)
            uint32_t ack_num = ntohl(tcp->ack_seq);
            if (ack_num != (SYN_SEQ + 1)) {
                std::cout << "[!] Received SYN-ACK with unexpected acknowledgment number: "
                          << ack_num << std::endl;
                continue;
            }

            server_seq = ntohl(tcp->seq);
            std::cout << "[+] Received SYN-ACK from " << inet_ntoa(src_addr.sin_addr)
                      << " with server sequence number " << server_seq << std::endl;
            receivedSynAck = true;
        }
    }

    // --------------------------
    // Step 3: Send Final ACK
    // --------------------------
    // In our simplified handshake, the assignment requires the client to send an ACK packet with its sequence number set to 600.
    // We also set the acknowledgment field to server's sequence number plus one (i.e., server_seq + 1).
    uint32_t final_ack = server_seq + 1; // Expected: 400 + 1 = 401.
    craft_and_send_packet(sock, server_addr, false, ACK_SEQ_FINAL, final_ack);

    std::cout << "[+] Handshake complete." << std::endl;

    close(sock);
    return 0;
}