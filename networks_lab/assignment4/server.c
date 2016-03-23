#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <mysql.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>

int port_SMTP;
int port_POP;
#define BUF_SIZE 5000

char *server = "10.5.18.68";
char *user = "13CS30011";
char *password = "cse12";
char *database = "13CS30011";
char *mail_table = "networks_4_mail";
char *user_table = "networks_4_user";

char garuda_name[20];


int SMTP_server_fd, POP_server_fd;
int SMTP_client_fd, POP_client_fd;

int status_SMTP, status_POP;

struct sockaddr_in server_SMTP, client_SMTP, server_POP, client_POP;
socklen_t addrlen = sizeof (struct sockaddr_in);


void init_ports(){
	//Create socket for SMTP
	SMTP_server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (SMTP_server_fd == -1)
	{
		printf("Could not create socket");
	}
	// printf("Socket SMTP fd = %d\n", SMTP_server_fd);

	server_SMTP.sin_family = AF_INET;
	server_SMTP.sin_addr.s_addr = INADDR_ANY;
	server_SMTP.sin_port = htons( port_SMTP );

	//Bind SMTP socket
	status_SMTP =  bind(SMTP_server_fd,(struct sockaddr *)&server_SMTP , sizeof(server_SMTP));
	if( status_SMTP < 0)
	{
		//print the error message
		perror("bind failed. Error");
		exit(1);
	}
	puts("SMTP bind done");

	//Create socket for POP
	POP_server_fd = socket(AF_INET , SOCK_STREAM , 0);
	if (POP_server_fd == -1)
	{
		printf("Could not create socket");
	}
	// printf("Socket POP fd = %d\n", POP_server_fd);

	server_POP.sin_family = AF_INET;
	server_POP.sin_addr.s_addr = INADDR_ANY;
	server_POP.sin_port = htons( port_POP );

	//Bind POP socket
	status_POP =  bind(POP_server_fd,(struct sockaddr *)&server_POP , sizeof(server_POP));
	if( status_POP < 0)
	{
		//print the error message
		perror("bind failed. Error");
		exit(1);
	}
	puts("POP bind done");

}

