#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<pthread.h>
#include<poll.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/sem.h>
#include<poll.h>
#include<pthread.h>
#include<sys/select.h>
#include <ifaddrs.h>
#include<sys/un.h>
#define SA (struct sockaddr*)
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>


unsigned short
csum (unsigned short *buf, int nwords)
{
	unsigned long sum;
	for (sum = 0; nwords > 0; nwords--)
	sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

int get_ip(char type[10],char ip[20]){
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    int flag = 1;
    getifaddrs(&ifAddrStruct);

  //printf("*%s\n",type);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if(strcmp(ifa->ifa_name,type)==0){
		strcpy(ip,addressBuffer);
		 //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
		flag = 0;
    break;

	    }
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    if(flag) return -1;
    return 0;
}

int
main (int argc, char *argv[])
{
	if (argc < 3)
	{
		printf ("need destination for tracert\n");
		exit (0);
	}
	int sfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
	char buf[4096] = { 0 };
	struct ip *ip_hdr = (struct ip *) buf;

	int one = 1;
	const int *val = &one;
	if (setsockopt (sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
	printf ("Cannot set HDRINCL!\n");

	struct sockaddr_in addr;
	addr.sin_port = htons (7);
	addr.sin_family = AF_INET;
	inet_pton (AF_INET, argv[2], &(addr.sin_addr));

	char self_ip[25];
    if(get_ip(argv[1], self_ip) == -1) {
    	printf("Wrong Interface!!\n");
	exit(0);
    }



	int hop = 0;


	while (1)
	{


		for(int i = 0; i < 3; i++) {


			ip_hdr->ip_hl = 5;
			ip_hdr->ip_v = 4;
			ip_hdr->ip_tos = 0;
			ip_hdr->ip_len = 20 + 8;
			ip_hdr->ip_id = 10000;
			ip_hdr->ip_off = 0;
			ip_hdr->ip_p = IPPROTO_ICMP;
			inet_pton (AF_INET, self_ip, &(ip_hdr->ip_src));
			inet_pton (AF_INET, argv[2], &(ip_hdr->ip_dst));

			struct icmphdr *icmphd = (struct icmphdr *) (buf + 20);
			icmphd->type = ICMP_ECHO;
			icmphd->code = 0;
			icmphd->checksum = 0;
			icmphd->un.echo.id = 0;


			ip_hdr->ip_ttl = hop;
			ip_hdr->ip_sum = csum ((unsigned short *) buf, 9);

			icmphd->un.echo.sequence = hop + 1;
			icmphd->checksum = csum ((unsigned short *) (buf + 20), 4);

			printf("Sending packet with hop = %d\n", hop);

			sendto (sfd, buf, sizeof(struct ip) + sizeof(struct icmphdr), 0, SA & addr, sizeof addr);

			char buff[4096] = { 0 };
			struct sockaddr_in addr2;
			socklen_t len = sizeof (struct sockaddr_in);
			printf("Waiting on receive\n");
			recvfrom (sfd, buff, sizeof(buff), 0, SA & addr2, &len);
			printf("Received\n");

			struct icmphdr *icmphd2 = (struct icmphdr *) (buff + 20);
			if (icmphd2->type != 0)
			printf ("hop limit:%d Address:%s\n", hop, inet_ntoa (addr2.sin_addr));
			else
			{
				printf ("Reached destination:%s with hop limit:%d\n",
				inet_ntoa (addr2.sin_addr), hop);
				exit (0);
			}
		}

		printf("\n");
		hop++;
	}

	return 0;
}
