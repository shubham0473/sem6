#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h> // For IPv4 data struct..
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <string.h> // For memset.

// The packet length
#define PCKT_LEN 8192

// RTLP header's structure
struct rtlpheader {
	unsigned short int rtlp_srcport;
	unsigned short int rtlp_destport;
	unsigned int rtlp_seq;
	unsigned int rtlp_ack;
	unsigned int rtlp_chksum;
	unsigned short int rtlp_flag;
}

typedef struct PACKET {
	struct iphdr ip;
	struct rtlpheader rtlp;
	char data[4096];
} packet;

struct timeval time_out_limit = {1, 0};

#define DATA 0
#define SYN 1
#define ACK 2
#define FIN 3
#define SYNACK 4
#define FINACK 5
#define ACKDATA 6

// 32 Bit checksum
unsigned short csum(unsigned int *buf, int nbytes)
{
	unsigned int ndwords = nbytes / 4;   //
	unsigned long sum;
	for(sum = 0; ndwords > 0; ndwords--)
	sum += *buf++;
	sum = (sum >> 32) + (sum & 0xffffffff);
	sum += (sum >> 32);
	return (unsigned int)(~sum);
}

unsigned int calculate_checksum(packet msg) {
	msg.rtlp.rtlp_chksum = 0;
	return csum(&msg.rtlp, sizeof(rtlpheader)) + csum(msg.data, 4096);
}

int verify_checksum(packet msg) {
	unsigned int received = msg.rtlp.rtlp_chksum;
	unsigned int calculated = calculate_checksum(msg);
	return received == calculated;
}

// 3-way Connection Establishment
void connect(int sd, struct sockaddr_in* client_addr, struct iphdr* ip_base, struct rtlpheader* rtlp_base) {
	packet synmsg;
	socklen_t addrlen;
	if(recvfrom(sd, &synmsg, 1000, 0, (struct sockaddr*) client_addr, &addrlen) < 0) {
		perror("Receive failed\n");
		return -1;
	}
	if(synmsg.rtlp.rtlp_flag == SYN) {
		printf("SYN received\n")
		if(verify_checksum(synmsg)) {
			printf("Checksum verified\n");
			ip_base->daddr = synmsg.ip.saddr;
			rtlp_base->rtlp_destport = synmsg.rtlp.rtlp_srcport;
			rtlp_base->rtlp_ack = synmsg.rtlp.rtlp_seq;
		}
		else {
			printf("Checksum mismatch!\n");
			return -1;
		}
	}
	else return -1;

	packet synackmsg;
	memset(synackmsg.data, 0, 4096);
	synackmsg.ip = *ip_base;
	synackmsg.rtlp = *rtlp_base;
	synackmsg.rtlp.rtlp_flag = SYNACK;
	synackmsg.rtlp.rtlp_chksum = calculate_checksum(synackmsg);

	client_addr->sin_family = AF_INET;
  	client_addr->sin_port = synackmsg.rltp.rtlp_destport;
  	client_addr->sin_addr.s_addr = synackmsg.ip.daddr;

	if(sendto(sd, &synackmsg, 1000, 0, (struct sockaddr*) client_addr, sizeof(struct sockaddr) )< 0) {
		perror ("SYNACK send failed\n");
		return -1;
	}
	else {
		printf("SYNACK sent\n");
	}

	int timeout = 1;
	fd_set fdset;
	FD_ZERO(&fdset);
	packet ackmsg;
	FD_SET(sd, &fdset);
	struct timeval timer = time_out_limit; // wait for 1 second
	if(select(sd, &fdset, NULL, NULL, &timer)) {
		if(FD_ISSET(sd, &fdset)) {
			timout = 0;
			addrlen = sizeof(sockaddr_in);
			if(recvfrom(sd, &ackmsg, 1000, 0, (struct sockaddr*) client_addr, &addrlen) < 0) {
				perror("ACK receive failed");
				return -1;
			}
		}
	}

	if(timeout) {
		printf("Connection timed out");
		return -1;
	}

	if(ackmsg.rtlp.rtlp_flag != ACK && ackmsg.rtlp.rtlp_flag != ACKDATA) {
		printf("Packet not of type ACK or ACKDATA");
		return -1;
	}
	else {
		printf("ACK received\n");
		if(verify_checksum(ackmsg)) {
			printf("Checksum verified\n");
			printf("Connection established! \n");
			client_addr->sin_family = AF_INET;
		  	client_addr->sin_port = ackmsg.rltp.rtlp_destport;
		  	client_addr->sin_addr.s_addr = ackmsg.ip.daddr;
			return 0;
		}
		else {
			printf("Checksum mismatch!\n");
			return -1;
		}
	}
}

// Source IP, source port, target IP, target port from the command line arguments
int main(int argc, char *argv[])
{
	srand(time(NULL));
		// Our own headers' structures
	struct iphdr ip_base;
	struct rtlpheader rtlp_base;

	if(argc != 3)
	{
		printf("- Invalid parameters!!!\n");
		printf("- Usage %s <server hostname/IP> <server port>\n", argv[0]);
		exit(-1);
	}

	// Create a raw socket with UDP protocol
	sd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if(sd < 0)
	{
		perror("socket() error");
		// If something wrong just exit
		exit(-1);
	}
	else
	printf("socket() - Using SOCK_RAW socket and RTLP is OK.\n");

	rtlp_base.rtlp_srcport = htons(atoi(argv[2]));
	//rtlp_base.rtlp_destport = htons(atoi(argv[4]));
	rtlp_base.rtlp_seq = rand() % 10000;
	rtlp_base.rtlp_ack = 0;

	// Fabricate the IP header or we can use the
	// standard header structures but assign our own values.
	ip_base.ihl = 5;
	ip_base.version = 4;
	ip_base.tos = 0;
	ip_base.tot_len = sizeof(packet);
	ip_base.id = htonl(54321);
	ip_base.ttl = 255; // hops
	ip_base.frag_off = 0; // hops
	ip_base.protocol = IPPROTO_RAW; //
	ip_base.saddr = inet_addr(argv[1]);	// Source IP address, can use spoofed address here!!!
	ip_base.check = 0; // checksum initially 0

	struct sockaddr_in client_addr;

	printf("Waiting for connection...\n");
	accept(sd, &client_addr, &ip_base, &rtlp_base);

	close(sd);
	return 0;
}
