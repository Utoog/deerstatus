#include "ping.h"
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/ip_icmp.h>
#include <fcntl.h>
#include <errno.h>

#define PING_PKG_S      64
#define PORT_NO         0
#define RECV_TIMEOUT    1
#define PING_SLEEP_RATE 1000000

struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKG_S - sizeof(struct icmphdr)];
};

static const char *ip_address_str = NULL;

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int send_ping(int ping_sockfd, struct sockaddr_in *ping_addr)
{
    int ttl_val = 64;
    unsigned int i = 0;
    socklen_t addr_len = 0;
    char rbuffer[128];
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    int status = 0;

    if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    {
        puts("Error setting socket options");
        return status;
    }

    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out));

    bzero(&pckt, sizeof(pckt));
    pckt.hdr.type = ICMP_ECHO;
    pckt.hdr.un.echo.id = getpid();

    for (i = 0; i < sizeof(pckt.msg) - 1; i++)
        pckt.msg[i] = i + '0';

    pckt.msg[i] = 0;
    pckt.hdr.un.echo.sequence = 0;
    pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

    usleep(PING_SLEEP_RATE);

    if (sendto(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr *)ping_addr, sizeof(*ping_addr)) <= 0)
    {
        printf("Ping packet sending failed: error %d\n", errno);
        return status;
    }

    addr_len = sizeof(r_addr);
    if (recvfrom(ping_sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr *)&r_addr, &addr_len) <= 0)
    {
        puts("Packet receive failed!\n");
        return status;
    }

    struct iphdr *ip_hdr = (struct iphdr *)rbuffer;
    int ip_header_len = ip_hdr->ihl * 4;

    struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + ip_header_len);
    if (!(recv_hdr->type == 0 && recv_hdr->code == 0))
    {
        printf("Error... Packet received with ICMP type %d code %d\n", recv_hdr->type, recv_hdr->code);
    }
    else
    {
        status = 1;
    }
    return status;
}

unsigned int get_ping_status(void)
{
    int sockfd;
    struct sockaddr_in addr_con;
    int ping_status = 0;

    addr_con.sin_family = AF_INET;
    int status = inet_pton(AF_INET, ip_address_str, &addr_con.sin_addr);
    if (status == 0)
    {
        puts("ip string is not valid!");
        return ping_status;
    }
    else if (status < 0)
    {
        printf("ip converting error: %d", errno);
        return ping_status;
    }

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        printf("Socket file descriptor not received: errno: %d\n", errno);
        return ping_status;
    }

    ping_status = send_ping(sockfd, &addr_con);

    close(sockfd);

    return ping_status;
}

void ping_set_ip_address(const char *ip_address)
{
    ip_address_str = ip_address;
}
