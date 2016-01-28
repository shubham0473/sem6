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
#define SOCKET_NO 23481

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;

char *server = "10.5.18.68";
char *user = "13CS30030";
char *password = "cse12";
char *database = "13CS30030";
char *table = "tb_news";

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



int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    int type=0;
    char head[100], body[1000], date[20];
    char query[10000];


    init_sql();

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( SOCKET_NO );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    while(1){
        read_size = recv(client_sock, client_message, 2000, 0);

        if(client_message){
            if(strcmp(client_message, "reporter") == 0) {
                write(client_sock, "ok", strlen("ok"));
                read_size = recv(client_sock, client_message, 2000, 0);
                if(strcmp(client_message, "academic") == 0) type = ACADEMIC;
                else type = NON_ACADEMIC;
                read_size = recv(client_sock, client_message, 2000, 0);
                strcpy(date, client_message);
                read_size = recv(client_sock, client_message, 2000, 0);
                strcpy(head, client_message);
                read_size = recv(client_sock, client_message, 2000, 0);
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
                write(client_sock, "ok", strlen("ok"));
                read_size = recv(client_sock, client_message, 2000, 0);
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
                    write(client_sock, client_message, strlen(client_message));
                }
                write(client_sock, "END", strlen("END"));

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
    close(socket_desc);
    return 0;
}
