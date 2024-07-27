## How to debug OAF

```
gdbserver :9000 /usr/bin/oafd


gdbserver --attach :9000 16561
```

# Config

```bash
# cat /etc/config/oafd
config global 'global'
	option work_mode '0'
	option enable '1'
	optim lan_if 'down1v0'

config appfilter 'appfilter'
	option websiteapps '8001 8010'
	option downloadapps '7001 7050'
	option videoapps '3001 3003 3052'

config feature 'feature'
	option update '0'
	option format 'v2.0'

config time 'time'
	option time_mode '0'
	option days '0 1 2 3 4 5 6'
	option start_time '00:00'
	option end_time '23:59'

config user 'user'
```


## How to test

```bash
# List all ubus commands
ubus -v list oafd
'oafd' @39235dd9
	"visit_list":{"(unknown)"}
	"dev_visit_time":{"(unknown)"}
	"app_class_visit_time":{"(unknown)"}
	"dev_list":{"(unknown)"}

# Call ubus command
ubus call oafd visit_list
{
	"dev_list": [
		{
			"hostname": "unknown",
			"mac": "d4:f3:37:24:74:99",
			"ip": "192.168.1.39",
			"visit_info": [

			]
		},
		{
			"hostname": "unknown",
			"mac": "00:0e:c6:c8:64:4e",
			"ip": "192.168.1.96",
			"visit_info": [

			]
		}
	]
}

# Call dev list
ubus call oafd dev_list


ubus call oafd dev_visit_time '{ "mac": "00:0e:c6:c8:64:4e" }'

ubus call oafd app_class_visit_time '{ "mac": "00:0e:c6:c8:64:4e" }'

```


## Configuration


```bash
brctl show
bridge name	    bridge id		    STP enabled	        interfaces
down		    7fff.c44bd1a02182	no		            eth1
                                                        eth2
                                                        eth3
                                                        eth4
up		        7fff.c44bd1a02181	no		            eth0
                                                        wlan0
                                                        wlan1

# ip route
route
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
default         192.168.11.1    0.0.0.0         UG    5      0        0 up0v0
192.168.1.0     *               255.255.255.0   U     10     0        0 down1v0
192.168.11.0    *               255.255.255.0   U     5      0        0 up0v0
```

OpenWifi network config:
```bash
vi /etc/config/network

config globals 'globals'
        option ula_prefix 'fdd8:3ddf:76f5::/48'

config switch
        option name 'switch0'
        option reset '0'
        option enable_vlan '0'

config interface 'loopback'
        option ifname 'lo'
        option proto 'static'
        option ipaddr '127.0.0.1'
        option netmask '255.0.0.0'

config device
        option name 'up'
        option type 'bridge'
        option stp '0'
        option igmp_snooping '1'

config device
        option name 'down'
        option type 'bridge'
        option stp '0'
        option igmp_snooping '1'

config interface 'up_none'
        option ifname 'up'
        option proto 'none'

config device
        option name 'lan'
        option macaddr 'c4:4b:d1:a0:21:82'

config device
        option name 'wan'
        option macaddr 'c4:4b:d1:a0:21:81'

config bridge-vlan
        option device 'up'
        option vlan '4090'
        list ports 'eth0'

config device
        option type '8021q'
        option name 'up0v0'
        option ifname 'up'
        option vid '4090'

config interface 'up0v0'
        option ucentral_name 'WAN'
        option ucentral_path '/interfaces/0'
        option ifname 'up0v0'
        option metric '5'
        option proto 'dhcp'
        option peerdns '1'

config bridge-vlan
        option device 'down'
        option vlan '4089'
        list ports 'eth1'
        list ports 'eth2'
        list ports 'eth3'
        list ports 'eth4'

config device
        option type '8021q'
        option name 'down1v0'
        option ifname 'down'
        option vid '4089'

config interface 'down1v0'
        option ucentral_name 'LAN'
        option ucentral_path '/interfaces/1'
        option ifname 'down1v0'
        option metric '10'
        option proto 'static'
        option ipaddr '192.168.1.1/24'
```

