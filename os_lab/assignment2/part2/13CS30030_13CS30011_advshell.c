/*Implementation of basic shell commands
Name: Shubham Agrawal, Divyansh Gupta
Roll: 13CS30030, 13CS30011
*/
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>


extern char **environ;

struct dirent  *dp;
struct stat     statbuf;
struct passwd  *pwd;
struct group   *grp;
struct tm      *tm;
char            datestring[256];


#define SH_RL_BUFSIZE 1024
#define SH_TOK_BUFSIZE 64
#define MAX_HISTORY_SIZE 20
#define SH_TOK_DELIM " \t\r\n\a"

int sh_cd(char **args);
int sh_clear();
int sh_mkdir(char **args);
int sh_rmdir(char **args);
int sh_ls(char **args);
int sh_history(char **args);
int sh_exit(char **args);
int sh_help(char **args);
int sh_env();
int sh_pwd();

typedef int Pipe[2];

char *builtin_str[] = {
    "cd",
    "clear",
    "mkdir",
    "rmdir",
    "ls",
    "history",
    "help",
    "exit",
    "env",
    "pwd"
};

int (*builtin_func[]) (char **) = {
    &sh_cd,
    &sh_clear,
    &sh_mkdir,
    &sh_rmdir,
    &sh_ls,
    &sh_history,
    &sh_help,
    &sh_exit,
    &sh_env,
    &sh_pwd
};

int sh_env() {
    int i;
    for (i = 0; environ[i] != NULL; i++)
    {
        printf("%s\n", environ[i]);
    }
    return 1;
}

int sh_pwd() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 1;
}

int sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
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

int sh_clear() {
    system("clear");
    return 1;
}


int sh_rmdir(char **args) {
    int status;
    if(args[1] == NULL){
        perror("rmdir: unable to find directory");
        return 1;
    }
    status = rmdir(args[1]);
    printf("%d", status);
    return 1;
}
char const * sperm(__mode_t mode) {
    static char local_buff[16] = {0};
    int i = 0;
    // user permissions
    if ((mode & S_IRUSR) == S_IRUSR) local_buff[i] = 'r';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IWUSR) == S_IWUSR) local_buff[i] = 'w';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IXUSR) == S_IXUSR) local_buff[i] = 'x';
    else local_buff[i] = '-';
    i++;
    // group permissions
    if ((mode & S_IRGRP) == S_IRGRP) local_buff[i] = 'r';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IWGRP) == S_IWGRP) local_buff[i] = 'w';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IXGRP) == S_IXGRP) local_buff[i] = 'x';
    else local_buff[i] = '-';
    i++;
    // other permissions
    if ((mode & S_IROTH) == S_IROTH) local_buff[i] = 'r';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IWOTH) == S_IWOTH) local_buff[i] = 'w';
    else local_buff[i] = '-';
    i++;
    if ((mode & S_IXOTH) == S_IXOTH) local_buff[i] = 'x';
    else local_buff[i] = '-';
    return local_buff;
}
int sh_ls(char **args) {
    DIR *dir;
    struct dirent *dp;
    char cwd[1024];

    getcwd(cwd, sizeof(cwd));

    if((dir  = opendir(cwd)) == NULL) {
        perror("\nUnable to open directory.");
        exit(0);
    }

    if(args[1] != NULL && strcmp(args[1], "-l") == 0){
        while ((dp=readdir(dir)) != NULL) {

            if (stat(dp->d_name, &statbuf) == -1)
            continue;
            /* Print out type, permissions, and number of links. */
            printf("%10.10s", sperm (statbuf.st_mode));
            printf("%4d", statbuf.st_nlink);

            /* Print out owner's name if it is found using getpwuid(). */
            if ((pwd = getpwuid(statbuf.st_uid)) != NULL)
            printf(" %-8.8s", pwd->pw_name);
            else
            printf(" %-8d", statbuf.st_uid);


            /* Print out group name if it is found using getgrgid(). */
            if ((grp = getgrgid(statbuf.st_gid)) != NULL)
            printf(" %-8.8s", grp->gr_name);
            else
            printf(" %-8d", statbuf.st_gid);


            /* Print size of file. */
            printf(" %9jd", (intmax_t)statbuf.st_size);


            tm = localtime(&statbuf.st_mtime);


            /* Get localized date string. */
            strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);


            printf(" %s %s\n", datestring, dp->d_name);
        }
    }
    else{
        while ((dp=readdir(dir)) != NULL) {
            printf("%s\n", dp->d_name);
        }
    }

    closedir(dir);
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