void run_SMTP() {
	listen(SMTP_server_fd , 5);

	while(1){
		puts("Waiting for incoming connections...");
		SMTP_client_fd = accept(SMTP_server_fd, (struct sockaddr *)&client_SMTP, (socklen_t*)&addrlen);
		if (SMTP_client_fd < 0)	{
			perror("accept failed");
			continue;
		}

		if(fork() == 0) { // fork to handle client request
			char client_ip[50];
			printf("New client connected\n");
			MYSQL *conn;
			MYSQL_RES *res;
			MYSQL_ROW row;
			conn = mysql_init(NULL);
			/* Connect to database */
			if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
				fprintf(stderr, "%s\n", mysql_error(conn));
				exit(0);
			}

			char buf[BUF_SIZE];
			sprintf(buf, "220 Welcome to %s Garuda SMTP Server", garuda_name);
			write(SMTP_client_fd, buf, strlen(buf));
			while(1) {
				memset(buf, '\0', BUF_SIZE);
				recv(SMTP_client_fd, buf, BUF_SIZE, 0);
				char command[20];
				sscanf(buf, "%s", command);

				if(!strcmp(command, "MAIL")) {
					char* temp = strstr(buf, ":");
					char sender[100];
					sscanf(temp, ": %s", sender);
					sprintf(buf, "250 Ok");
					write(SMTP_client_fd, buf, strlen(buf));
					memset(buf, '\0', BUF_SIZE);
					recv(SMTP_client_fd, buf, BUF_SIZE, 0);
					sscanf(buf, "%s", command);

					if(!strcmp(command, "RCPT")) {
						char* temp = strstr(buf, ":");
						char receiver[100];
						sscanf(temp, ": %s", receiver);
						sprintf(buf, "250 Ok");
						write(SMTP_client_fd, buf, strlen(buf));
						memset(buf, '\0', BUF_SIZE);
						recv(SMTP_client_fd, buf, BUF_SIZE, 0);
						sscanf(buf, "%s", command);

						if(!strcmp(command, "DATA")) {
							sprintf(buf, "354 End data with <CR><LF>.<CR><LF>");
							write(SMTP_client_fd, buf, strlen(buf));
							memset(buf, '\0', BUF_SIZE);
							recv(SMTP_client_fd, buf, BUF_SIZE, 0);
							char* temp = strstr(buf, "\r\n.\r\n");
							char body[100];
							if(temp != NULL) {
								int pos = 0;
								for(char* ptr = buf; ptr != temp; ptr++) {
									body[pos++] = *ptr;
								}
								body[pos] = '\0';
							}
							else {
								sprintf(buf, "550 <CRLF>.<CRLF> expected");
								write(SMTP_client_fd, buf, strlen(buf));
								break;
							}

							// printf("Sender: %s, Recipient: %s, Data: %s\n", sender, receiver, body);

							char query[100];
							sprintf(query, "INSERT INTO networks_4_mail(sender, recipient, data, timestamp, domain) VALUES ('%s', '%s', '%s', NOW(), '%s')", sender, receiver, body, garuda_name);

							// printf("QUERY: %s\n\n", query);

							if (mysql_query(conn, query)) {
								fprintf(stderr, "%s\n", mysql_error(conn));
								break;
							}
							printf("MAIL SENT! Sender: %s, Recipient: %s, Data: %s\n", sender, receiver, body);

							//TODO: TEST if mail is being added to DB

							sprintf(buf, "250 Message received");
							write(SMTP_client_fd, buf, strlen(buf));

						}
						else {
							sprintf(buf, "550 Expecting DATA");
							write(SMTP_client_fd, buf, strlen(buf));
							break;
						}
					}
					else {
						sprintf(buf, "550 Expecting RCPT");
						write(SMTP_client_fd, buf, strlen(buf));
						break;
					}
				}
				else if (!strcmp(command, "VRFY")) {
					break;
				}
				else if (!strcmp(command, "QUIT")) {
					sprintf(buf, "221 BYE!");
					write(SMTP_client_fd, buf, strlen(buf));
					break;
				}
			}


			// sprintf(query, "SELECT newsID, head FROM tb_news WHERE tb_news.type = %d ORDER BY tb_news.news_date DESC", type);
			// if (mysql_query(conn, query)) {
			// 	fprintf(stderr, "%s\n", mysql_error(conn));
			// 	exit(1);
			// }
			// res = mysql_use_result(conn);
			// while ((row = mysql_fetch_row(res)) != NULL){
			// 	sprintf(server_message, "%s | %s\n", row[0], row[1]);
			// 	write(SMTP_client_fd, server_message, strlen(server_message));
			// 	recv(SMTP_client_fd, client_message, 2000, 0);
			// 	if(strcmp(client_message, "ok") != 0) break;
			// }


			close(SMTP_client_fd);
			mysql_free_result(res);
			mysql_close(conn);
			exit(0);
		}
	}
}

