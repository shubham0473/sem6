/*Implementation of basic shell commands
Name: Shubham Agrawal, Divyansh Gupta
Roll: 13CS30030, 13CS30011
*/
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <signal.h>



#define MESSAGE_SIZE 3000
typedef struct msgbuf {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;

#define SH_RL_BUFSIZE 1024
#define SH_TOK_BUFSIZE 64
#define MAX_HISTORY_SIZE 20
#define SH_TOK_DELIM " \t\r\n\a"


int sh_cd(char **args);
int sh_couple();
int sh_uncouple();
int sh_history(char **args);

int couple_flag = 0;
int client_id = -1;
int msgqid;
int save_out = -1;
key_t MESSAGEQ_KEY = 131;

void init_msqid(){
    if ((msgqid = msgget(MESSAGEQ_KEY, 0666)) < 0) {
        perror("msgget");
        exit(1);
    }
}

char *builtin_str[] = {
    "history",
    "cd"
};

int (*builtin_func[]) (char **) = {
    &sh_history,
    &sh_cd
};



int sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int sh_couple(){
    int out_saved;
    Message reg_msg, response;
    reg_msg.mtype = 1;
    strcpy(reg_msg.mtext, "Requesting client ID");
    if(msgsnd(msgqid, &reg_msg, strlen(reg_msg.mtext), 0) == -1){
        printf("error coupling\n");
    }
    if(msgrcv(msgqid, &response, 10, 3, 0) == -1){
        perror("msgrv failed\n");
    }
    if(response.mtext != NULL && response.mtype == 3){
        client_id = atoi(response.mtext);
        printf("Client ID: %d\n", client_id);
        couple_flag = 1;
    }

    return 1;
}

int redirect(char **args, char *line){
    if(strcmp(args[0], "uncouple") == 0) return sh_uncouple();
    int fd[2];
    Message *cl_msg;
    int msg_status;

    char *output;
    cl_msg = (Message*)malloc(sizeof(Message));
    output = (char*)malloc((MESSAGE_SIZE+1)*sizeof(char));

    if(pipe(fd) != 0 ) {          /* make a fd */
        exit(1);
    }
    // output = (char*)malloc((MESSAGE_SIZE+1)*sizeof(char));
    save_out = dup(1);
    dup2(fd[1], 1);
    int status = sh_execute(args);
    read(fd[0], output, MESSAGE_SIZE);
    dup2(save_out, 1);
    sprintf(cl_msg->mtext, "Terminal %d : %s\n %s",  client_id, line, output);
    cl_msg->mtype = 5;
    if(msgsnd(msgqid, cl_msg, strlen(cl_msg->mtext), 0) == -1){
        printf("Error redirect\n");
    }
    free(output);
    return 1;
}

int sh_uncouple(){
    Message dereg_msg;
    dereg_msg.mtype = 2;
    sprintf(dereg_msg.mtext, "%d", client_id);
    if(msgsnd(msgqid, &dereg_msg, strlen(dereg_msg.mtext), IPC_NOWAIT) == -1){
        printf("error decoupling\n");
    }
    client_id = -1;
    couple_flag = 0;
    return 1;
}

int sh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
}

int sh_history(char **args) {
    HIST_ENTRY **list;
    list = history_list();
    int max = 20;
    int j=0;
    int i = history_length;
    if(args[1] != NULL)
    {
        max = atoi(args[1]);
        if(i > max)
        {
            for(j = 0; j < max; j++){
                printf("%s\n", list[i-j-1]->line);
            }
        }
    }
    else{
        while(list[j] != NULL){
            printf("history %d: %s\n", j, list[j]->line);
            j++;
        }
    }

    return 1;

}




char **sh_split_line(char *line)
{
    int bufsize = SH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += SH_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


void sh_loop(void)
{
    char *line;
    char **args;
    int status;


    using_history();
    stifle_history(MAX_HISTORY_SIZE);
    char cwd[1024];
    do {
        fflush(stdout);
        getcwd(cwd, sizeof(cwd));
        printf("%s", cwd);
        line = readline(">");
        args = sh_split_line(line);
        add_history(line);
        if(couple_flag == 0){
            status = sh_execute(args);
        }
        else{
            status = redirect(args, line);

        }
        // fprintf(stderr, "%s\n", );
        free(line);
        free(args);
    } while (status);
}


int sh_launch(char **args)
{
    pid_t pid;
    int status;
    int i = 0;
    int background = 0;
    for(i = 0; args[i]!= NULL; i++){
        if(strcmp(args[i], "&") == 0) background = 1;
    }
    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("");
    } else {
        // Parent process
        do {
            if(background == 1)
            {
                printf("process running at pid = %d\n", pid);
                return 1;
            }
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int sh_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    if(strcmp(args[0], "couple") == 0) return sh_couple();
    if(strcmp(args[0], "uncouple") == 0) return sh_uncouple();

    for (i = 0; i < sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

void sh_receive(){
    Message response;
    while(1){
        if(msgrcv(msgqid, &response, MESSAGE_SIZE, 6, 0) == -1){
            perror("msgrv failed\n");
        }
        if(response.mtext != NULL && response.mtype == 6){
            printf("%s\n", response.mtext);
        }
    }
}


/**
@brief Split a line into tokens (very naively).
@param line The line.
@return Null-terminated array of tokens.
*/
void uncouple_exit(int sig){
    if(sig == SIGINT){
        Message dereg_msg;
        couple_flag = 0;
        dereg_msg.mtype = 2;
        sprintf(dereg_msg.mtext, "%d", client_id);
        if(msgsnd(msgqid, &dereg_msg, strlen(dereg_msg.mtext), 0) == -1){
            printf("error decoupling\n");
        }
        client_id = -1;
    }
}


int main(int argc, char* argv[]){
    int parent_pid;
    signal(SIGINT, uncouple_exit);
    init_msqid();

    if((parent_pid = fork())==0){
        sh_loop();


    }
    else{
        sh_receive();

    }
    // Perform any shutdown/cleanup.
    return EXIT_SUCCESS;
}
