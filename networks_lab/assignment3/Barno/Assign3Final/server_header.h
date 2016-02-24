 #include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
//#include <mysql/mysql.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <string>
#include <cctype>
#include <ctype.h>
#include <netinet/ip.h>    //Provides declarations for ip header
#include <ifaddrs.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <signal.h>
#include <list>
#include <vector>
#include <time.h>

using namespace std;

// Packet length
#define PCKT_LEN 8192
#define SYN  1
#define ACK  2
#define FIN  3
#define DATA  4
#define SYNACK 5
#define FINACK 6
#define MAX 10000

int SEQNO = 200;
int SEQRCV = 100;

int curr_seq;



struct rltpheader
{
  unsigned short src_port;
  unsigned short dst_port;
  unsigned int seq_no;
  unsigned int ack_no;
  unsigned int chksum;
  unsigned short type;
};

unsigned int calc_chksum()
{
    return 1;
}

// Simple checksum function, may use others such as Cyclic Redundancy Check, CRC
unsigned short csum(char *buf, int len)
{
  unsigned long sum;
  for(sum=0; len>0; len--)
  {
      sum += *buf++;
  }
  sum = (sum >> 16) + (sum &0xffff);
  sum += (sum >> 16);
  return (unsigned short)(~sum);
}

struct datagram
{
 struct iphdr iph;
 struct rltpheader rltph;
 char data[4096];
};

