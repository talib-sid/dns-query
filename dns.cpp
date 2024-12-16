#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <winsock.h>
#include "dns.hpp"

// Search in cache (will implement later)
template <typename Cache>
void search(Cache&);

// src: https://www.iana.org/domains/root/servers
// IP Addresses of all 13 root name servers around the world

const std::vector<std::string> RootServer_IPv4AdrrList {
    "198.41.0.4",
    "170.247.170.2",
    "199.7.91.13",
    "192.203.230.10",
    "192.5.5.241",
    "192.112.36.4",
    "198.97.190.53",
    "192.36.148.17",
    "192.58.128.30",
    "193.0.14.129",
    "199.7.83.42",
    "202.12.27.33"
};

// DNS Resolver
// Recursively call on the RootServers

void queryRootServer(const std::string& rootServerIP, std::string& domainName){
    // opening a UDP Socket
    SOCKET udpSocket = socket(AF_INET,SOCK_DGRAM,0);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    //
    // Root Sv Address
    //
    sockaddr_in rootServerAddress{};
    rootServerAddress.sin_family = AF_INET;
    rootServerAddress.sin_port = htons(53);
    // Convert IP string to binary
    rootServerAddress.sin_addr.s_addr = inet_addr(rootServerIP.c_str()); 
        if (rootServerAddress.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Invalid IP address." << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }

    //
    // Creating the packet
    //

    DNSHeader dnsHeader;
    dnsHeader.id = htons(1); // Identification
    dnsHeader.flags = htons(0x0100); // Standard query
    dnsHeader.qdcount = htons(1); // One question

    DNSQuestion dnsQuestion;
    dnsQuestion.qname = domainName;
    dnsQuestion.qtype = htons(1); // Type A
    dnsQuestion.qclass = htons(1); // Class IN

    // Serialize DNS query
    std::vector<u_char> dnsQuery(sizeof(DNSHeader) + domainName.size() + 2 + sizeof(uint16_t) * 2);
    memcpy(dnsQuery.data(), &dnsHeader, sizeof(DNSHeader));
    size_t offset = sizeof(DNSHeader);

    // Convert domain name to DNS format
    size_t pos = 0;
    while ((pos = domainName.find('.')) != std::string::npos) {
        dnsQuery[offset++] = pos;
        memcpy(dnsQuery.data() + offset, domainName.c_str(), pos);
        offset += pos;
        domainName.erase(0, pos + 1);
    }

    dnsQuery[offset++] = domainName.size();
    memcpy(dnsQuery.data() + offset, domainName.c_str(), domainName.size());
    offset += domainName.size();
    dnsQuery[offset++] = 0;

    // Add QTYPE and QCLASS
    memcpy(dnsQuery.data() + offset, &dnsQuestion.qtype, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    memcpy(dnsQuery.data() + offset, &dnsQuestion.qclass, sizeof(uint16_t));

    // Send the query now
    int sendResult = sendto(udpSocket, reinterpret_cast<const char*>(dnsQuery.data()), dnsQuery.size(), 0, reinterpret_cast<sockaddr*>(&rootServerAddress), sizeof(rootServerAddress));
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }

    // Receive response
    char recvBuffer[512];
    sockaddr_in fromAddr;
    int fromAddrSize = sizeof(fromAddr);
    int recvResult = recvfrom(udpSocket, recvBuffer, sizeof(recvBuffer), 0, reinterpret_cast<sockaddr*>(&fromAddr), &fromAddrSize);
    if (recvResult == SOCKET_ERROR) {
        std::cerr << "recvfrom failed with error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return;
    }
    std::cout << "Received " << recvResult << " bytes from " << inet_ntoa(fromAddr.sin_addr) << "\n";

    // for (ssize_t i = 0; i < recvResult; ++i) {
    //     printf("%02x ", recvBuffer[i]);
    // }
    // std::cout << std::endl;
    // Sleep(100);
    std::cout << "Received response from root server: " << rootServerIP << std::endl;

    // Cleanup
    closesocket(udpSocket);
    // WSACleanup();

}
int main(){
    // Initialise WINSOCK
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
    }

    std::string domainName = "example.com";
    for (const auto& rootServerIP : RootServer_IPv4AdrrList) {
        queryRootServer(rootServerIP, domainName);
        Sleep(100);
    }


    // Close WINSOCK
    WSACleanup();
}