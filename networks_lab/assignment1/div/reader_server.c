#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num = 23465;
#define BUF_SIZE 1000

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
    rst = listen (sfd, SOMAXCONN);
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
    
    /****************** Send - receive messages **********************/

    char buf[BUF_SIZE];
    rst = recv(cfd, buf, BUF_SIZE, 0);
    if (rst == -1)
    {
        perror ("Server read failed");
        exit (1);
    }
    if(!strcmp(buf, "reader"))
    {
        rst = send(cfd, "ok", BUF_SIZE, 0);
	if (rst == -1)
    	{
    		perror ("Server: Send failed");
    		exit (1);
    	}
    }
    else
    {
        printf("Unknown client\n");
    }
    
    rst = recv(cfd, buf, BUF_SIZE, 0); //Receive newsgroup
    if (rst == -1)
    {
        perror ("Server: read failed");
        exit (1);
    }
    printf("Newsgroup: %s\n", buf); //USE THIS TO NARROW SQL QUERY
	
	/////////////LOOP This part for each news article
	
	int dummyno = 5;
	char* dummyhead = "Nehruites rediscover joy of socket programming";
	sprintf(buf, "%d. %s", dummyno, dummyhead);

	rst = send(cfd, buf, BUF_SIZE, 0); //send article no and head
	if (rst == -1)
    	{
    		perror ("Server: Send failed");
    		exit (1);
    	}
		

	//////////////////

    rst = send(cfd, "END", BUF_SIZE, 0); //End of headlines
    if (rst == -1)
    {
    	perror ("Server: Send failed");
    	exit (1);
    }


    rst = recv(cfd, buf, BUF_SIZE, 0); //Receive selected article no
    if (rst == -1)
    {
        perror ("Server read failed");
        exit (1);
    }
    printf("Article no.: %d\n", atoi(buf));
   
     
    char* dummybody = "Shubham Agrawal and Divyansh Gupta, third year undergraduate students of the Department of Computer Science and Engineering, successfully completed their networks lab assignment in one night. This assignment was perceived to be rather tough even by the maggus of their department.";
    rst = send(cfd, dummybody, BUF_SIZE, 0); //Send fulltext of selected article
    if (rst == -1)
    {
    	perror ("Server: Send failed");
    	exit (1);
    }
    
    
    /****************** Close ****************************************/
    rst = close (sfd); // Close the socket file descriptor.
    if (rst == -1)
    {
        perror ("Server close failed");
        exit (1);
    }
    
}
