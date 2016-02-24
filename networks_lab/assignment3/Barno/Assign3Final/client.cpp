#include "client_header.h"

int main(int argc,char *argv[])
{
    struct sockaddr_in dest_addr,srv_addr;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(atoi(argv[2]));
    dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
   

    const int on = 1;
    //Create a raw sockets

    int sd = socket (PF_INET, SOCK_RAW, IPPROTO_RAW);
    if(sd == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket");
        exit(1);
    }

    if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0) 
    {
      perror ("setsockopt() failed to set IP_HDRINCL ");
      return -1;
    }

    struct datagram datag,datag1; 
    char source_ip[32] , *data , *pseudogram;
     
    connect(sd,&dest_addr,argv[3]);

    char msg[1000] = "ECHO REQ ";
    int reqno = 1;
    string s = to_string(reqno++);
    strcat(msg,s.c_str());
    cout<<msg<<endl;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(atoi(argv[2]));
    dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
    send(sd,msg,&dest_addr,argv[3]);
    receive(sd,msg,argv[3]);
    cout<<"Received Message : "<<msg<<endl;
    cout<<"*********"<<endl;

    for(int j = 0; j < 9; j++)
    {
        strcpy(msg,"ECHO REQ ");
        s = to_string(reqno++);
        // char msg[1000] = "ECHO REQ ";
        strcat(msg,s.c_str());
        cout<<msg<<endl;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(atoi(argv[2]));
        dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
        send(sd,msg,&dest_addr,argv[3]);
        receive(sd,msg,argv[3]);
        cout<<"Received Message : "<<msg<<endl;
        cout<<"*********"<<endl;
    }

    // strcpy(msg,"ECHO REQ ");
    // s = to_string(reqno++);
    // // char msg[1000] = "ECHO REQ ";
    // strcat(msg,s.c_str());
    // cout<<msg<<endl;
    // dest_addr.sin_family = AF_INET;
    // dest_addr.sin_port = htons(atoi(argv[2]));
    // dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
    // send(sd,msg,&dest_addr,argv[3]);
    // receive(sd,msg,argv[3]);
    // cout<<"Message : "<<msg<<endl;
    // cout<<"*********"<<endl;

    close_conn_client(sd,&dest_addr,argv[3]);

    return 0;
} 