Firewall:
```
cat /etc/config/firewall

cat /etc/config/firewall 

config defaults
	option syn_flood '1'
	option input 'ACCEPT'
	option output 'ACCEPT'
	option forward 'REJECT'

config include
	option path '/etc/firewall.user'

config rule
	option name 'Allow-ssh-up0v0'
	option src 'up0v0'
	option dest_port '22'
	option proto 'tcp'
	option target 'ACCEPT'

config rule
	option name 'Allow-ssh-down1v0'
	option src 'down1v0'
	option dest_port '22'
	option proto 'tcp'
	option target 'ACCEPT'

config zone
	option name 'up0v0'
	option input 'REJECT'
	option output 'ACCEPT'
	option forward 'REJECT'
	option masq '1'
	option mtu_fix '1'
	list network 'up0v0'

config rule
	option name 'Allow-Ping'
	option src 'up0v0'
	option proto 'icmp'
	option icmp_type 'echo-request'
	option family 'ipv4'
	option target 'ACCEPT'

config rule
	option name 'Allow-IGMP'
	option src 'up0v0'
	option proto 'igmp'
	option family 'ipv4'
	option target 'ACCEPT'

config rule
	option name 'Support-UDP-Traceroute'
	option src 'up0v0'
	option dest_port '33434:33689'
	option proto 'udp'
	option family 'ipv4'
	option target 'REJECT'
	option enabled 'false'

config rule
	option name 'Allow-DHCP-Renew'
	option src 'up0v0'
	option proto 'udp'
	option dest_port '68'
	option target 'ACCEPT'
	option family 'ipv4'

config rule
	option name 'Allow-DHCPv6'
	option src 'up0v0'
	option proto 'udp'
	option src_ip 'fc00::/6'
	option dest_ip 'fc00::/6'
	option dest_port '546'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-MLD'
	option src 'up0v0'
	option proto 'icmp'
	option src_ip 'fe80::/10'
	option icmp_type '143/0'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-ICMPv6-Input'
	option src 'up0v0'
	option proto 'icmp'
	list icmp_type 'echo-request'
	list icmp_type 'echo-reply'
	list icmp_type 'destination-unreachable'
	list icmp_type 'packet-too-big'
	list icmp_type 'time-exceeded'
	list icmp_type 'bad-header'
	list icmp_type 'unknown-header-type'
	list icmp_type 'router-solicitation'
	list icmp_type 'neighbour-solicitation'
	list icmp_type 'router-advertisement'
	list icmp_type 'neighbour-advertisement'
	option limit '1000/sec'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-ICMPv6-Forward'
	option src 'up0v0'
	option dest '*'
	option proto 'icmp'
	list icmp_type 'echo-request'
	list icmp_type 'echo-reply'
	list icmp_type 'destination-unreachable'
	list icmp_type 'packet-too-big'
	list icmp_type 'time-exceeded'
	list icmp_type 'bad-header'
	list icmp_type 'unknown-header-type'
	option limit '1000/sec'
	option family 'ipv6'
	option target 'ACCEPT'

config zone
	option name 'down1v0'
	option input 'ACCEPT'
	option output 'ACCEPT'
	option forward 'ACCEPT'
	list network 'down1v0'

config forwarding
	option src 'down1v0'
	option dest 'up0v0'

config rule
	option name 'Allow-Ping'
	option src 'down1v0'
	option proto 'icmp'
	option icmp_type 'echo-request'
	option family 'ipv4'
	option target 'ACCEPT'

config rule
	option name 'Allow-IGMP'
	option src 'down1v0'
	option proto 'igmp'
	option family 'ipv4'
	option target 'ACCEPT'

config rule
	option name 'Support-UDP-Traceroute'
	option src 'down1v0'
	option dest_port '33434:33689'
	option proto 'udp'
	option family 'ipv4'
	option target 'REJECT'
	option enabled 'false'

config rule
	option name 'Allow-DHCP-Renew'
	option src 'down1v0'
	option proto 'udp'
	option dest_port '68'
	option target 'ACCEPT'
	option family 'ipv4'

config rule
	option name 'Allow-DHCPv6'
	option src 'down1v0'
	option proto 'udp'
	option src_ip 'fc00::/6'
	option dest_ip 'fc00::/6'
	option dest_port '546'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-MLD'
	option src 'down1v0'
	option proto 'icmp'
	option src_ip 'fe80::/10'
	option icmp_type '143/0'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-ICMPv6-Input'
	option src 'down1v0'
	option proto 'icmp'
	list icmp_type 'echo-request'
	list icmp_type 'echo-reply'
	list icmp_type 'destination-unreachable'
	list icmp_type 'packet-too-big'
	list icmp_type 'time-exceeded'
	list icmp_type 'bad-header'
	list icmp_type 'unknown-header-type'
	list icmp_type 'router-solicitation'
	list icmp_type 'neighbour-solicitation'
	list icmp_type 'router-advertisement'
	list icmp_type 'neighbour-advertisement'
	option limit '1000/sec'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-ICMPv6-Forward'
	option src 'down1v0'
	option dest '*'
	option proto 'icmp'
	list icmp_type 'echo-request'
	list icmp_type 'echo-reply'
	list icmp_type 'destination-unreachable'
	list icmp_type 'packet-too-big'
	list icmp_type 'time-exceeded'
	list icmp_type 'bad-header'
	list icmp_type 'unknown-header-type'
	option limit '1000/sec'
	option family 'ipv6'
	option target 'ACCEPT'

config rule
	option name 'Allow-DNS-down1v0'
	option src 'down1v0'
	option dest_port '53'
	option family 'ipv4'
	list proto 'tcp'
	list proto 'udp'
	option target 'ACCEPT'

config rule
	option name 'Allow-DHCP-down1v0'
	option src 'down1v0'
	option dest_port '67'
	option family 'ipv4'
	option proto 'udp'
	option target 'ACCEPT'

config rule
	option name 'Allow-DHCPv6-down1v0'
	option src 'down1v0'
	option dest_port '547'
	option family 'ipv6'
	option proto 'udp'
	option target 'ACCEPT'

```