int sh_help(char **args)
{
    int i;
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}


int sh_mkdir(char **args)
{
    if(args[1] == NULL){
        perror("mkdir: directory name not specified");
        return 1;
    }
    char cwd[128];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd,args[1]);
    int status = mkdir(cwd, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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

void reverse_search(int sig) {
    if(sig == SIGQUIT){
        char *line;
        printf("Enter command to reverse search:");
        line = readline("");
        int index = history_search(line, 0);
        printf("%d\n", index);
        char* guess_line;
        HIST_ENTRY* hist = history_get(index);
        guess_line = hist->line;
        printf("%s\n", guess_line);
    }

}

void sh_loop(void)
{
    char *line;
    char **args;
    int status;
    int i;
    using_history();
    stifle_history(MAX_HISTORY_SIZE);
    char cwd[1024];
    int flag = 0;
    int argc = 0;
    // rl_initialize();
    // rl_bind_keyseq("\\C-\\", reverse_search);
    signal(SIGQUIT, reverse_search);
    do {
        getcwd(cwd, sizeof(cwd));
        printf("%s", cwd);
        line = readline(">");
        args = sh_split_line(line);
        for(i = 0; args[i] != NULL; i++);
        argc = i;
        add_history(line);
        for(i = 0; args[i] != NULL; i++) {
            if(strcmp(args[i], ">") == 0 && args[i+1] != NULL){
                int save_out = dup(1);
                int file = open(args[i+1], O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                dup2(file, 1);
                status = sh_execute(args, argc);
                flag = 1;
                dup2(save_out, 1);
            }
            if(strcmp(args[i], "<") == 0 && args[i+1] != NULL){
                int save_in = dup(0);
                int file = open(args[i+1], O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                dup2(file, 0);
                status = sh_execute(args, argc);
                flag = 1;
                dup2(save_in, 0);
            }

        }

        if(flag == 0) status = sh_execute(args, argc);

        free(line);
        free(args);
    }
    while (status);
}

int sh_exit(char **args)
{
    return 0;
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

char*** split_multiple(int argc, char **argv, int *cmdc)
{
    /* Split the command line into sequences of arguments */
    /* Break at pipe symbols as arguments on their own */
    char ***cmdv;            // Way too many
    cmdv = (char***)malloc((argc/2)*sizeof(char**));
    char **args;
    args = (char**)malloc((argc+1)*sizeof(char*));
    int cmdn = 0;
    int argn = 0;
    int i, j;
    cmdv[cmdn++] = &args[argn];
    for (i = 0; i < argc - 1; i++)
    {
        char *arg = argv[i];
        if (strcmp(arg, "|") == 0)
        {
            if (i == 0)
                printf("Syntax error: pipe before any command\n");
            if (args[argn-1] == 0)
                printf("Syntax error: two pipes with no command between\n");
            arg = 0;
        }
        args[argn++] = arg;
        if (arg == 0)
            cmdv[cmdn++] = &args[argn];
    }

    if (args[argn-1] == 0)
        printf("Syntax error: pipe with no command following\n");
    args[argn] = 0;
    *cmdc = cmdn;
    return cmdv;
}

int spawn_proc (int in, int out, char** cmd)
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      return execvp (cmd[0], cmd);
    }

  return pid;
}

int fork_pipes (int n, char ***cmd)
{
  int i;
  pid_t pid;
  int in, fd [2];

  /* The first process should get its input from the original file descriptor 0.  */
  in = 0;

  /* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);

      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      spawn_proc (in, fd [1], cmd[i]);

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];
    }

  /* Last stage of the pipeline - set stdin be the read end of the previous pipe
     and output to the original file descriptor 1. */
  if (in != 0)
    dup2 (in, 0);

  /* Execute the last stage with the current process. */
    execvp(cmd[i][0], cmd[i]);
    return 1;
}

int sh_execute(char **args, int argc)
{
    int i;
    char ***cmd;
    int cmdn;
    if(argc > 2){
        // printf("%d\n", argc);
        cmd = split_multiple(argc, args, &cmdn);
        return fork_pipes(cmdn, cmd);
    }

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }


    return sh_launch(args);
}



int main(int argc, char* argv[]){
    sh_loop();
    // Perform any shutdown/cleanup.
    return EXIT_SUCCESS;
}
