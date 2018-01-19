

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include "Common.hpp"


bool InvalidIP(const std::string& ip)
{
    in_addr_t ret = INADDR_NONE;
    ret = inet_addr(ip.c_str());
    return INADDR_NONE == ret ? true : false;
}

bool InvalidPort(const std::string& port)
{
    unsigned i = 0;
    for ( ;i<port.length(); ++i ) {
        if ( !isdigit(port[i]) ) {
            return true;
        }
    }

    return false;
}

int StringAsInt(const std::string& str)
{
    int i;
    std::stringstream ss;
    ss<<str;
    ss>>i;

    return i;
}

std::string i2s(const int& i)
{
    std::stringstream ss;
    ss << i;
    return ss.str();
}

int s2i(const std::string& s)
{
    int i;
    std::stringstream ss;
    ss<<s;
    ss>>i;
    return i;
}

std::string GetPeerInfo(struct sockaddr_in * paddr)
{
    std::string str;
    str += inet_ntoa(paddr->sin_addr);
    str += ":";
    std::stringstream ss;
    ss<<ntohs(paddr->sin_port);
    str += ss.str();

    return str;
}

std::string GetPeerIP(struct sockaddr_in *paddr)
{
    return std::string(inet_ntoa(paddr->sin_addr));
}

unsigned short GetPeerPort(struct sockaddr_in *paddr)
{
    return ntohs(paddr->sin_port);
}

uint64_t ntohll(const uint64_t& input)
{
    uint64_t rval;
    uint8_t *data = (uint8_t *)&rval;

    data[0] = input>>56;
    data[1] = input>>48;
    data[2] = input>>40;
    data[3] = input>>32;
    data[4] = input>>24;
    data[5] = input>>16;
    data[6] = input>>8;
    data[7] = input>>0;

    return rval;
}

uint64_t htonll(const uint64_t& input)
{
    return (ntohll(input));
}

