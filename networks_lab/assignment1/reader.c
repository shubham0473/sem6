#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

int port_num = 23481;
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
        perror ("Client: socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);



    /***************** Assign an address of the server **************/
    struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.
    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.

    // Clear the two addresses.
    memset (&srv_addr, 0, addrlen);

    // Assign values to the server address.
    srv_addr.sin_family = AF_INET; // IPv4.
    srv_addr.sin_port   = htons (port_num); // Port Number.

    rst = inet_pton (AF_INET, "127.0.0.1", &srv_addr.sin_addr); /* To
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        exit (1);
    }

    printf ("Trying to connect to server\n");
    /***************** Connect to the server ************************/
    rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
    if (rst == -1)
    {
        perror ("Client: Connect failed.");
        exit (1);
    }
    printf ("Connected to server\n");

    //printf ("Going to sleep\n");
    //sleep (3); // For testing purposes.
    //printf ("Out of sleep\n");

    /**************** Send-Receive messages ************************/
    char buf[BUF_SIZE];

    printf ("Sending reader identification.\n");
    rst = send (sfd, "reader", BUF_SIZE, 0);
    if (rst == -1)
    {
        perror ("Client: Send failed");
        exit (1);
    }

    printf ("Waiting to receive acknowledgement.\n");
    rst = recv (sfd, buf, BUF_SIZE, 0);
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
			rst = send(sfd, "academic", BUF_SIZE, 0);
		}
		else {
			rst = send(sfd, "non-academic", BUF_SIZE, 0);
		}
    		if (rst == -1)
    		{
    	   		perror ("Client: Send failed");
    	   		exit (1);
    		}


        while(1){
            rst = recv (sfd, buf, BUF_SIZE, 0);
    			if (rst == -1)
    			{
        			perror ("Client: Receive failed");
        			exit (1);
    			}
                if(strcmp(buf, "END") == 0) break;
                printf("%s", buf);
            rst = send(sfd, "ok", BUF_SIZE, 0);
            memset(buf, 0, sizeof(buf));
            if (rst == -1)
    		{
    	   		perror ("Client: Send failed");
    	   		exit (1);
    		}
        }
		// do{ //receive and display all news headlines
		//
        // }
		// while();

		printf("Select an article to view:");

		scanf("%d", &option);

		sprintf(buf, "%d", option);
		rst = send(sfd, buf, BUF_SIZE, 0); //select an article
		if (rst == -1)
    		{
    	   		perror ("Client: Send failed");
    	   		exit (1);
    		}

		rst = recv (sfd, buf, BUF_SIZE, 0); //receive fulltext
 		if (rst == -1)
 		{
  			perror ("Client: Receive failed");
       			exit (1);
  		}

		//printf("Here is your article:\n%s\n\n", buf); //remove if xterm works

		FILE* fp = fopen("article.txt", "w");
		fprintf(fp, "%s", buf);
		fclose(fp);

		int cid = fork();
		if(cid == 0) {
			printf("Child process says hi!");
			rst = execlp("gedit", "gedit", "article.txt", (const char*) NULL);
			if(rst == -1)
				perror("Error in exec");
		}
		else {
			printf("Parent process says hi!");
			char c;
			scanf("%c", &c);
			if(c == '#') {
				kill(cid, SIGKILL);
			}
		}

     }




    /****************** Close ****************************************/
    rst = close (sfd); // Close the socket file descriptor.
    if (rst == -1)
    {
        perror ("Client close failed");
        exit (1);
    }


}
