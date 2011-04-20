#ifndef __include_network_h
#define __include_network_h

#include <lwip/netif.h>
#include <lwip/dhcp.h>

void network_init();
void network_poll();
void network_print_config();

extern struct netif netif;

#endif