/**
 * @file       BlynkSocket.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

#ifndef BlynkSocket_h
#define BlynkSocket_h

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include <Blynk/BlynkProtocol.h>

class BlynkTransportSocket
{
public:
    BlynkTransportSocket()
        : sockfd(-1), domain(NULL), port(NULL)
    {}

    void begin(const char* d, const char* p) {
        this->domain = d;
        this->port = p;
    }

    bool connect()
    {
        BLYNK_LOG("Connecting to %s:%s", domain, port);

        struct addrinfo hints;
        struct addrinfo *res;  // will point to the results

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

        // get ready to connect
        getaddrinfo(domain, port, &hints, &res);

        if ((sockfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            BLYNK_LOG("Can't create socket");
            return false;
        }

        if (::connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
        {
            BLYNK_LOG("Can't connect to %s", domain);
            return false;
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

        int one = 1;
        setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

        freeaddrinfo(res); // TODO: Leak here

        return true;
    }

    void disconnect()
    {
        if (sockfd != -1) {
            while (::close(sockfd) < 0) {
                usleep(10000);
            }
            sockfd = -1;
        }
    }

    size_t read(void* buf, size_t len) {
        ssize_t rlen = ::read(sockfd, buf, len);
        if (rlen == -1) {
            //BLYNK_LOG("Read error %d: %s", errno, strerror(errno));
            if (errno == ETIMEDOUT || errno == EWOULDBLOCK || errno == EAGAIN) {
                return 0;
            }
            disconnect();
            return -1;
        }
        return rlen;
    }

    size_t write(const void* buf, size_t len) {
        return ::write(sockfd, buf, len);
    }

    bool connected() { return sockfd >= 0; }
    int available() { return BLYNK_MAX_READBYTES; }

protected:
    int    sockfd;
    const char* domain;
    const char* port;
};

class BlynkSocket
    : public BlynkProtocol<BlynkTransportSocket>
{
    typedef BlynkProtocol<BlynkTransportSocket> Base;
public:
    BlynkSocket(BlynkTransportSocket& transp)
        : Base(transp)
    {}

    void begin(const char* auth,
               const char* domain = BLYNK_DEFAULT_DOMAIN,
               const char* port   = TOSTRING(BLYNK_DEFAULT_PORT))
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

};

#endif
