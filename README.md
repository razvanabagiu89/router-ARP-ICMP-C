# PCom: Implementing a router alongside ARP & ICMP protocols
# Student: Abagiu Ioan-Razvan
# Group: 321CD

# About
Implement a router's behaviour in C and also implement the ARP & ICMP protocols
in order to deliver packages through a network involving 2 routers and 4 hosts

# How to run
- run the network: sudo python3 topo.py
- run the tests: sudo python3 topo.py tests

# Libraries for the implementations
- check skel.h

# Flow
First of all, extract the ETHERNET header and check if the packet received it's
either an ARP packet or an IP packet.

If it's an ARP packet, parse the ARP header and check if it's an ARP REQUEST or
ARP REPLY. If it is a REQUEST, check if its destination matches the router's
destination and then send an ARP REPLY back. If it's not, drop the packet.
Else, if it's a REPLY, update the arp_table and iterate through the queue to
find all packets that can be delivered further.

If it's an IP packet, parse the IP & ICMP header. Check if the packet's
destination matches the router and also check if the packet is an ICMP ECHO
REQUEST, and if it is, send back an ICMP ECHO REPLY. If it's not for the
router, then drop it.

Check checksum, if wrong drop packet. Then, check TTL, if TTL <= 1 send an ICMP
ERROR saying that time exceeded - ICMP TIME EXCEEDS ERROR. Also, check if the
packet's IP is in the routing table of the current router. If it's not, send an
ICMP HOST UNREACHABLE ERROR.

Decrement TTL, recompute checksum and search in the arp_table an entry for the
packet's next_hop calculated by the get_best_route() function. If next_hop is
missing in the arp_table, send an ARP REQUEST by using a broadcast and enqueue
the packet to be sent later. If everything goes alright and next_hop can be
found in the arp_table, just send the packet.

# Implementation

- print_rtable: used for debug reasons to print the rtable

- parse_rtable: parse the rtable coming from the filenamein arg

- comparator: used in qsort for sorting the rtable in order to get a better
search complexity

- binary_search: also used for the same reason

- get_best_route: searches through the rtable to find the best_route using the
longest prefix match algorithm

- get_arp_entry: mathces the given ip with its arp_entry

# Difficulties

Modified the parameter in the send_arp() function from skel.c because it
interfered with a define from the headers.
modification: from 'op' to 'op1' to break the confusion

# Project structure

```bash
├── checker.py
├── check.sh
├── hosts_output
│   ├── checksum
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward02
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward03
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward10
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward10across
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward10packets
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward12
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward13
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward20
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward21
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward23
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward30
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward31
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward32
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── forward_no_arp
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── host_unreachable
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── icmp_timeout
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── router_arp_reply
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── router_arp_request
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── router_icmp
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   ├── ttl
│   │   ├── 0-host-err.txt
│   │   ├── 0-host-out.txt
│   │   ├── 1-host-err.txt
│   │   ├── 1-host-out.txt
│   │   ├── 2-host-err.txt
│   │   ├── 2-host-out.txt
│   │   ├── 3-host-err.txt
│   │   └── 3-host-out.txt
│   └── wrong_checksum
│       ├── 0-host-err.txt
│       ├── 0-host-out.txt
│       ├── 1-host-err.txt
│       ├── 1-host-out.txt
│       ├── 2-host-err.txt
│       ├── 2-host-out.txt
│       ├── 3-host-err.txt
│       └── 3-host-out.txt
├── include
│   ├── list.h
│   ├── queue.h
│   └── skel.h
├── info.py
├── list.c
├── Makefile
├── __pycache__
│   ├── info.cpython-38.pyc
│   └── tests.cpython-38.pyc
├── queue.c
├── README.md
├── router
├── router0 -> router
├── router_0_err
├── router_0_out
├── router1 -> router
├── router_1_err
├── router_1_out
├── router.c
├── rtable0.txt
├── rtable1.txt
├── rtable.txt
├── skel.c
├── test
├── tests.py
└── topo.py

```
