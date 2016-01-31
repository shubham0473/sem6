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



#define MESSAGE_SIZE 3000

#define SH_RL_BUFSIZE 1024
#define SH_TOK_BUFSIZE 64
#define MAX_HISTORY_SIZE 20
#define SH_TOK_DELIM " \t\r\n\a"

typedef struct msgbuf {
    long mtype;
    char mtext[MESSAGE_SIZE+1];
} Message;

int sh_cd(char **args);
int sh_couple();
int sh_uncouple();
int sh_history(char **args);

int couple_flag = 0;
int msg_type = -1;     //apparently the client ID will be the msg_type
int msgqid;
key_t MESSAGEQ_KEY = 131;

void init_msqid(){
    if ((msgqid = msgget(MESSAGEQ_KEY, 0666)) < 0) {
        perror("msgget");
        exit(1);
    }
}

char *builtin_str[] = {
    "couple",
    "history",
    "cd",
    "uncouple"
};

int (*builtin_func[]) (char **) = {
    &sh_couple,
    &sh_history,
    &sh_cd,
    &sh_uncouple
};



int sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int sh_couple(int in, int out, int *save_out){
    Message reg_msg, response;
    reg_msg.mtype = 101;
    strcpy(reg_msg.mtext, "Requesting client ID");
    if(msgsnd(msgqid, &reg_msg, strlen(reg_msg.mtext), 0) == -1){
        printf("error coupling\n");
    }
    sleep(1);
    if(msgrcv(msgqid, &response, 10, 100, 0) == -1){
        perror("msgrv failed\n");
    }
    msg_type = atoi(response.mtext);
    printf("%d\n", msg_type);
    couple_flag = 1;

    save_out = dup(1);
    dup2(out, 1);
    return 1;
}

int sh_uncouple(int *save_out){
    Message dereg_msg;
    couple_flag = 0;
    dereg_msg.mtype = msg_type;
    strcpy(dereg_msg.mtext, "Requesting client deregistration");
    if(msgsnd(msgqid, &dereg_msg, strlen(dereg_msg.mtext), 0) == -1){
        printf("error decoupling\n");
    }
    msg_type = -1;
    dup2(*save_out, 1);
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
    init_msqid();
    char *output;
    int save_out;
    int fd[2];
    int msg_status;
    Message cl_msg;

    cl_msg.mtype = 0;

    if(pipe(fd) != 0 ) {          /* make a fd */
        exit(1);
    }

    output = (char*)malloc((MESSAGE_SIZE+1)*sizeof(char));

    using_history();
    stifle_history(MAX_HISTORY_SIZE);
    char cwd[1024];
    do {
        getcwd(cwd, sizeof(cwd));
        printf("%s", cwd);
        line = readline(">");
        args = sh_split_line(line);
        add_history(line);
        status = sh_execute(args, fd[0], fd[1], save_out);

        if(couple_flag == 1){
            read(fd[0], cl_msg.mtext, MESSAGE_SIZE);
            cl_msg.mtype = msg_type;
            msg_status = msgsnd(msgqid, &cl_msg, strlen(cl_msg.mtext), 0);
            printf("%s\n", cl_msg.mtext);
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

int sh_execute(char **args, int in, int out, int *save_out)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    if(strcmp(args[0], "couple") == 0) return sh_couple(in, out, save_out);
    if(strcmp(args[0], "uncouple") == 0) return sh_uncouple(save_out);

    for (i = 0; i < sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

void sh_receive(){
    Message response;
    if(msgrcv(msgqid, &response, 10, 100, 0) == -1){
        perror("msgrv failed\n");
    }
    if(response.mtext != NULL){
        printf("Terminal %d:", msg_type);
        printf("%s\n", response.mtext);
    }
}


/**
@brief Split a line into tokens (very naively).
@param line The line.
@return Null-terminated array of tokens.
*/


int main(int argc, char* argv[]){
    int parent_pid;
    if((parent_pid = fork())==0){
        sh_receive();
    }
    else{
        sh_loop();
    }
    // Perform any shutdown/cleanup.
    return EXIT_SUCCESS;
}
