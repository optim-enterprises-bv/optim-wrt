#ifndef __APPFILTER_H__
#define __APPFILTER_H__

#define MIN_INET_ADDR_LEN 7
#define CMD_GET_LAN_IP   "ifconfig %s | grep 'inet addr' | awk '{print $2}' | awk -F: '{print $2}'"
#define CMD_GET_LAN_MASK "ifconfig %s | grep 'inet addr' | awk '{print $4}' | awk -F: '{print $2}'"

struct client_config {
    const char *lan_if;
    int debug;
};

extern struct client_config client;

#endif