## OpenWrt config


```bash
brctl show
bridge name	bridge id		    STP enabled	    interfaces
br-lan		7fff.9483c42f89af	no		        wlan0-1
							                    eth1

# Route
route
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
default         192.168.86.1    0.0.0.0         UG    0      0        0 wlan0
192.168.86.0    *               255.255.255.0   U     0      0        0 wlan0
192.168.100.0   *               255.255.255.0   U     0      0        0 br-lan
```


```bash
cat /etc/config/network 

config interface 'loopback'
	option device 'lo'
	option proto 'static'
	option ipaddr '127.0.0.1'
	option netmask '255.0.0.0'

config globals 'globals'
	option ula_prefix 'fdf5:d9fc:25d5::/48'

config device
	option name 'br-lan'
	option type 'bridge'
	list ports 'eth1'

config device
	option name 'eth1'
	option macaddr '94:83:c4:2f:89:af'

config interface 'lan'
	option device 'br-lan'
	option proto 'static'
	option netmask '255.255.255.0'
	option ip6assign '60'
	option ipaddr '192.168.100.1'

config device
	option name 'eth0'
	option macaddr '94:83:c4:2f:89:ae'

config interface 'wan'
	option device 'eth0'
	option proto 'dhcp'

config interface 'wan6'
	option device 'eth0'
	option proto 'dhcpv6'

config interface 'wwan'
	option proto 'dhcp'
```