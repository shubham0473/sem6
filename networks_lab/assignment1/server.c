#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <mysql.h>

#define ACADEMIC 1
#define NON_ACADEMIC 2
#define MAX_HEAD_SIZE 100
#define MAX_BODY_SIZE 1000
#define SOCKET_TCP 23481
#define SOCKET_UDP 23465

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

char *server = "10.5.18.68";
char *user = "13CS30030";
char *password = "cse12";
char *database = "13CS30030";
char *table = "tb_news";

int sock_UDP;
int sock_TCP;
int sock_TCP_client;

int status_TCP;
int status_UDP;

struct sockaddr_in server_UDP, client_UDP; // For sever and client addresses.
struct sockaddr_in server_TCP, client_TCP;
socklen_t addrlen = sizeof (struct sockaddr_in);


void init_sql(){
    conn = mysql_init(NULL);
   /* Connect to database */
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      return;
   }
   else printf("connection successful\n");
}

void init_ports(){
    //Create socket for TCP
    sock_TCP = socket(AF_INET , SOCK_STREAM , 0);
    if (sock_TCP == -1)
    {
        printf("Could not create socket");
    }
    printf("Socket TCP fd = %d\n", sock_TCP);

    server_TCP.sin_family = AF_INET;
    server_TCP.sin_addr.s_addr = INADDR_ANY;
    server_TCP.sin_port = htons( SOCKET_TCP );

    //Create socket for UDP
    sock_UDP = socket (AF_INET, SOCK_DGRAM, 0);
    if (sock_UDP == -1)
    {
        perror ("Server_1.c socket error");
        exit (1);
    }
    printf ("Socket UDP fd = %d\n", sock_UDP);
    socklen_t addrlen = sizeof (struct sockaddr_in);

    /* Clear the two addresses. */
    memset (&server_UDP, 0, addrlen);
    memset (&client_UDP, 0, addrlen);

    /* Assign a server address. */
    server_UDP.sin_family = AF_INET; // IPv4
    server_UDP.sin_port   = htons (SOCKET_UDP);

    status_UDP = inet_pton (AF_INET, "127.0.0.1", &server_UDP.sin_addr);
    if (status_UDP <= 0)
    {
        perror ("Server Presentation to network address conversion.\n");
        exit (1);
    }

}

void bind_sockets(){
    //Bind TCP socket
    status_TCP =  bind(sock_TCP,(struct sockaddr *)&server_TCP , sizeof(server_TCP));
    if( status_TCP < 0)
    {
        //print the error message
        perror("bind failed. Error");
        exit(1);
    }
    puts("TCP bind done");

    status_UDP = bind (sock_UDP, (struct sockaddr *) &server_UDP, addrlen);
    if (status_UDP < 0)
    {
        perror ("Server: bind failed");
        exit (1);
    }
    puts("UDP bind done");
}


int main(int argc , char *argv[])
{
    int read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    int type=0;
    char head[100], body[1000], date[20];
    char query[10000];

    init_sql();
    init_ports();
    bind_sockets();

    //Listen TCP
    listen(sock_TCP , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");

    //accept connection from an incoming client
    sock_TCP_client = accept(sock_TCP, (struct sockaddr *)&client_TCP, (socklen_t*)&addrlen);
    if (sock_TCP_client < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    while(1){
        read_size = recv(sock_TCP_client, client_message, 2000, 0);

        if(client_message){
            if(strcmp(client_message, "reporter") == 0) {
                write(sock_TCP_client, "ok", strlen("ok"));
                read_size = recv(sock_TCP_client, client_message, 2000, 0);
                if(strcmp(client_message, "academic") == 0) type = ACADEMIC;
                else type = NON_ACADEMIC;
                read_size = recv(sock_TCP_client, client_message, 2000, 0);
                strcpy(date, client_message);
                read_size = recv(sock_TCP_client, client_message, 2000, 0);
                strcpy(head, client_message);
                read_size = recv(sock_TCP_client, client_message, 2000, 0);
                strcpy(body, client_message);
                sprintf(query, "INSERT INTO tb_news (head, body, news_date, type) VALUES ('%s', '%s', '%s', %d)"  , head, body, date, type);
                printf("%s\n", query);
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                else printf("query executed\n");
                res = mysql_use_result(conn);

            }
            else if(strcmp(client_message, "reader") == 0){
                write(sock_TCP_client, "ok", strlen("ok"));
                read_size = recv(sock_TCP_client, client_message, 2000, 0);
                if(strcmp(client_message, "academic") == 0) type = ACADEMIC;
                else if(strcmp(client_message, "non-academic") == 0) type = NON_ACADEMIC;
                printf("%d\n", type);
                sprintf(query, "SELECT * FROM tb_news WHERE tb_news.type = %d ORDER BY tb_news.news_date DESC", type);
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                else printf("query executed\n");
                res = mysql_use_result(conn);
                while ((row = mysql_fetch_row(res)) != NULL){
                    strcpy(client_message, row[0]);
                    write(sock_TCP_client, client_message, strlen(client_message));
                }
                write(sock_TCP_client, "END", strlen("END"));

            }

            if(read_size == 0)
            {
                puts("Client disconnected");
                fflush(stdout);
            }
            else if(read_size == -1)
            {
                perror("recv failed");
            }


        }


    }

    mysql_free_result(res);
    mysql_close(conn);
    close(sock_TCP);
    return 0;
}
