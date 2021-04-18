/* 
	Abagiu Ioan-Razvan
	321CD
*/
#include "queue.h"
#include "skel.h"

// helper function to print the rtable
void print_rtable(struct route_table_entry *rtable, int rtable_size) {

	for(int i = 0; i < rtable_size; i++) {

		struct in_addr prefix;
		prefix.s_addr = rtable[i].prefix;

		struct in_addr next;
		next.s_addr = rtable[i].next_hop;

		struct in_addr mask;
		mask.s_addr = rtable[i].mask;


		printf("prefix: %s | ", inet_ntoa(prefix));
		printf("next_hop: %s | ", inet_ntoa(next));
		printf("mask: %s | ", inet_ntoa(mask));
		printf("interface: %d\n", rtable[i].interface);
	}
}

int parse_rtable(struct route_table_entry *rtable, char *filenamein) {

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	int readLine = 0;
	char *helper;

	fp = fopen(filenamein, "r");
	if(fp == NULL) {
		perror("error on opening filenamein");
		return -1;
	}

	int i = 0;
	while((readLine = getline(&line, &len, fp)) != -1) {

		struct in_addr aux_addr;
		
		helper = strtok(line, " ");
		inet_aton(helper, &aux_addr);
		rtable[i].prefix = aux_addr.s_addr;

		helper = strtok(NULL, " ");
		inet_aton(helper, &aux_addr);
		rtable[i].next_hop = aux_addr.s_addr;

		helper = strtok(NULL, " ");
		inet_aton(helper, &aux_addr);
		rtable[i].mask = aux_addr.s_addr;

		helper = strtok(NULL, " ");
		rtable[i].interface = atoi(helper);

		i++;
	}

	fclose(fp);
	free(line);
	return i;
}

// used on qsort
int comparator(const void *p1, const void *p2) {

	const struct route_table_entry *e1 = p1;
	const struct route_table_entry *e2 = p2;

	if (e1->prefix < e2->prefix)
		return -1;
	else if (e1->prefix > e2->prefix)
		return 1;
	else if (e1->mask < e2->mask)
		return -1;
	else { 
		return 1;
	}
}

// binary search implementation for rtable
int binary_search(int l, int r, __u32 dest_ip, struct route_table_entry *rtable) {

	if (l <= r) {
		int mid = (l + r)/2;

		if ((rtable[mid].mask & dest_ip) == rtable[mid].prefix) {

			while((rtable[mid].mask & dest_ip) == rtable[mid].prefix) {
				mid++;
			}
		return mid - 1;
		}
		else if ((rtable[mid].mask & dest_ip) < rtable[mid].prefix) {
			return binary_search(l, mid - 1, dest_ip, rtable);
		}
		else {
			return binary_search(mid + 1, r, dest_ip, rtable);
		}
  	}
   return -1;
}

struct route_table_entry *get_best_route(__u32 dest_ip, struct route_table_entry *rtable, int rtable_size) {

	int pos = binary_search(0, rtable_size - 1, dest_ip, rtable);
	if(pos == -1) {
		return NULL;
	}
	else {
		return &rtable[pos];
	}
}

struct arp_entry* get_arp_entry(__u32 ip, struct arp_entry *arp_table, int arp_table_size) {

