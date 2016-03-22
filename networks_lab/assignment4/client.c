#include <stdio.h>
#include <string.h> // For memset.
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().

void send_email();
int show_menu();
void retrieve_email();
int connect_server();

int debug;

#define BUF_SIZE 10000

int show_menu() {
	printf("\n1. Send new email\n");
	printf("2. Retrieve emails\n");
	printf("0. Exit Garuda Client\n");
	printf("Enter option:	");
	int choice;
	scanf("%d", &choice);
	switch(choice) {
		case 1 :
			send_email();
			return 0;
		case 2 :
			retrieve_email();
			return 0;
		case 0 :
			return 1;
		default:
			printf("Invalid input! Try again\n");
			return 0;
	}
}

void send_email() {
	int sfd = connect_server();
	if(sfd == -1) return;

	char buf[BUF_SIZE], reply_message[BUF_SIZE];
	int reply_code;
	memset(reply_message, 0, BUF_SIZE);

	//// Receive welcome message
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	sscanf(buf, "%d %s", &reply_code, reply_message);
	if(reply_code != 220) {
		printf("Error: %s\n", reply_message);
		close(sfd);
		return;
	}

	//// Send MAIL FROM:
	char sender[100];
	printf("Enter sender:	");
	scanf("%s", sender);
	sprintf(buf, "MAIL FROM: %s\r\n", sender);
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	sscanf(buf, "%d %s", &reply_code, reply_message);
	if(reply_code != 250) {
		printf("Error: %s\n", reply_message);
		close(sfd);
		return;
	}

	//// Send RCPT TOs
	int more_rcpts = 0;
	do {
		char receiver[100];
		printf("Enter receiver:	");
		scanf("%s", receiver);
		sprintf(buf, "RCPT TO: %s\r\n", receiver);
		write(sfd, buf, strlen(buf));
		memset(buf, '\0', BUF_SIZE);
		recv(sfd, buf, BUF_SIZE, 0);
		sscanf(buf, "%d ", &reply_code);
		if(reply_code != 250 && reply_code != 251) {
			printf("Error: %s\n", reply_message);
			close(sfd);
			return;
		}
		printf("Add more recipients [y/n]?:	");
		char temp;
		getchar();
		scanf("%c", &temp);
		if(temp == 'y' || temp == 'Y') more_rcpts = 1;
		else more_rcpts = 0;
	} while(more_rcpts);

	//// Send DATA and message
	write(sfd, "DATA\r\n", strlen("DATA\r\n"));
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	sscanf(buf, "%d %s", &reply_code, reply_message);
	if(reply_code != 354) {
		printf("Error: %s\n", reply_message);
		close(sfd);
		return;
	}
	char subject[100];
	FILE* message_file = fopen("temp.txt", "w");
	fprintf(message_file, "Remove this message and enter your mail body here.\n");
	fclose(message_file);
	if(fork() == 0) {
		execlp("gedit", "gedit", "-s", "temp.txt", (char *) NULL);
	}
	printf("Waiting for message body in Gedit\n");
	wait(NULL);
	message_file = fopen("temp.txt", "r");
	char body[BUF_SIZE - 100];
	int pos = 0;
	while((body[pos++] = fgetc(message_file)) != EOF);
	body[--pos] = '\0';
	fclose(message_file);
	remove("temp.txt"); // Remove email from system for security
	sprintf(buf, "%s\r\n.\r\n", body);
	printf("Sending message of %d bytes...\n", pos);
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	memset(buf, '\0', BUF_SIZE);
	int rst = recv(sfd, buf, BUF_SIZE, 0);
	if(rst == -1) {
		printf("Error: Could not receive reply from server\n");
		close(sfd);
		return;
	}
	printf("Reply %d bytes: %s\n", rst, buf);
	sscanf(buf, "%d %s", &reply_code, reply_message);
	if(reply_code != 250) {
		printf("Error: %s\n", reply_message);
		close(sfd);
		return;
	}

	printf("Mail successfully sent!\n");
	close(sfd);
	return;
}

