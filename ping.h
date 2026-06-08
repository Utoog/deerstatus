#ifndef PING_H
#define PING_H

int ping_init(void);
void ping_set_ip_address(const char *ip_address);
unsigned int get_ping_status(void);
int ping_start(void);
void ping_close(void);

#endif