int eval_checkSum(struct datagram msg){
  int checkSum = 0;
  checkSum += msg.rltph.src_port;
  checkSum %= MAX;
  checkSum += msg.rltph.dst_port;
  checkSum %= MAX;
  checkSum += msg.rltph.seq_no;
  checkSum %= MAX;
  checkSum += msg.rltph.ack_no;
  checkSum %= MAX;

  int i;
  while(i < 4096){
    if(!msg.data[i]) break;
    checkSum += (msg.data[i] * i) % MAX;
    checkSum %= MAX;
  }

  return checkSum;
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


void initip_data(struct datagram *glbl_data,char net[10])
{
    char sourceip[25];
    int rst = get_ip(net,sourceip);
    if(rst == -1)
    {
        cout<<"                    //*Wrong Interface!!*//"<<endl;
        exit(0);
    }
	//cout<<"IP: "<<sourceip<<endl;
    glbl_data->iph.ihl = 5;
    glbl_data->iph.version = 4;
    glbl_data->iph.tos = 0;
    glbl_data->iph.tot_len = sizeof (struct datagram);
    glbl_data->iph.id = htonl (54321); //Id of this packet
    glbl_data->iph.frag_off = 0;
    glbl_data->iph.ttl = 255;
    glbl_data->iph.protocol = IPPROTO_RAW;
    glbl_data->iph.check = 0;      //Set to 0 before calculating checksum
    ////////Insert source ip here//////////
    // glbl_data->iph.saddr = inet_addr ( argv[1] );    //Spoof the source ip address

    glbl_data->iph.saddr = inet_addr ( sourceip );
    // glbl_data->iph.daddr = dest_addr.sin_addr.s_addr;
    glbl_data->iph.check = 0;

    glbl_data->rltph.src_port=htons(10000);
}


int accept(int sd,struct sockaddr_in * client_addr_ptr, char net[10]){

  int flag = 1;
  int time_out = 0;
  srand (time(NULL));
  SEQNO = rand() % 8000 + 1000;
  struct datagram synmsg;
  struct datagram synAckMsg;
  socklen_t addrlen;
  if (recvfrom (sd, &synmsg, 500, 0, (struct sockaddr *) client_addr_ptr, &addrlen) < 0)  {
        perror ("pehla recvfrom() failed ");
        return -1;
  }

  while(1){
  if(flag){SEQRCV = synmsg.rltph.seq_no;}
  //cout<<SEQRCV<<endl;
  initip_data(&synAckMsg,net);
  cout<<"                    //*SYN RECIEVED!!*//"<<endl;
  synAckMsg.iph.daddr = synmsg.iph.saddr;
  synAckMsg.rltph.dst_port=htons(10000);
  synAckMsg.rltph.seq_no = SEQNO;
  synAckMsg.rltph.ack_no = SEQRCV;
  synAckMsg.rltph.chksum = calc_chksum();
  synAckMsg.rltph.type = SYNACK;

  char str[INET_ADDRSTRLEN];
  inet_ntop( AF_INET, &(synAckMsg.iph.daddr), str, INET_ADDRSTRLEN );
  //cout<<str<<endl;

  client_addr_ptr->sin_family = AF_INET;
  client_addr_ptr->sin_port = synAckMsg.rltph.dst_port;
  client_addr_ptr->sin_addr.s_addr = synAckMsg.iph.daddr;
  memset (synAckMsg.data, 0, 4096);

  if (sendto (sd, &synAckMsg, 500, 0, (struct sockaddr *) client_addr_ptr, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        return -1;
  }
  //cout<<"sent"<<endl;

  time_out = 1;
fd_set set;
        FD_ZERO(&set);
        FD_SET(sd,&set);
        struct timeval tv = {2, 0};   // sleep for 2 sec
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
	   if(FD_ISSET(sd,&set)){
            time_out = 0;
	    addrlen = sizeof(sockaddr_in);
           if (recvfrom (sd, &synmsg, 500, 0, (struct sockaddr *) client_addr_ptr, &addrlen) < 0)  {
                perror ("recvfrom() failed ");
                return -1;
           }
          }
        }

    if(time_out) {flag = 0; continue;}



  if(synmsg.rltph.type == SYN) {flag = 1;continue;}

  if( synmsg.rltph.ack_no == SEQNO) {
   client_addr_ptr->sin_family = AF_INET;
   client_addr_ptr->sin_port = synmsg.rltph.dst_port;
   client_addr_ptr->sin_addr.s_addr = synAckMsg.iph.daddr;
   cout<<"                    //*connection established successfully!!*//"<<endl;
   sleep(3);
   break;
  }
  else{flag = 0;}

 }

  return 0;
}

int fin_ack(int sd,struct sockaddr_in  client_addr, char net[10]){

  int time_out = 0;
  struct datagram synmsg;
  struct datagram synAckMsg;
  socklen_t addrlen;
  sockaddr_in recv_addr;

  while(1){
  //cout<<SEQRCV<<endl;
  initip_data(&synAckMsg,net);
  cout<<"                    //*FIN RECEIVED*//"<<endl;
  synAckMsg.iph.daddr = client_addr.sin_addr.s_addr;
  synAckMsg.rltph.dst_port=htons(10000);
  synAckMsg.rltph.seq_no = SEQNO;
  synAckMsg.rltph.ack_no = SEQRCV;
  synAckMsg.rltph.chksum = calc_chksum();
  synAckMsg.rltph.type = FINACK;

  char str[INET_ADDRSTRLEN];
  inet_ntop( AF_INET, &(synAckMsg.iph.daddr), str, INET_ADDRSTRLEN );
  //cout<<str<<endl;
  memset (synAckMsg.data, 0, 4096);

  if (sendto (sd, &synAckMsg, 500, 0, (struct sockaddr *) &client_addr, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        return -1;
  }
  //cout<<"sent"<<endl;

  time_out = 1;
  fd_set set;
        FD_ZERO(&set);
        FD_SET(sd,&set);
        struct timeval tv = {2, 0};   // sleep for 2 sec
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
	   if(FD_ISSET(sd,&set)){
            time_out = 0;
	    addrlen = sizeof(sockaddr_in);
           if (recvfrom (sd, &synmsg, 500, 0, (struct sockaddr *) &recv_addr, &addrlen) < 0)  {
                perror ("recvfrom() failed ");
                return -1;
           }
          }
        }

    if(time_out) {continue;}

  if(synmsg.rltph.type == FIN) {continue;}

  if( synmsg.rltph.ack_no == SEQNO) {
   close(sd);
   sleep(3);
   break;
  }


 }

  return 0;
}

int receive(int sd,char data[4096], char net[10]){

  struct datagram synmsg;
  struct datagram synAckMsg;
  struct sockaddr_in dest_addr;
  socklen_t addrlen;
  int flag_not_first = 0;

 while(1){

 {
        int flag = 0;
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sd,&set);
        struct timeval tv = {2, 0};   // sleep for 2 sec
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
	   if(FD_ISSET(sd,&set)){
            flag = 1;
            flag_not_first = 1;
	    addrlen = sizeof(dest_addr);
            if (recvfrom (sd, &synmsg, 500, 0, (struct sockaddr *) &dest_addr, &addrlen) < 0)  {
                perror ("pa recvfrom() failed ");
                return -1;
	    }
	     cout<<"                    //*****//"<<endl;
          }
        }
	if(flag==0)
		if(flag_not_first) break;
		else continue;
 }

  if(synmsg.rltph.type == FIN) {
   struct sockaddr_in  client_addr;
   client_addr.sin_family = AF_INET;
   client_addr.sin_port = synmsg.rltph.src_port;
   client_addr.sin_addr.s_addr = synmsg.iph.saddr;
   SEQRCV = synmsg.rltph.seq_no;
   fin_ack(sd,client_addr,net);
   return -2;
  }

  //cout<<"**"<<synmsg.data<<endl;
  if(synmsg.rltph.chksum != eval_checkSum(synmsg)) continue;


  strcpy(data,synmsg.data);
  //cout<<SEQRCV<<endl;
  synAckMsg = synmsg;
  initip_data(&synAckMsg,net);
  cout<<"                    //*DATA MESSAGE RECEIVED!!*//"<<endl;
  synAckMsg.iph.daddr = synmsg.iph.saddr;
  synAckMsg.rltph.dst_port=htons(10000);
  synAckMsg.rltph.seq_no = SEQNO;
  synAckMsg.rltph.ack_no = SEQRCV + strlen(synmsg.data);
  synAckMsg.rltph.chksum = eval_checkSum(synAckMsg);
  synAckMsg.rltph.type = DATA;

  SEQRCV = synmsg.rltph.seq_no;

  struct sockaddr_in client_addr;
  char str[INET_ADDRSTRLEN];
 // inet_nstop( AF_INET, &(synAckMsg.iph.daddr), str, INET_ADDRSTRLEN );
 // cout<<str<<endl;
	//cout<<"gchgch"<<endl;
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = synAckMsg.rltph.dst_port;
  client_addr.sin_addr.s_addr = synAckMsg.iph.daddr;
  memset (synAckMsg.data, 0, 4096);
  //cout<<"gh"<<endl;
  // strcpy(synAckMsg.data,"yoyo");
//cout<<synAckMsg.rltph.ack_no<<"ack"<<endl;

  if (sendto (sd, &synAckMsg, 500, 0, (struct sockaddr *) &client_addr, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        return -1;
  }

  //cout<<"sent"<<endl;
 }

  return 0;
}