void retrieve_email() {
	int sfd = connect_server();
	if(sfd == -1) return;

	char buf[BUF_SIZE];

	//// Receive greeting
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	if(buf[0] != '+') {
		printf("Error: %s\n", buf);
		close(sfd);
		return;
	}

	//// Send USER
	char user[100];
	printf("Enter full username (eg. testuser@garudaserver.com):	");
	scanf("%s", user);
	sprintf(buf, "USER %s\r\n", user);
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	if(buf[0] != '+') {
		printf("Error: %s\n", buf);
		close(sfd);
		return;
	}

	//// Send PASS
	char* pass = getpass("Enter password:	");
	sprintf(buf, "PASS %s\r\n", pass);
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	if(buf[0] != '+') {
		printf("Error: %s\n", buf);
		close(sfd);
		return;
	}

	printf("User %s logged in to Garuda POP server\n", user);

	//// Try to get STAT
	sprintf(buf, "STAT\r\n");
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n", buf);
	if(buf[0] == '+') {
		int num_maildrop, size_maildrop;
		char temp[10];
		sscanf(buf, "%s %d %d", temp, &num_maildrop, &size_maildrop);
		printf("You have %d new mails! (Total size = %d Bytes)\n", num_maildrop, size_maildrop);
	}
	else {
		printf("STATs unavailable\n");
	}

	//// Try to get LIST
	sprintf(buf, "LIST\r\n");
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	char mail_list[BUF_SIZE];
	memset(mail_list, '\0', BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply: %s\n\n", buf);
	if(buf[0] != '+') {
		printf("Error: %s\n", buf);
		close(sfd);
		return;
	}
	strcat(mail_list, buf + 4);
	while(1) {
		memset(buf, '\0', BUF_SIZE);
		sleep(1);
		if(recv(sfd, buf, BUF_SIZE, MSG_DONTWAIT) == -1) break;
		printf("Reply partial list: %s\n", buf);
		strcat(mail_list, buf);
	}

	memset(buf, '\0', BUF_SIZE);
	if(recv(sfd, buf, BUF_SIZE, MSG_DONTWAIT) != -1)
		printf("Reply TESTING: %s\n", buf);

	printf("\nYou have the following emails:\nMail-id	Size\n%s\n", mail_list);

	//// Try to RETR
	printf("Enter mail id to read:	");
	int mail_id;
	scanf("%d", &mail_id);
	sprintf(buf, "RETR %d\r\n", mail_id);
	write(sfd, buf, strlen(buf));
	if(debug) printf("Sent: %s\n", buf);
	memset(buf, '\0', BUF_SIZE);
	int rcv = recv(sfd, buf, BUF_SIZE, 0);
	if(debug) printf("Reply %d bytes: %s\n", rcv, buf);
	if(buf[0] != '+') {
		printf("Error: %s\n", buf);
		close(sfd);
		return;
	}

	FILE* message_file = fopen("temp.txt", "w");
	for(int i = 6; i < rcv; i++)
	{
		// fprintf(stdout, "%c", buf[i]);
		fprintf(message_file, "%c", buf[i]);
		fflush(message_file);
	}
	if(fork() == 0) {
		execlp("gedit", "gedit", "-s", "temp.txt", (char *) NULL);
	}
	printf("Please close the Gedit window when you're done reading.\n");
	wait(NULL);
	fclose(message_file);
	remove("temp.txt");

	return;
}

int connect_server() {
	char server[20];
	int port;
	printf("\nEnter IP of Garuda Server:	");
	scanf("%s", server);
	printf("Enter port of Garuda Server:	");
	scanf("%d", &port);

	struct sockaddr_in srv_addr; // Addresses of the server.
	socklen_t addrlen = sizeof (struct sockaddr_in); // size of the address.
	memset (&srv_addr, 0, addrlen);
	// Assign values to the server address.
	srv_addr.sin_family = AF_INET; // IPv4.
	srv_addr.sin_port   = htons(port); // Port Number.
	if(inet_pton (AF_INET, server, &srv_addr.sin_addr) <= 0) {
		printf("Invalid server address.\n");
		return -1;
	}

	int sfd = socket(AF_INET, SOCK_STREAM, 0);

	printf ("Trying to connect to server...\n");
	if(connect (sfd, (struct sockaddr *) &srv_addr, addrlen) == -1) {
		printf("Could not connect to server\n");
		return -1;
	}
	printf ("Connected to Garuda Server\n");

	return sfd;
}


int main(int argc, char* argv[]) {
	printf("\nWelcome to Garuda Email Client!\n\n");
	if(argc > 1) debug = 1;
	else debug = 0;

	int exit_flag = 0;
	while(!exit_flag) {
		exit_flag = show_menu();
	}

	printf("\nExiting Garuda Email Client. Bye!\n\n");

	return 0;
}
