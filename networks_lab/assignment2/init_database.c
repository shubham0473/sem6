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

char *server = "10.5.18.68";
char *user = "13CS30030";
char *password = "cse12";
char *database = "13CS30030";
char *table = "tb_seat";

int main(){
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    conn = mysql_init(NULL);
    char query[1000];
    int c = 0;
    char a[13][4] = {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13"};


    /* Connect to database */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 0;
    }

    printf("start\n");

    for(int j = 0; j < 13; j++){
        for(int i = 1; i <= 54; i++){
            if(c == 0){
                sprintf(query, "INSERT INTO tb_seat (train_no, coach_no, seat_no, seat_type, flag, train_name, coach_type) VALUES (12301, '%s', %d, 'LB', 1, 'Rajdhani Exp', 'AC')", a[j], i);
                printf("%s\n", query);
                c = 1;
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("DONE\n");
                continue;
            }
            else if(c == 1){
                sprintf(query, "INSERT INTO tb_seat (train_no, coach_no, seat_no, seat_type, flag, train_name, coach_type) VALUES (12301, '%s', %d, 'UB', 1, 'Rajdhani Exp', 'AC')", a[j], i);
                c = 2;
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("DONE\n");
                continue;
            }
            else if(c == 2){
                sprintf(query, "INSERT INTO tb_seat (train_no,coach_no, seat_no, seat_type, flag, train_name, coach_type) VALUES (12301, '%s', %d, 'LB', 1, 'Rajdhani Exp', 'AC')", a[j], i);
                c = 3;
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("DONE\n");
                continue;
            }
            else if(c == 3){
                sprintf(query, "INSERT INTO tb_seat (train_no, coach_no, seat_no, seat_type, flag, train_name, coach_type) VALUES (12301, '%s', %d, 'UB', 1, 'Rajdhani Exp', 'AC')", a[j], i);
                c = 4;
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("DONE\n");
                continue;
            }
            else if(c == 4){
                sprintf(query, "INSERT INTO tb_seat (train_no, coach_no, seat_no, seat_type, flag, train_name, coach_type) VALUES (12301, '%s', %d, 'SL', 1, 'Rajdhani Exp', 'AC')", a[j], i);
                c = 5;
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("DONE\n");
                continue;
            }
            else if(c == 5){
                sprintf(query, "INSERT INTO tb_seat (train_no, coach_no, seat_no, seat_type, flag, train_name, coach_type) VALUES (12301, '%s', %d, 'SU', 1, 'Rajdhani Exp', 'AC')", a[j], i);
                c = 0;
                if (mysql_query(conn, query)) {
                    fprintf(stderr, "%s\n", mysql_error(conn));
                    exit(1);
                }
                res = mysql_use_result(conn);
                printf("DONE\n");
                continue;
            }


        }
    }
    // mysql_free_result(res);
    mysql_close(conn);
    return 0;
}