int close_conn_server(int sd,struct sockaddr_in * client_addr_ptr, char net[10]){

  int flag = 1;
  int time_out = 0;
  struct datagram synmsg;
  struct datagram synAckMsg;
  socklen_t addrlen;
  if (recvfrom (sd, &synmsg, 500, 0, (struct sockaddr *) client_addr_ptr, &addrlen) < 0)  {
        perror ("pehla recvfrom() failed ");
        return -1;
  }

  while(1){
  if(flag){SEQRCV = synmsg.rltph.seq_no;}
  //cout<<SEQRCV<<endl;
  initip_data(&synAckMsg,net);
  cout<<"                    //*FIN*//"<<endl;
  synAckMsg.iph.daddr = synmsg.iph.saddr;
  synAckMsg.rltph.dst_port=htons(10000);
  synAckMsg.rltph.seq_no = SEQNO;
  synAckMsg.rltph.ack_no = SEQRCV;
  synAckMsg.rltph.chksum = calc_chksum();
  synAckMsg.rltph.type = FINACK;

  char str[INET_ADDRSTRLEN];
  inet_ntop( AF_INET, &(synAckMsg.iph.daddr), str, INET_ADDRSTRLEN );
  //cout<<str<<endl;

  client_addr_ptr->sin_family = AF_INET;
  client_addr_ptr->sin_port = synAckMsg.rltph.dst_port;
  client_addr_ptr->sin_addr.s_addr = synAckMsg.iph.daddr;
  memset (synAckMsg.data, 0, 4096);

  if (sendto (sd, &synAckMsg, 500, 0, (struct sockaddr *) client_addr_ptr, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        return -1;
  }
  //cout<<"sent"<<endl;

  time_out = 1;
fd_set set;
        FD_ZERO(&set);
        FD_SET(sd,&set);
        struct timeval tv = {2, 0};   // sleep for 2 sec
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
	   if(FD_ISSET(sd,&set)){
            time_out = 0;
	    addrlen = sizeof(sockaddr_in);
           if (recvfrom (sd, &synmsg, 500, 0, (struct sockaddr *) client_addr_ptr, &addrlen) < 0)  {
                perror ("recvfrom() failed ");
                return -1;
           }
          }
        }

    if(time_out) {flag = 0; continue;}



  if(synmsg.rltph.type == FIN) {flag = 1;continue;}

  if( synmsg.rltph.ack_no == SEQNO) {
   client_addr_ptr->sin_family = AF_INET;
   client_addr_ptr->sin_port = synmsg.rltph.dst_port;
   client_addr_ptr->sin_addr.s_addr = synAckMsg.iph.daddr;
   cout<<"                    //*SESSION CLOSED SUCCESSFULLY!!*//"<<endl;
   close(sd);
   sleep(3);
   break;
  }
  else{flag = 0;}

 }

  return 0;
}

