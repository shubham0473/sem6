#include "server_header.h"

int main(int argc,char *argv[])
{

    int n;
    char n_string[10];
    char server_ip[25];
    if(get_ip(argv[1],server_ip) == -1) {
    	printf("Wrong Interface!!\n");
	exit(0);
    }
    	
     const int on = 1;
    //Create a raw socket
    int sd = socket (PF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sd == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket");
        exit(1);
    }

    //Datagram to represent the packet
    struct datagram datag,datag1; 
    char source_ip[32] , *data , *pseudogram;
     
    //zero out the packet buffer
    memset (datag.data, 0, 4096);
     
  
    struct sockaddr_in dest_addr;
    // struct pseudo_header psh;
     
    //Data part

    strcpy(datag.data , "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

    socklen_t sender_len;
    accept(sd,&dest_addr,argv[1]);
   

    char msg[4096];
    while(1){
     if(receive(sd,msg,argv[1]) < 0) break;
     cout<<msg<<endl;
     n = atoi(msg + 9);
     //cout<<"N : "<<n<<endl;

     strcpy(msg,"ECHO RES ");
     sprintf(n_string,"%d",n+1);
     strcat(msg,n_string);
     if(send(sd,msg,&dest_addr,argv[1]) < 0) break;
    }
    
    return 0;
} 


