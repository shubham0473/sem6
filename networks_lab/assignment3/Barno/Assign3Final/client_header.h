 #include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <mysql/mysql.h>
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

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <signal.h>
#include <list>
#include <vector>
#include <time.h>

#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

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

int SEQNO = 100;
int SEQRCV = 200;
 
struct rltpheader
{
    unsigned short src_port;
    unsigned short dst_port;
    unsigned int seq_no;
    unsigned int ack_no;
    unsigned int chksum;
    unsigned int type;
};



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

void initip_data(struct datagram *glbl_data,char net[10])
{
    char sourceip[25];
    int rst = get_ip(net,sourceip);
    if(rst == -1)
    {
        cout<<"Wrong Interface!!"<<endl;
        exit(0);
    }
    glbl_data->iph.ihl = 5;
    glbl_data->iph.version = 4;
    glbl_data->iph.tos = 0;
    glbl_data->iph.tot_len = sizeof (struct datagram);
    glbl_data->iph.id = htonl (54321); //Id of this packet
    glbl_data->iph.frag_off = 0;
    glbl_data->iph.ttl = 255;
    glbl_data->iph.protocol = IPPROTO_RAW;
    glbl_data->iph.check = 0;      //Set to 0 before calculating checksum
    
    glbl_data->iph.saddr = inet_addr ( sourceip );
   
    glbl_data->iph.check = 0;

    glbl_data->rltph.src_port=htons(10000);
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

int connect(int sockid,struct sockaddr_in *dest_addr,char net[10])
{
    const int on = 1;
    srand (time(NULL));
    SEQNO = rand() % 8000 + 1000;
    cout<<"SEQNO : "<< SEQNO <<endl;
    while(1)
    {
        struct datagram synmsg;
        initip_data(&synmsg,net);
        cout<<"                    /* INIT IP FOR SYN DONE */"<<endl;
        synmsg.iph.daddr = dest_addr->sin_addr.s_addr;
        synmsg.rltph.dst_port=dest_addr->sin_port;
        synmsg.rltph.seq_no = SEQNO;
        synmsg.rltph.ack_no = 0;
        synmsg.rltph.chksum = calc_chksum();
        synmsg.rltph.type = SYN;
        //zero out the packet buffer
        memset (synmsg.data, 0, 4096);

        socklen_t sender_len;

        if (sendto (sockid, &synmsg, 500, 0, (struct sockaddr *) dest_addr, sizeof (struct sockaddr)) < 0)  
        {
            perror ("sendto() failed ");
            return -1;
        }

        cout<<"                    /* SEQ SENT */"<<endl;

        struct datagram synAckMsg;

        fd_set set;
        FD_ZERO(&set);
        FD_SET(sockid,&set);
        struct timeval tv = {2, 0};   // sleep for ten minutes!
        int timeout = 1;
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
            if(FD_ISSET(sockid,&set))
            {
                timeout = 0;
                if (recvfrom (sockid, &synAckMsg, 500, 0, (struct sockaddr *) dest_addr, &sender_len) < 0)  {
                    perror ("receive() failed ");
                }
            }
        }

        if(timeout)
        {
            continue;
        }

        /////////////////If the correct ACK is received///////////
        if(synAckMsg.rltph.ack_no == SEQNO)
        {
            cout<<"                    /* SEQ+ACK Received */"<<endl;
            SEQRCV = synAckMsg.rltph.seq_no;
            while(1)
            {
                struct datagram ackmsg;
                initip_data(&ackmsg,net);
                cout<<"                    /* INIT IP FOR ACK DONE */"<<endl;;
                ackmsg.iph.daddr = synAckMsg.iph.saddr;
                ackmsg.rltph.dst_port = synAckMsg.rltph.src_port;
                ackmsg.rltph.seq_no = 0;
                ackmsg.rltph.ack_no = SEQRCV ;
                ackmsg.rltph.chksum = calc_chksum();
                ackmsg.rltph.type = ACK;
                //zero out the packet buffer
                memset (ackmsg.data, 0, 4096);

                socklen_t sender_len;

                dest_addr->sin_family = AF_INET;
                dest_addr->sin_port = ackmsg.rltph.dst_port;
                dest_addr->sin_addr.s_addr = ackmsg.iph.daddr;

                char str[100];
                inet_ntop(AF_INET,&(dest_addr),str,INET_ADDRSTRLEN);
                // cout<<"Server IP: "<<str<<endl;

                if (sendto (sockid, &ackmsg, 500, 0, (struct sockaddr *) dest_addr, sizeof (struct sockaddr)) < 0)  
                {
                    perror ("sendto() failed ");
                    return -1;
                }

                cout<<"                    /* ACK SENT */"<<endl;
                
                ////////////Extra Receive///////////
                fd_set set;
                FD_ZERO(&set);
                FD_SET(sockid,&set);
                struct timeval tv = {2, 0}; 
                timeout = 1;
                if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
                {
                    if(FD_ISSET(sockid,&set))
                    {
                        // cout<<"Extra recv"<<endl;
                        timeout = 1;
                        if (recvfrom (sockid, &synAckMsg, 500, 0, (struct sockaddr *) dest_addr, &sender_len) < 0)  {
                            perror ("receive() failed ");
                        }
                    }
                }
                if(timeout)
                {
                    break;
                }
            }
            if(timeout)
            {
                break;
            }
        }
    }

    return 0;
}