int send(int sockid,char message[4096],struct sockaddr_in *dest_addr,char net[10])
{
    const int on = 1;
    SEQNO = SEQNO + strlen(message);
    while(1)
    {
        cout<<"                    /* SENDING DATA */"<<endl;
        int flag=0;
        struct datagram synmsg;
        initip_data(&synmsg,net);
        cout<<"                    /* INIT IP FOR DATA DONE */"<<endl;
        synmsg.iph.daddr = dest_addr->sin_addr.s_addr;
        synmsg.rltph.dst_port=dest_addr->sin_port;
        synmsg.rltph.seq_no = SEQNO;
        synmsg.rltph.ack_no = 0;

        synmsg.rltph.type = DATA;
        //zero out the packet buffer
        memset (synmsg.data, 0, 4096);
        strcpy(synmsg.data,message);

        socklen_t sender_len;
        synmsg.rltph.chksum = eval_checkSum(synmsg);

        if (sendto (sockid, &synmsg, 500, 0, (struct sockaddr *) dest_addr, sizeof (struct sockaddr)) < 0)
        {
            perror ("sendto() failed ");
            return -1;
        }

        // cout<<"sent data"<<endl;

        struct datagram synAckMsg1;

        fd_set set;
        FD_ZERO(&set);
        FD_SET(sockid,&set);
        struct timeval tv = {3, 0};
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
            if(FD_ISSET(sockid,&set))
            {
                flag = 1;
                if (recvfrom (sockid, &synAckMsg1, 500, 0, (struct sockaddr *) dest_addr, &sender_len) < 0)  {
                    perror ("receive() failed ");
                }
            }
        }
        if(flag && synAckMsg1.rltph.ack_no == synmsg.rltph.seq_no)
        {
            cout<<"                    /* DATA SENT SUCCESSFULLY */"<<endl;
            sleep(3);
            break;
        }

    }
}
