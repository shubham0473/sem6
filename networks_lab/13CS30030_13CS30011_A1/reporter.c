#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>

#define ACADEMIC 1
#define NON_ACADEMIC 2
#define SOCKET_NO 23481
#define BUF_SIZE 1000


int main(int argc , char *argv[])
{
    int sock, rst;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET ,SOCK_STREAM , IPPROTO_TCP);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    // server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( SOCKET_NO );

    rst = inet_pton (AF_INET, "127.0.0.1", &server.sin_addr); /* To
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        return 1;
    }
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //keep communicating with server
    /**************** Send-Receive messages ************************/
    char buf[BUF_SIZE] = {'\0'};

    printf ("Sending reporter identification.\n");
    rst = send (sock, "reporter", BUF_SIZE, 0);
    if (rst == -1)
    {
        perror ("Client: Send failed");
        exit (1);
    }

    printf ("Waiting to receive acknowledgement.\n");
    rst = recv (sock, buf, BUF_SIZE, 0);
    if (rst == -1)
    {
        perror ("Client: Receive failed");
        exit (1);
    }
    if(!strcmp(buf, "ok"))
    {
        int option;
		printf("Select newsgroup (1. Academic 2.Non-academic):");
		scanf("%d", &option);
		if(option == 1) {
			rst = send(sock, "academic", BUF_SIZE, 0);
		}
		else {
			rst = send(sock, "non-academic", BUF_SIZE, 0);
		}
    	if (rst == -1)
    	{
    	   	perror ("Client: Send failed");
    	   	exit (1);
    	}

        printf("Enter date of news article (yyyymmdd): ");
        scanf("%s", buf);
        rst = send (sock, buf, BUF_SIZE, 0);
            if (rst == -1)
            {
                perror ("Client: Send failed");
                exit (1);
            }

        printf("Enter headline: ");
        getchar();
        fgets(buf, BUF_SIZE, stdin);
        rst = send (sock, buf, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed");
            exit (1);
        }

        printf("Enter body: ");
        fgets(buf, BUF_SIZE, stdin);
        rst = send (sock, buf, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed");
            exit (1);
        }
     }

    close(sock);
    return 0;
}
