#include "server.h"
#include "ping.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void program_exit(void)
{
    puts("Closing server");
    ping_close();
}

void signal_handler(int sig)
{
    exit(sig);
}

void print_help(const char *program_name)
{
    printf(
        "Usage: %s <options>\n"
        "Options:\n"
        "\t-h - print this message\n"
        "\t-i IPADDRESS - set ip address of a deer to be pinged\n"
        "\t-p - set the port where the deer will operate (default: %d)\n", program_name, DEFAULT_SERVER_PORT);
    exit(1);
}

void parse_args(int argc, char **argv)
{
    if (argc < 2)
        print_help(argv[0]);

    int ipflag = 0;
    int opt;
    while ((opt = getopt(argc, argv, "hi:p:")) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(argv[0]);
                break;
            case 'i':
                ipflag = 1;
                ping_set_ip_address(optarg);
                break;
            case 'p':
                server_set_port(atoi(optarg));
                break;
            case ':':
                printf("Argument needed: %c\n", optopt);
                exit(1);
                break;
            case '?':
                printf("Unknown argument: %c\n", optopt);
                exit(1);
                break;
        }
    }

    if (!ipflag)
    {
        puts("The ip '-i' option is mandarory");
        exit(1);
    }
}

int main(int argc, char **argv)
{
    parse_args(argc, argv);

    int status;
    if ((status = ping_init()) != 0)
        return status;

    signal(SIGINT, signal_handler);
    atexit(program_exit);

    if (ping_start() != 0) return 1;
    server_start();
    return 0;
}