void run_POP() {

	listen(POP_server_fd , 5);

	while(1){
		puts("Waiting for incoming connections...");
		POP_client_fd = accept(POP_server_fd, (struct sockaddr *)&client_POP, (socklen_t*)&addrlen);
		if (POP_client_fd < 0)	{
			perror("accept failed");
			continue;
		}

		if(fork() == 0) { // fork to handle client request
			char client_ip[50];
			printf("New client connected\n");
			MYSQL *conn;
			MYSQL_RES *res;
			MYSQL_ROW row;
			conn = mysql_init(NULL);
			/* Connect to database */
			if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
				fprintf(stderr, "%s\n", mysql_error(conn));
				exit(0);
			}

			char buf[BUF_SIZE];
			sprintf(buf, "+Ok Welcome to %s Garuda POP Server", garuda_name);
			write(POP_client_fd, buf, strlen(buf));
			char user[100], pass[100];
			while(1) {
				memset(buf, '\0', BUF_SIZE);
				recv(POP_client_fd, buf, BUF_SIZE, 0);
				char command[20];

				sscanf(buf, "%s %s", command, user);

				if(!strcmp(command, "USER")) {
					sprintf(buf, "+Ok");
					write(POP_client_fd, buf, strlen(buf));

					memset(buf, '\0', BUF_SIZE);
					recv(POP_client_fd, buf, BUF_SIZE, 0);
					sscanf(buf, "%s %s", command, pass);
					if(!strcmp(command, "PASS")) {
						char query[1000];
						sprintf(query, "SELECT * FROM networks_4_user WHERE user = '%s' AND password = '%s' AND domain = '%s'", user, pass, garuda_name);
						// printf("QUERY: %s", query);
						if (mysql_query(conn, query)) {
							fprintf(stderr, "%s\n", mysql_error(conn));
							exit(1);
						}
						res = mysql_use_result(conn);
						if ((row = mysql_fetch_row(res)) != NULL){
							sprintf(buf, "+Ok");
							write(POP_client_fd, buf, strlen(buf));
						}
						mysql_free_result(res);
					}
					else {
						sprintf(buf, "-Err");
						write(POP_client_fd, buf, strlen(buf));
						continue;
					}
				}
				else if (!strcmp(command, "LIST")) {
					char query[1000];
					sprintf(query, "SELECT * FROM networks_4_mail WHERE recipient = '%s' AND domain = '%s'", user, garuda_name);
					// printf("QUERY: %s", query);

					if (mysql_query(conn, query)) {
						fprintf(stderr, "%s\n", mysql_error(conn));
						exit(1);
					}
					res = mysql_use_result(conn);
					memset(buf, 0, BUF_SIZE);
					sprintf(buf, "+Ok\n");
					while ((row = mysql_fetch_row(res)) != NULL){
						char str[10];
						sprintf(str, "%s\t%d\n", row[5], (int)strlen(row[2]));
						strcat(buf, str);
					}
					mysql_free_result(res);

					write(POP_client_fd, buf, strlen(buf));
					continue;
				}
				else if (!strcmp(command, "RETR")) {
					int msgid;
					sscanf(buf, "%s %d", command, &msgid);

					char query[1000];
					sprintf(query, "SELECT * FROM networks_4_mail WHERE mailID = '%d'", msgid);
					// printf("QUERY: %s", query);

					if (mysql_query(conn, query)) {
						fprintf(stderr, "%s\n", mysql_error(conn));
						exit(1);
					}
					res = mysql_use_result(conn);
					memset(buf, 0, BUF_SIZE);
					sprintf(buf, "+Ok\n");
					if ((row = mysql_fetch_row(res)) != NULL){
						sprintf(buf, "+Okk\n\nSender: %s\nReceiver: %s\nBody: %s\nTimestamp: %s\n\n", row[0], row[1], row[2], row[3]);
					}
					mysql_free_result(res);

					write(POP_client_fd, buf, strlen(buf));
					continue;

				}
				else if (!strcmp(command, "STAT")) {
					char query[1000];
					sprintf(query, "SELECT * FROM networks_4_mail WHERE recipient = '%s' AND domain = '%s'", user, garuda_name);
					// printf("QUERY: %s", query);

					if (mysql_query(conn, query)) {
						fprintf(stderr, "%s\n", mysql_error(conn));
						exit(1);
					}
					res = mysql_use_result(conn);
					memset(buf, 0, BUF_SIZE);
					sprintf(buf, "+Ok\n");
					int num_maildrop = 0, size_maildrop = 0;
					while ((row = mysql_fetch_row(res)) != NULL){
						num_maildrop++;
						size_maildrop += strlen(row[2]);
					}
					mysql_free_result(res);
					sprintf(buf, "+Ok %d %d", num_maildrop, size_maildrop);

					write(POP_client_fd, buf, strlen(buf));
					continue;
				}
				else if (!strcmp(command, "QUIT")) {
					sprintf(buf, "+Ok OKBYE!");
					write(POP_client_fd, buf, strlen(buf));
					break;
				}
				else {
					sprintf(buf, "-Err");
					write(POP_client_fd, buf, strlen(buf));
					continue;
				}
			}

			close(POP_client_fd);
			mysql_close(conn);
			exit(0);
		}
	}
}

int main(int argc , char *argv[])
{
	if(argc < 2) {
		printf("Usage: ./server <abc.com/xyz.com>\n");
		exit(0);
	}
	printf("Select a port for SMTP:	");
	scanf("%d", &port_SMTP);
	printf("Select a port for POP:	");
	scanf("%d", &port_POP);

	strcat(garuda_name, argv[1]);

	init_ports();

	if(fork() == 0) {
		printf("Starting SMTP server\n");
		run_SMTP();
		close(SMTP_server_fd);
		printf("SMTP server shutting down\n");
	}
	else {
		printf("Starting POP server\n");
		run_POP();
		close(POP_server_fd);
		printf("POP server shutting down\n");
	}

	return 0;

}
