#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <mysql.h>

char *server = "10.5.18.68";
char *user = "13CS30030";
char *password = "cse12";
char *database = "13CS30030";
char *table = "tb_seat";

#define TRUE   1
#define FALSE  0
//#define PORT 9999

#define PNUM 0
#define TRAIN 1
#define AC 2
#define QTY 3
#define PREF 4
#define AGE 5

char* field[10];

int get_field(char* line, char* delim)
{
	//printf("Get field called on %s with %s delim\n", line, delim);
	const char* tok;
	int i;

	char* temp;
	//sprintf(temp, "%s\n", delim);
	for (tok = strtok(line, delim), i = 0; tok && *tok;	tok = strtok(NULL, delim), i++) {
		field[i] = (char *) tok;
	}
	return i;
}


int main(int argc , char *argv[])
{
	int PORT = atoi(argv[1]);
	int opt = TRUE;
	int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;

	char buffer[1025];  //data buffer of 1K

	//set of socket descriptors
	fd_set readfds;

	//a message
	char* welcome_message = "Welcome to The Ticket Counter \r\n";

	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple conections , this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", PORT);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	int parent_pid;

	if ((parent_pid = fork()) == 0){ // Child process to display current booking status
		MYSQL *conn;
		MYSQL_RES *res;
		MYSQL_ROW row;
		conn = mysql_init(NULL);
		char query[1000];
		int opt = 0;
		if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
			fprintf(stderr, "%s\n", mysql_error(conn));
			return 0;
		}

		printf("Press 1 to display the current booking status.\n");

		while(TRUE) {
			scanf("%d", &opt);
			if(opt == 1){
				strcpy(query, "select train_name, coach_type, sum(flag) as available, count(*) as total from tb_seat group by coach_type, train_name;");
				if (mysql_query(conn, query)) { // 0 on success
					fprintf(stderr, "%s\n", mysql_error(conn));
					exit(1);
				}
				res = mysql_use_result(conn);
				printf("Train Name \t coach_type \t available \t total\n");
				while ((row = mysql_fetch_row(res)) != NULL){
					int total = atoi(row[3]);
					int flag_sum = atoi(row[2]);
					int booked = flag_sum - total;
					int available = total - booked;
					printf("%s \t %s \t\t %d \t\t %d\n", row[0], row[1], available, total);
				}
				mysql_free_result(res);
				printf("Press 1 to display the current booking status.\n");
			}
		}

	}
	else { // Parent process accepts connections and books tickets
		while(TRUE) {
			//clear the socket set
			FD_ZERO(&readfds);

			//add master socket to set
			FD_SET(master_socket, &readfds);
			max_sd = master_socket;

			//add child sockets to set
			for (i = 0 ; i < max_clients ; i++) {
				//socket descriptor
				sd = client_socket[i];

				//if valid socket descriptor then add to read list
				if(sd > 0)
				FD_SET( sd , &readfds);

				//highest file descriptor number, need it for the select function
				if (sd > max_sd)
				max_sd = sd;
			}

			// Wait for an activity on any of the sockets , timeout is NULL , so wait indefinitely
			// Effectively selects on a first-come-first-serve basis
			activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

			if ((activity < 0) && (errno!=EINTR)) {
				printf("select error");
			}

			//If something happened on the master socket , then its an incoming connection
			if (FD_ISSET(master_socket, &readfds)) {
				if ((new_socket = accept(master_socket, (struct sockaddr*) &address, (socklen_t*) &addrlen))<0) {
					perror("Server: accept error");
					exit(EXIT_FAILURE);
				}

				//inform user of socket number - used in send and receive commands
				printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

				//send new connection greeting message
				if( send(new_socket, welcome_message, strlen(welcome_message), 0) != strlen(welcome_message) ) {
					perror("send");
				}

				puts("Welcome message sent successfully");

				//add new socket to array of sockets
				for (i = 0; i < max_clients; i++) {
					//if position is empty
					if(client_socket[i] == 0 ) {
						client_socket[i] = new_socket;
						printf("Adding to list of sockets as %d\n" , i);
						break;
					}
				}
			}

			//else its some IO operation on some other socket :)
			for (i = 0; i < max_clients; i++) {

				sd = client_socket[i];
				if (FD_ISSET( sd , &readfds)) {

					//Check if it was for closing , and also read the incoming message
					if ((valread = recv(sd , buffer, 1024, 0)) == 0) {
						//Somebody disconnected , get his details and print
						getpeername(sd , (struct sockaddr*) &address , (socklen_t*) &addrlen);
						printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

						//Close the socket and mark as 0 in list for reuse
						close( sd );
						client_socket[i] = 0;
					}
					//Else a client requested for booking
					else {
						printf("\nBooking request: %s\n", buffer);
						get_field(buffer, ",");

						int passID = atoi(field[PNUM]);
						int train_no = atoi(field[TRAIN]);
						int berth = atoi(field[QTY]);
						char* coach_type = field[AC];

						// printf("passID = %d, train_no = %d, berth = %d, coach_type = %s\n", passID, train_no, berth, coach_type);

						char* pref_list = strdup(field[PREF]);
						get_field(pref_list, "-");
						char* pref[berth];
						for(int i = 0; i < berth; i++) {
							pref[i] = strdup(field[i]);
							//printf("pref %d = %s\n", i, pref[i]);
						}

						int booked = 0;
						char response[berth][100]; // seat allocation of individual passengers
						char server_response[100]; // final response by server
						sprintf(server_response, "%d,", passID);

						// Connect with DB
						MYSQL *conn;
						MYSQL_RES *res;
						MYSQL_ROW row;
						conn = mysql_init(NULL);
						char query[1000], query2[1000];
						int opt = 0;
						if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
							fprintf(stderr, "%s\n", mysql_error(conn));
							return 0;
						}

						// printf("Connecting to database...\n");

						// Query total availability in train
						sprintf(query, "SELECT coach_type, count(*) as available FROM tb_seat WHERE flag = 1 and train_no = %d and coach_type = '%s'", train_no, coach_type);
						if (mysql_query(conn, query)) {
							fprintf(stderr, "%s\n", mysql_error(conn));
							exit(1);
						}
						res = mysql_store_result(conn);
						row = mysql_fetch_row(res);

						// If train has enough berths, check if a coach has enough berths
						if(atoi(row[1]) >= berth){

							printf("%s seats available in train %d\n", row[1], train_no);
							mysql_free_result(res);

							// Query availability in coaches
							sprintf(query, "SELECT coach_no, count(*) as available FROM tb_seat WHERE flag = 1 and train_no = %d and coach_type = '%s' group by coach_no", train_no, coach_type);
							if (mysql_query(conn, query)) {
								fprintf(stderr, "%s\n", mysql_error(conn));
								exit(1);
							}
							res = mysql_store_result(conn);

							// Check if any coach has enough berths
							while((row = mysql_fetch_row(res)) != NULL){
								if(atoi(row[1]) >= berth) {
									printf("%s seats available in coach %s\n", row[1], row[0]);
									// Try to book as per prefernce
									for(booked = 0; booked < berth;){
										int seat_booked = 0;

										sprintf(query2, "UPDATE tb_seat SET flag = 3 WHERE flag = 1 and seat_type = '%s' and coach_no = '%s' and train_no = %d LIMIT 1", pref[booked], row[0], train_no);
										// printf("\nAttempting: %s\n", query2);
										if (mysql_query(conn, query2)) {
											fprintf(stderr, "%s\n", mysql_error(conn));
											exit(1);
										}
										if((int) mysql_affected_rows(conn) != 0){
											seat_booked = 1;
											// printf("Seat Booked\n");
											mysql_query(conn,"SELECT coach_no, seat_no, seat_type FROM tb_seat WHERE flag = 3");
											// printf("Queried");
											MYSQL_RES *res1 = mysql_store_result(conn);
											// printf("Stored");
											MYSQL_ROW row1 = mysql_fetch_row(res1);
											// printf("Fetched");
											sprintf(response[booked], "%s/%s/%s", row1[0], row1[1], row1[2]);
											// printf("AA. Passenger %d: %s\n", booked, response[booked]);
											mysql_free_result(res1);
											mysql_query(conn, "UPDATE tb_seat SET flag = 2 WHERE flag = 3");
											booked++;
											continue;
										}

										// If prefered seat not available,
										// book any available seat in coach
										if(seat_booked == 0){
											sprintf(query2, "UPDATE tb_seat SET flag = 3 WHERE flag = 1 and coach_no = '%s' and train_no = %d LIMIT 1", row[0], train_no);
											// printf("\nAttempting: %s\n", query2);
											if (mysql_query(conn, query2)) {
												fprintf(stderr, "%s\n", mysql_error(conn));
												exit(1);
											}
											if((int) mysql_affected_rows(conn) != 0){
												seat_booked = 1;
												// printf("Seat Booked\n");
												mysql_query(conn,"SELECT coach_no, seat_no, seat_type FROM tb_seat WHERE flag = 3");
												// printf("Queried");
												MYSQL_RES *res1 = mysql_store_result(conn);
												// printf("Stored");
												MYSQL_ROW row1 = mysql_fetch_row(res1);
												// printf("Fetched");
												sprintf(response[booked], "%s/%s/%s", row1[0], row1[1], row1[2]);
												// printf("AB. Passenger %d: %s\n", booked, response[booked]);
												mysql_free_result(res1);
												mysql_query(conn, "UPDATE tb_seat SET flag = 2 WHERE flag = 3");
												booked++;
												continue;
											}
										}
									} // All seats booked
									// All seats booked in same coach
									break;
								}
							}
							mysql_free_result(res);

							// No coach has enough seats, staggered alotment
							while(booked < berth){
								int seat_booked = 0;

								// Try to give prefered seat
								sprintf(query2, "UPDATE tb_seat SET flag = 3 WHERE flag = 1 and seat_type = '%s'and train_no = %d LIMIT 1", pref[booked], train_no);
								// printf("\nAttempting: %s\n", query2);
								if (mysql_query(conn, query2)) {
									fprintf(stderr, "%s\n", mysql_error(conn));
									exit(1);
								}
								if((int) mysql_affected_rows(conn) != 0){
									seat_booked = 1;
									mysql_query(conn,"SELECT coach_no, seat_no, seat_type FROM tb_seat WHERE flag = 3");
									MYSQL_RES *res1 = mysql_store_result(conn);
									MYSQL_ROW row1 = mysql_fetch_row(res1);
									sprintf(response[booked], "%s/%s/%s", row1[0], row1[1], row1[2]);
									// printf("BA. Passenger %d: %s\n", booked, response[booked]);
									mysql_free_result(res1);
									mysql_query(conn, "UPDATE tb_seat SET flag = 2 WHERE flag = 3");
									booked++;
									continue;
								}

								// If prefered seat not available,
								// book any available seat in coach
								if(seat_booked == 0){
									sprintf(query2, "UPDATE tb_seat SET flag = 3 WHERE flag = 1 and train_no = %d LIMIT 1", train_no);
									// printf("\nAttempting: %s\n", query2);
									if (mysql_query(conn, query2)) {
										fprintf(stderr, "%s\n", mysql_error(conn));
										exit(1);
									}
									if((int) mysql_affected_rows(conn) != 0){
										seat_booked = 1;
										mysql_query(conn,"SELECT coach_no, seat_no, seat_type FROM tb_seat WHERE flag = 3");
										MYSQL_RES *res1 = mysql_store_result(conn);
										MYSQL_ROW row1 = mysql_fetch_row(res1);
										sprintf(response[booked], "%s/%s/%s", row1[0], row1[1], row1[2]);
										// printf("BB. Passenger %d: %s\n", booked, response[booked]);
										mysql_free_result(res1);
										mysql_query(conn, "UPDATE tb_seat SET flag = 2 WHERE flag = 3");
										booked++;
										continue;
									}
								}
							}

							for(int i = 0; i < berth; i++) {
								if(i != 0) strcat(server_response, "-");
								strcat(server_response, response[i]);
								// printf("server_response = %s", server_response);
							}
						}
						else { //Not enough seats in train
								mysql_free_result(res);
								strcat(server_response, "NA");
						}

						printf("Server response: %s\n", server_response);
						write(sd, server_response, strlen(server_response));
					}
				}
			}
		}
	}



	return 0;
}
