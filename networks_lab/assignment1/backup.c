/*
    C socket server example, handles multiple clients using threads
*/

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

//the thread function
void *connection_handler(void *);

char *string_copy(char *dst, char* src, int start, int end){
    int i,j=0;

    for(i = start; i < end; i++, j++){
        dst[j] = src[i];
    }
    return dst;
}

void msg_parser(char *str, int category, char *date, char *head, char *body){

    int i = 0, j;

    while(str[i] == '&' && str[i+1] == '&'){
        if(strcmp(string_copy(date, str, 0, i-1), "academic") == 0) category = ACADEMIC;
        else category = NON_ACADEMIC;
        i++;
    }
    i = i++;
    j = i;
    while(str[i] == '&' && str[i+1] == '&'){
        date = string_copy(date, str, j, i-1);
        printf("%s\n", date);
        i++;
    }
    i = i+2;
    j = i;
    while(str[i] == '&' && str[i+1] == '&'){
        date = string_copy(head, str, j, i-1);
        printf("%s\n", date);
        i++;
    }
    i = i+2;
    j = i;
    while(str[i] == '&' && str[i+1] == '&'){
        date = string_copy(body, str, j, i-1);
        printf("%s\n", date);
        i++;
    }
}


int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    init_sql();

    if(conn == NULL)
    {
        printf("Error %u %s\n", mysql_errno(conn), mysql_error(conn));
      exit(1);
    }

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
    server.sin_port = htons( 23465 );

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


    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[MAX_BODY_SIZE];

    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));

    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));

    
    //Receive a message from client
    while( (read_size = recv(sock , client_message , MAX_BODY_SIZE , 0)) > 0 )
    {
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
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

    //Free the socket pointer
    free(socket_desc);

    mysql_close(conn);
    return 0;

}