int close_conn_client(int sockid,struct sockaddr_in *dest_addr,char net[10])
{
    const int on = 1;
    // srand (time(NULL));
    // SEQNO = rand() % 8000 + 1000;
    // cout<<"FINNO : "<< SEQNO <<endl;
    while(1)
    {
        // cout<<"yoyo"<<endl;
        struct datagram synmsg;
        initip_data(&synmsg,net);
        cout<<"                    /* INIT IP FOR FIN DONE! */"<<endl;
        synmsg.iph.daddr = dest_addr->sin_addr.s_addr;
        synmsg.rltph.dst_port=dest_addr->sin_port;
        synmsg.rltph.seq_no = SEQNO;
        synmsg.rltph.ack_no = 0;
        synmsg.rltph.chksum = calc_chksum();
        synmsg.rltph.type = FIN;
        //zero out the packet buffer
        memset (synmsg.data, 0, 4096);

        socklen_t sender_len;

        if (sendto (sockid, &synmsg, 500, 0, (struct sockaddr *) dest_addr, sizeof (struct sockaddr)) < 0)  
        {
            perror ("sendto() failed ");
            return -1;
        }

        // cout<<"sent"<<endl;

        struct datagram synAckMsg;

        fd_set set;
        FD_ZERO(&set);
        FD_SET(sockid,&set);
        struct timeval tv = {2, 0};   // sleep for ten minutes!
        // int selret = select(FD_SETSIZE, &set, NULL, NULL, &tv);
        // cout<<"SELRET: "<<selret<<endl;
        int timeout = 1;
        if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
        {
            if(FD_ISSET(sockid,&set))
            {
                // cout<<"aa rha hai"<<endl;
                timeout = 0;
                if (recvfrom (sockid, &synAckMsg, 500, 0, (struct sockaddr *) dest_addr, &sender_len) < 0)  {
                    perror ("receive() failed ");
                    // flag = 1;
                }
            }
        }

        if(timeout)
        {
            continue;
        }

        // if (recvfrom (sockid, &synAckMsg, 500, 0, (struct sockaddr *) dest_addr, &sender_len) < 0)  {
        //     perror ("sendto() failed ");
        //     return -1;
        // }

        // cout<<"Ack: "<<synAckMsg.rltph.ack_no<<endl;
        // cout<<"Uska seq: "<<synAckMsg.rltph.seq_no<<endl;


        if(synAckMsg.rltph.ack_no == SEQNO)
        {
            // cout<<SEQNO;
            SEQRCV = synAckMsg.rltph.seq_no;
            while(1)
            {
                struct datagram ackmsg;
                initip_data(&ackmsg,net);
                cout<<"                    /* INIT IP FOR ACK DONE */"<<endl;;
                ackmsg.iph.daddr = synAckMsg.iph.saddr;
                ackmsg.rltph.dst_port = synAckMsg.rltph.src_port;
                ackmsg.rltph.seq_no = 0;
                ackmsg.rltph.ack_no = SEQRCV ;
                ackmsg.rltph.chksum = calc_chksum();
                ackmsg.rltph.type = ACK;
                //zero out the packet buffer
                memset (ackmsg.data, 0, 4096);

                // Set flag so socket expects us to provide IPv4 header.
                // if (setsockopt (sockid, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0) 
                // {
                //   perror ("setsockopt() failed to set IP_HDRINCL ");
                //   return -1;
                // }

                socklen_t sender_len;

                dest_addr->sin_family = AF_INET;
                dest_addr->sin_port = ackmsg.rltph.dst_port;
                dest_addr->sin_addr.s_addr = ackmsg.iph.daddr;

                // cout<<"Sending data : "<<datag.data<<endl;

                // cout<<"yaya"<<endl;
                char str[100];
                inet_ntop(AF_INET,&(dest_addr),str,INET_ADDRSTRLEN);
                // cout<<"Uska IP: "<<str<<endl;

                if (sendto (sockid, &ackmsg, 500, 0, (struct sockaddr *) dest_addr, sizeof (struct sockaddr)) < 0)  
                {
                    perror ("sendto() failed ");
                    return -1;
                }

                // cout<<"Ack: "<<ackmsg.rltph.ack_no<<endl;

                fd_set set;
                FD_ZERO(&set);
                FD_SET(sockid,&set);
                struct timeval tv = {2, 0};   // sleep for ten minutes!
                // int selret = select(FD_SETSIZE, &set, NULL, NULL, &tv);
                // cout<<"SELRET: "<<selret<<endl;
                timeout = 1;
                if (select(FD_SETSIZE, &set, NULL, NULL, &tv))
                {
                    if(FD_ISSET(sockid,&set))
                    {
                        ////////////////Extra Receive/////////////
                        timeout = 1;
                        if (recvfrom (sockid, &synAckMsg, 500, 0, (struct sockaddr *) dest_addr, &sender_len) < 0)  {
                            perror ("receive() failed ");
                            // flag = 1;
                        }
                    }
                }
                if(timeout)
                {
                    break;
                }
            }
            if(timeout)
            {
                break;
            }
            // break;
        }
    }
    cout<<"                    /*CONNECTION CLOSED!*/"<<endl;
    close(sockid);

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
  cout<<"                    /*FIN RECEIVED*/"<<endl;  
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
  cout<<"                    /*DATA MESSAGE RECEIVED!!*/"<<endl; 
  synAckMsg.iph.daddr = synmsg.iph.saddr;
  synAckMsg.rltph.dst_port=htons(10000);
  synAckMsg.rltph.seq_no = SEQNO;
  synAckMsg.rltph.ack_no = SEQRCV + strlen(synmsg.data);
  synAckMsg.rltph.chksum = eval_checkSum(synAckMsg);
  synAckMsg.rltph.type = DATA;

  

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
  //strcpy(synAckMsg.data,"yoyo");
//cout<<synAckMsg.rltph.ack_no<<"ack"<<endl;

  if (sendto (sd, &synAckMsg, 500, 0, (struct sockaddr *) &client_addr, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        return -1;      
  }
        
  //cout<<"sent"<<endl;
 }

  SEQRCV = synmsg.rltph.seq_no;
  return 0;
}

