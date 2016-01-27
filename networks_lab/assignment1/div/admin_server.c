#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num = 23465;
#define BUF_SIZE 30

void main () 
{
    int rst; // Return status of functions.


    /***************** Create a socket *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_DGRAM, 0); 
    if (sfd == -1) 
    {
        perror ("Server_1.c socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);
    



    


    /***************** Binding the server to an address. *************/
    struct sockaddr_in srv_addr, cl_addr; // For sever and client addresses.
    socklen_t addrlen = sizeof (struct sockaddr_in);
    
    
    /* Clear the two addresses. */
    memset (&srv_addr, 0, addrlen);
    memset (&cl_addr, 0, addrlen);
    
    /* Assign a server address. */
    srv_addr.sin_family = AF_INET; // IPv4
    srv_addr.sin_port   = htons (port_num);
    
    
    /* The servers IP address. */
    rst = inet_pton (AF_INET, "127.0.0.1", &srv_addr.sin_addr);
    if (rst <= 0)
    {
        perror ("Server Presentation to network address conversion.\n");
        exit (1);
    }
    
    rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen);
    if (rst < 0)
    {
        perror ("Server: bind failed");
        exit (1);
    }





    /****************** Send - receive messages **********************/
    /* Buffer for receiving a message. */
    char buf[BUF_SIZE] = {'\0'};
    int b_recv   = 0; // Number of bytes received.
    
    // Flags for recvfrom.
    int flags = 0;

    b_recv = recvfrom (sfd, buf, BUF_SIZE, flags, 
                        (struct sockaddr *) &cl_addr, &addrlen);
    if (b_recv == -1)
    {
        perror ("Server: recvfrom failed");
        exit (1);
    }
    
    printf ("Date received = |%s|\n", buf); //////////////////USE IN SQL QUERY
	






    
    /* Printing client's address.
    const char *buf2 = inet_ntop (AF_INET, (struct sockaddr *) &cl_addr, buf, BUF_SIZE);
    if (buf2 == NULL)
    {
        perror ("Server: Couldn't convert client's address to presentation");
        exit (1);
    }
    
    printf ("Client's address = %s\n", buf2);
    
    buf2 = " World";
    // Send a reply to the client. 
    rst = sendto (sfd, buf2, sizeof (buf2), flags, 
                    (struct sockaddr *) &cl_addr, 
                    sizeof (struct sockaddr_in));
    if (rst < 0)
    {
        perror ("Server: Couldn't send");
        exit (1);
    }
 	*/
    





    /****************** Close ****************************************/
    rst = close (sfd); // Close the socket file descriptor.
    if (rst < 0)
    {
        perror ("Server close failed");
        exit (1);
    }
}
