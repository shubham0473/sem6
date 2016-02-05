#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

//int port_num = 9999;
#define BUF_SIZE 1000

void main (int argc, char* argv[])
{
	int port_num = atoi(argv[2]);
	if(argc < 2) {
		printf("Enter IP of the server\n");
		exit(0);
	}

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
	srv_addr.sin_port   = htons ( port_num ); // Port Number.

	rst = inet_pton (AF_INET, argv[1], &srv_addr.sin_addr); /* To
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


	/**************** Send-Receive messages ************************/
	char buf[BUF_SIZE];

	recv(sfd, buf, 1000, 0);
	printf("Server: %s\n", buf);

	FILE* request_stream = fopen("Booking.csv", "r");
	FILE* response_stream = fopen("Responses.csv", "w");
	char line[1024];
	while (fgets(line, 1024, request_stream))
	{
		sleep(5);
		printf("Sending: %s\n", line);
		write(sfd, line, strlen(line));
		// recv(sfd, buf, 1000, 0);
		// if(strcmp(buf, "ok") != 0) {
		// 	fprintf(response_stream, "Could not book: %s\n", line);
		// 	printf("Could not book: %s\n", line);
		// 	continue;
		// }
		recv(sfd, buf, 1000, 0);
		fprintf(response_stream, "%s\n", buf);
		printf("Response: %s\n", buf);

	}
	//write(sfd, "END", strlen("END"));


	/****************** Close ****************************************/
	rst = close (sfd); // Close the socket file descriptor.
	if (rst == -1)
	{
		perror ("Client close failed");
		exit (1);
	}


}