    for(int i = 0; i < arp_table_size; i++) {
        if(ip == arp_table[i].ip) {
            return &arp_table[i];
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
	packet m;
	int rc;

	init(argc - 2, argv + 2);

	// parsing rtable
	struct route_table_entry *rtable = malloc(sizeof(struct route_table_entry) * 65000);
	int rtable_size = parse_rtable(rtable, argv[1]);

	
	// sortare ascendenta dupa prefix & masca pentru a imbunatati cautarea 
	qsort(rtable, rtable_size, sizeof(struct route_table_entry), comparator);

	int initial_size = 5;
	struct arp_entry *arp_table = malloc(sizeof(struct  arp_entry) * initial_size);
	int arp_table_size = 0;

	queue q = queue_create();

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		// get eth header from message
		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		
		// store router's mac and ip from the message interface
		uint8_t *my_mac = malloc(6 * sizeof(uint8_t));
		get_interface_mac(m.interface, my_mac);
		char* source = get_interface_ip(m.interface);
		struct in_addr source_ip;
		inet_aton(source, &source_ip);

		// ARP PACKET
		if(eth_hdr->ether_type == htons(ETHERTYPE_ARP)) {
			
			// parse arp header 
			struct arp_header *arp_hdr = parse_arp(m.payload);

			// ARP REQUEST
			if(arp_hdr->op == htons(ARPOP_REQUEST)) {

				// if it's for the router, send the reply
				if(arp_hdr->tpa == source_ip.s_addr) {

					// arp reply
					struct ether_header *eth_hdr_reply = malloc(sizeof(struct ether_header));
					build_ethhdr(eth_hdr_reply, my_mac, arp_hdr->sha, htons(ETHERTYPE_ARP));
					send_arp(arp_hdr->spa, arp_hdr->tpa, eth_hdr_reply, m.interface, htons(ARPOP_REPLY));
				}
				// if it is not for the router, drop it
				else {
					continue;
				}
			}

			// ARP REPLY
			else if(arp_hdr->op == htons(ARPOP_REPLY)) {

				// new queue, because the current one will be iterated through to guide all the packets 
				queue updated_q = queue_create();

				// update the arp table
				struct arp_entry arp;
				memcpy(&arp.ip, &arp_hdr->spa, 4);
				memcpy(arp.mac, arp_hdr->sha, 6);
				arp_table[arp_table_size] = arp;
	 			arp_table_size++;
				if(arp_table_size == initial_size - 1) {
					initial_size *= 2;
					arp_table = realloc(arp_table, initial_size);
				}

				// iterate the queue and deliver all packets that can be delivered
				while(!queue_empty(q)) {
					
					packet *m_aux = (packet*) queue_deq(q);

					struct iphdr *ip_hdr_aux = (struct iphdr *)(m_aux->payload + sizeof(struct ether_header)); 	
					struct ether_header *eth_hdr_aux = (struct ether_header *)m_aux->payload;
					struct route_table_entry *best_route = get_best_route(ip_hdr_aux->daddr, rtable, rtable_size);
					struct arp_entry *mac_aux = get_arp_entry(best_route->next_hop, arp_table, arp_table_size);

					// if the ip is in the arp_table, send the packet
					if(mac_aux != NULL) {		
						memcpy(eth_hdr_aux->ether_dhost, mac_aux->mac, 6);
						memcpy(eth_hdr_aux->ether_shost, my_mac, 6);
						send_packet(best_route->interface, m_aux);
					}	
					// if not, maintain it in the queue at the same position
					else {
						queue_enq(updated_q, m_aux);
					}
				}
			// update queue
			q = updated_q;
			}
		}
		// IP PACKET
		else if(eth_hdr->ether_type == htons(ETHERTYPE_IP)) {

			// parse both ip and icmp headers
			struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));
			struct icmphdr *icmp_hdr = parse_icmp(m.payload);

			// if it's for the router
			if(ip_hdr->daddr == source_ip.s_addr) {
				if(ip_hdr->protocol == 1) {
					if(icmp_hdr->type == ICMP_ECHO) {
						send_icmp(ip_hdr->saddr, source_ip.s_addr, my_mac, eth_hdr->ether_shost, ICMP_ECHOREPLY, 0, m.interface, 0, 0);
					}
				}
				// if it's not an icmp echo then drop
				continue;
			}

			// check checksum, if wrong drop
			if(ip_checksum(ip_hdr, sizeof(struct iphdr)) != 0) {
            	continue;
        	}

			if(ip_hdr->ttl <= 1){
				// icmp time exceeded
				send_icmp_error(ip_hdr->saddr, source_ip.s_addr, my_mac, eth_hdr->ether_shost, 11, 0, m.interface);
				continue;
			}

			// update ttl & checksum
			ip_hdr->ttl--;
			ip_hdr->check = 0;
			ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));

			struct route_table_entry* best_route = get_best_route(ip_hdr->daddr, rtable, rtable_size);
			if(best_route == NULL){
				// icmp host unreachable
				send_icmp_error(ip_hdr->saddr, source_ip.s_addr, my_mac, eth_hdr->ether_shost, 3, 0, m.interface);
				continue;
			}
				
			struct arp_entry *arp = get_arp_entry(best_route->next_hop, arp_table, arp_table_size);
			memcpy(eth_hdr->ether_shost, my_mac, 6);

			// send ARP REQUEST if entry is not found
			if(arp == NULL) {

				// enqueue the packet with the new interface
				packet *aux = malloc(sizeof(packet));
				memcpy(aux, &m, sizeof(packet));
				aux->interface = best_route->interface;
				queue_enq(q, aux);

				// ethernet header
				struct ether_header *eth_hdr_req = malloc(sizeof(struct ether_header));
				uint8_t *mac = malloc(6 * sizeof(uint8_t));
				get_interface_mac(aux->interface, mac);
				// set broadcast
				uint8_t *broadcast = malloc(6 * sizeof(uint8_t));
				memset(broadcast, 0xff, 6);
				build_ethhdr(eth_hdr_req, mac, broadcast,  htons(ETHERTYPE_ARP));

				char* source_req = get_interface_ip(aux->interface);
				struct in_addr source_ip_req;
				inet_aton(source_req, &source_ip_req);

				send_arp(best_route->next_hop, source_ip_req.s_addr, eth_hdr_req, aux->interface, htons(ARPOP_REQUEST));
			}
				
			else {
				// if everything is alright, send packet
				memcpy(eth_hdr->ether_dhost, arp->mac, 6);
				send_packet(best_route->interface, &m);
			}
		}
	}
	return 0;
}
