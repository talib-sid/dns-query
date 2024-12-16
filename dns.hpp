#pragma once
#include <iostream>
#include <stdint.h>

// DNS query
class DNSHeader {
    public:
        uint16_t id; // Identification
        uint16_t flags; // Flags
        uint16_t qdcount; // Number of questions
        uint16_t ancount; // Number of answers
        uint16_t nscount; // Number of authority records
        uint16_t arcount; // Number of additional records

        DNSHeader(){
            ancount = 0;
            nscount = 0;
            arcount = 0;
        }
};

class DNSQuestion {
    public:
        std::string qname; // Query name
        uint16_t qtype; // Query type
        uint16_t qclass; // Query class
        
};

std::string parseResponseFromDNSQuery(char* recvBuffer) {
    DNSHeader dnsHeader;
    memcpy(&dnsHeader, recvBuffer, sizeof(DNSHeader));
    std::string response;

    response += "ID: " + std::to_string(ntohs(dnsHeader.id)) + "\n";
    response += "Flags: " + std::to_string(ntohs(dnsHeader.flags)) + "\n";
    response += "Questions: " + std::to_string(ntohs(dnsHeader.qdcount)) + "\n";
    response += "Answers: " + std::to_string(ntohs(dnsHeader.ancount)) + "\n";
    response += "Authority Records: " + std::to_string(ntohs(dnsHeader.nscount)) + "\n";
    response += "Additional Records: " + std::to_string(ntohs(dnsHeader.arcount)) + "\n";

    return response;
}