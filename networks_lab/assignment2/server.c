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
#define PORT 8888



int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "Welcome to The Ticket Counter \r\n";

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

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
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

    if((parent_pid = fork()) == 0){
        MYSQL *conn;
        MYSQL_RES *res;
        MYSQL_ROW row;
        conn = mysql_init(NULL);
        char query[1000];
        int opt = 0;
        if (!mysql_real_connect(conn, server,
            user, password, database, 0, NULL, 0)) {
                fprintf(stderr, "%s\n", mysql_error(conn));
                return 0;
            }
        printf("Press 1 to display the current booking status\n");
        while(1){
            scanf("%d", &opt);
            if(opt == 1){
                strcpy(query, "select train_name, coach_type, count(flag=1) as available, count(*) as total from tb_seat group by coach_type, train_name;");
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("Train Name \t coach_type \t available \t total\n");
                while ((row = mysql_fetch_row(res)) != NULL){
                    printf("%s \t %s \t\t %s \t\t %s\n", row[0], row[1], row[2], row[3]);
                }
            }
        }

    }
    else{
        while(TRUE)
        {

            //clear the socket set
            FD_ZERO(&readfds);

            //add master socket to set
            FD_SET(master_socket, &readfds);
            max_sd = master_socket;

            //add child sockets to set
            for ( i = 0 ; i < max_clients ; i++)
            {
                //socket descriptor
                sd = client_socket[i];

                //if valid socket descriptor then add to read list
                if(sd > 0)
                FD_SET( sd , &readfds);

                //highest file descriptor number, need it for the select function
                if(sd > max_sd)
                max_sd = sd;
            }

            //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
            activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

            if ((activity < 0) && (errno!=EINTR))
            {
                printf("select error");
            }

            //If something happened on the master socket , then its an incoming connection
            if (FD_ISSET(master_socket, &readfds))
            {
                if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                //inform user of socket number - used in send and receive commands
                printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                //send new connection greeting message
                if( send(new_socket, message, strlen(message), 0) != strlen(message) )
                {
                    perror("send");
                }

                puts("Welcome message sent successfully");

                //add new socket to array of sockets
                for (i = 0; i < max_clients; i++)
                {
                    //if position is empty
                    if( client_socket[i] == 0 )
                    {
                        client_socket[i] = new_socket;
                        printf("Adding to list of sockets as %d\n" , i);

                        break;
                    }
                }
            }

            //else its some IO operation on some other socket :)
            for (i = 0; i < max_clients; i++)
            {
                sd = client_socket[i];

                if (FD_ISSET( sd , &readfds))
                {
                    //Check if it was for closing , and also read the incoming message
                    if ((valread = recv(sd , buffer, 1024, 0)) == 0)
                    {
                        //Somebody disconnected , get his details and print
                        getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                        printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                        //Close the socket and mark as 0 in list for reuse
                        close( sd );
                        client_socket[i] = 0;
                    }

                    //Echo back the message that came in
                    else
                    {
                        //do the sql operations
                        buffer[valread] = '\0';
                        send(sd , buffer , strlen(buffer) , 0 );
                    }
                }
            }
        }
    }



    return 0;
}
