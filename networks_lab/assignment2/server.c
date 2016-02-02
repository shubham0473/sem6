#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <sys/select.h> // For select().

int port_num = 23465;
#define BUF_SIZE 30

void main ()
{
    int rst; // Return status of functions.
    int cfd; // File descriptor for the client.

    /**************** Create a socket. *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
                * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
    if (sfd == -1)
    {
        perror ("Server: socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);



    /***************** Assign an address to the server **************/
    struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.
    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.

    // Clear the two addresses.
    memset (&srv_addr, 0, addrlen);
    memset (&cli_addr, 0, addrlen);

    // Assign values to the server address.
    srv_addr.sin_family = AF_INET; // IPv4.
    srv_addr.sin_port   = htons (port_num); // Port Number.

    rst = inet_pton (AF_INET, "127.0.0.1", &srv_addr.sin_addr); /* To
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
        perror ("Server Presentation to network address conversion.\n");
        exit (1);
    }



    /****************** Bind the server to an address. ***************/
    rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen); /* Note
        * the type casting of the pointer to the server's address. */
    if (rst == -1)
    {
        perror ("Server: Bind failed");
        exit (1);
    }



    /***************** listen (): Mark the socket as passive. *******/
    printf ("Max connections allowed to wait: %d\n", SOMAXCONN);
    rst = listen (sfd, 1);
    if (rst == -1)
    {
        perror ("Server: Listen failed");
        exit (1);
    }



    /***************** accept (): Waiting for connections ************/
    cfd = accept (sfd, (struct sockaddr *) &cli_addr, &addrlen); /*
        * Returns the file descriptor for the client.
        * Stores the address of the client in cli_addr. */
    if (cfd == -1)
    {
        perror ("Server: Accept failed");
        exit (1);
    }
    printf ("A Connection accepted\n");



    int cfd2 = accept (sfd, (struct sockaddr *) &cli_addr, &addrlen); /*
        * Returns the file descriptor for the client.
        * Stores the address of the client in cli_addr. */
    if (cfd2 == -1)
    {
        perror ("Server: Accept failed");
        exit (1);
    }
    printf ("A Connection accepted\n");
    printf ("Two client fds == %d, %d\n", cfd, cfd2);


    /**************** Select (): **************************************/
    //printf ("Going to sleep\n");
    //sleep (10); // For testing purposes.
    //printf ("Out of sleep\n");


    fd_set rd, wr, er;

    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&er);
    FD_SET(cfd, &rd);
    FD_SET(cfd2, &rd);

    struct timeval tm;
    tm.tv_sec = 10;
    tm.tv_usec = 0;

    printf ("Select block starts:...\n");
    rst = select (100, &rd, &wr, &er, &tm);
    if (rst == -1)
    {
        perror ("Select failed: ");
        exit (1);
    }
    printf ("Select block ends:...\n");

    printf ("Number of set fds obtained by select() = %d\n", rst);

    if (FD_ISSET (cfd, &rd))
        printf ("Message obtained from child with fd = %d\n", cfd);
    else
        printf ("No message received from child with fd = %d\n", cfd);

    if (FD_ISSET (cfd2, &rd))
        printf ("Message obtained from child with fd = %d\n", cfd2);
    else
        printf ("No message received from child with fd = %d\n", cfd2);






    /****************** Send - receive messages **********************/

    rst = write (cfd, "Hello", 6);
    if (rst == -1)
    {
        perror ("Server write failed");
        exit (1);
    }



    printf ("Final sleep\n");
    sleep (10);
    printf ("Server exiting\n");

    /****************** Close ****************************************/
    printf ("Closing connection...\n");
    rst = close (sfd); // Close the socket file descriptor.
    if (rst == -1)
    {
        perror ("Server close failed");
        exit (1);
    }
}
