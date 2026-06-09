#include "ping.h"
#include <stdatomic.h>
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
#include <pthread.h>

#define PING_PKG_S      64
#define PORT_NO         0
#define RECV_TIMEOUT    1
#define PING_SLEEP_RATE 1000000

struct ping_socket_t
{
    int sockfd;
    const char *ip_address;
    struct sockaddr_in addr_con;
    pthread_t thread;
    atomic_int ping_status;
} prv_ping_socket;

struct ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKG_S - sizeof(struct icmphdr)];
};

struct ping_socket_t *get_psocket_instance(void)
{
    return &prv_ping_socket;
}

static unsigned short checksum(void *b, int len)
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

static void *ping_polling(void *ptr)
{
    struct ping_socket_t *pdev = (struct ping_socket_t *)ptr;
    int ttl_val = 64;
    unsigned int i = 0;
    socklen_t addr_len = 0;
    char rbuffer[128];
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    int msgnum = 0;

    if (setsockopt(pdev->sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    {
        puts("Error setting socket options");
        return NULL;
    }

    setsockopt(pdev->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out));

    while (1)
    {
        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();

        for (i = 0; i < sizeof(pckt.msg) - 1; i++)
            pckt.msg[i] = i + '0';

        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msgnum++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

        usleep(PING_SLEEP_RATE);

        if (sendto(pdev->sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr *)&pdev->addr_con, sizeof(pdev->addr_con)) <= 0)
        {
            printf("Ping packet sending failed: error %d\n", errno);
            atomic_store(&pdev->ping_status, 0);
            continue;
        }

        addr_len = sizeof(r_addr);
        if (recvfrom(pdev->sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr *)&r_addr, &addr_len) <= 0)
        {
            puts("Packet receive failed!\n");
            atomic_store(&pdev->ping_status, 0);
            continue;
        }

        struct iphdr *ip_hdr = (struct iphdr *)rbuffer;
        int ip_header_len = ip_hdr->ihl * 4;

        struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + ip_header_len);
        if (!(recv_hdr->type == 0 && recv_hdr->code == 0))
        {
            printf("Error... Packet received with ICMP type %d code %d\n", recv_hdr->type, recv_hdr->code);
            atomic_store(&pdev->ping_status, 0);
        }
        else
        {
            atomic_store(&pdev->ping_status, 1);
        }
    }
    return NULL;
}

int ping_init(void)
{
    struct ping_socket_t *pdev = get_psocket_instance();
    pdev->addr_con.sin_family = AF_INET;
    int status = inet_pton(AF_INET, pdev->ip_address, &pdev->addr_con.sin_addr);
    if (status == 0)
    {
        puts("ip string is not valid!");
        return 1;
    }
    else if (status < 0)
    {
        printf("ip converting error: %d", errno);
        return 1;
    }

    pdev->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (pdev->sockfd < 0)
    {
        printf("Socket file descriptor not received: errno: %d\n", errno);
        return 1;
    }
    return 0;
}

unsigned int get_ping_status(void)
{
    struct ping_socket_t *pdev = get_psocket_instance();
    return atomic_load(&pdev->ping_status);
}

void ping_set_ip_address(const char *ip_address)
{
    struct ping_socket_t *pdev = get_psocket_instance();
    pdev->ip_address = ip_address;
}

void ping_close(void)
{
    struct ping_socket_t *pdev = get_psocket_instance();
    pthread_cancel(pdev->thread);
    pthread_join(pdev->thread, NULL);
    close(pdev->sockfd);
}

int ping_start(void)
{
    struct ping_socket_t *pdev = get_psocket_instance();
    int status = pthread_create(&pdev->thread, NULL, ping_polling, pdev);
    if (status != 0)
    {
        puts("Couldn't create ping polling thread");
        return 1;
    }
    return 0;
}
