
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>

#include <sys/socket.h> // For the socket () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton ().

int port_num = 23453;
#define BUF_SIZE 30

void main (int argc , char *argv[])
{
    int rst; // Return status of functions.
    int sfd; // Socket file descriptor.
    if(argc < 2){
        printf("Enter IP of the server\n");
        exit(0);
    }
    char buf[BUF_SIZE];

    /***************** Create a socket *******************************/
    sfd = socket (AF_INET, SOCK_DGRAM, 0); /* AF_INET --> IPv4,
    * SOCK_DGRAM --> UDP Protocol, 0 --> for the protocol. */
    if (sfd == -1)
    {
        perror ("Client_1.c socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);



    /****************** Send - receive messages **********************/

    //printf("%s\n", hash);



    char msg_len = 10;
    /* int  flags   = 0 | MSG_DONTWAIT; /* Client doesn't wait even if
    * server is not running.
    * The client will return with EAGAIN if the send-buffer is full.
    * */
    int flags = 0; /* Even Now the client doesn't wait even if server
    * is not running.
    * Now the client will wait if its send-buffer is full.
    * */

    struct sockaddr_in dest_addr; /* sockaddr_in because we are using
    * IPv4. Type casted to struct sockaddr * at time of
    * various system calls. */

    socklen_t addrlen = sizeof (struct sockaddr_in);


    // Initializing destination address.
    memset (&dest_addr, 0, addrlen); // Initializes address to zero.

    dest_addr.sin_family = AF_INET;  // Address is in IPv4 format.
    dest_addr.sin_port   = htons (port_num);  // Port number of the server.

    rst = inet_pton (AF_INET, argv[1], &dest_addr.sin_addr); /* Note
    * that third field should point to an in_addr (in6_addr). */
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        exit (1);
    }


    char* password = getpass("Enter password:");
    char* hash = crypt(password, "DS");

    if(strcmp(hash, "DS1lxBt1j0ltM") == 0){
        printf("LOGIN SUCCESSFUL\n");
    }
    else exit(0);

    printf("Enter date before which you wish to delete articles (yyyymmdd):");
    char msg[10];
    scanf("%s", msg);

    /* Sending message to the server. */
    rst = sendto (sfd, msg, 20, flags, (struct sockaddr *) &dest_addr,
    sizeof (struct sockaddr_in)); /* Value of rst is 20,
    * on successful transmission; i.e. It has nothing to do with a
    * NULL terminated string.

    */
    if (rst < 0)
    {
        perror ("Client: Sendto function call failed");
        exit (1);
    }
    else
    {
        printf ("Sent data size = %d\n", rst);
    }

    rst = recvfrom(sfd, buf, BUF_SIZE, flags, (struct sockaddr *)&dest_addr, &addrlen);
    if (rst == -1)
    {
        perror ("Server: recvfrom failed");
        exit (1);
    }

    if (rst < 0)
    {
        perror ("Client: recvfrom function call failed");
        exit (1);
    }
    else
    {
        printf ("%s\n", buf);
    }
    /*
    char buf[BUF_SIZE] = {'\0'};
    struct sockaddr_in sender_addr;
    socklen_t sender_len;
    // Receive a message from the server.
    rst = recvfrom (sfd, buf, BUF_SIZE, flags,
    (struct sockaddr *) &sender_addr,
    &sender_len);
    if (rst < 0)
    {
    perror ("Client: couldn't receive");
    exit (1);
}
printf ("Message from server = |%s|\n", buf);

// Address of the server.
const char *buf2 = inet_ntop (AF_INET, (struct sockaddr *) &sender_addr, buf,
BUF_SIZE);
if (buf2 == NULL)
{
perror ("Client: Conversion of sender's address to presentation failed");
exit (1);
}

printf ("Servers address = %s\n", buf2);

*/





/****************** Close ****************************************/
rst = close (sfd); // Close the socket file descriptor.
if (rst < 0)
{
    perror ("Client close failed");
    exit (1);
}
}
