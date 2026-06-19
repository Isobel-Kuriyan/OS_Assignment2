#include<stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>


typedef struct CommandInfo{
    char *storecommand;
    pid_t pid;
    char *start;
    char* end;
    long long total_duration;
}CommandInfo;

CommandInfo* prev_history=NULL;
int numCommands=0;

void strip_newline(char *s) {//to remove newline character
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

void giveHistory(CommandInfo* prev_history,int count){
    for(int i=0;i<count;i++){
        printf("%s\n",prev_history[i].storecommand);
    }
}

void givefullHistory(CommandInfo* prev_history,int count){
    for(int i=0;i<count;i++){
        printf("%s\n",prev_history[i].storecommand);
        printf("start time: %s\n",prev_history[i].start);
        printf("end time: %s\n",prev_history[i].end);
        printf("total duration: %lld ms\n", prev_history[i].total_duration);
        printf("PID: %d\n", (int)prev_history[i].pid); 
        
    }
    printf("\n");
}

void signal_handler(int signal) {
    // Check if the signal is SIGINT
    if (signal == SIGINT) {
        printf("Got the CTRL+C command, exiting from the shell\n");
        givefullHistory(prev_history, numCommands);
    } 
    exit(0);// exit 0 leaves the current process("process" not function)
}

//SIGINT is for Ctrl+ C and SIGSTP is for Ctrl + Z

int main(int argc, char *argv[]){
    char* line=NULL;
    size_t len=0;
    

        
    while(1){
        signal(SIGINT,signal_handler);
        struct timespec tstart, tend;
        CommandInfo command;//creation of struct

        //stdin(standard input usually the terminal) is where to read the input from 
        getline(&line, &len, stdin);// will get the line whenever user types something and presses enter
        strip_newline(line);// to remove the newlie character "\n" from the end of the string
        

        if(strcmp(line, "history") == 0){
            giveHistory(prev_history, numCommands);
            continue;
        }

        // making an argument array 
        command.storecommand= malloc(strlen(line) + 1);// + 1 to accomodate the null terminator
        //strlen doesn't count the "\0", but it counts the"\n" 
        strcpy(command.storecommand,line);

        char ***argArr = malloc(100 * sizeof *argArr); // to store all pipe separated commands
        int pipe_commands=0;

        // cat file.c | grep main | wc -l

        int offset=0;
        int iterator=0;
        while (iterator != (int)strlen(line)) {
            char **args = calloc(50, sizeof *args);  // FIX: zero-init argv
            // here calloc is usesful because execvp needs a null terminated list
            int len = 0; //is the number of char for the current token
            int count = 0;// counts the number of arguments( tokens) in a single command
            bool flag = false;

            for (int i = iterator; i < (int)strlen(line); i++) {
                // printf("%c", line[i]);

                if (line[i] == ' ') {
                    if (len > 0) {// FIX: only flush if we have a token(one word, or meaningful piece of text extracted from a string)
                        args[count++] = strndup(line + offset, (size_t)len);
                        args[count] = NULL;
                        len = 0;
                    }
                    flag = true;
                    offset = i + 1;                                  // FIX: next token starts after space
                }
                else {
                    flag = false;
                    if (line[i] != '|') {                            // FIX: don't count the '|' into token
                        if (len == 0) offset = i;                    // FIX: mark start of token
                        len++;
                    }
                }

                if (line[i] == '|') {
                    if (!flag && len > 0) {                          // FIX: flush token before the pipe
                        args[count++] = strndup(line + offset, (size_t)len);
                        args[count] = NULL;
                        len = 0;
                    }
                    argArr[pipe_commands++] = args;
                    iterator = i + 1;                                // next segment after '|'
                    offset = iterator;                                // FIX: reset token start correctly
                    count = 0;
                    break;
                }

                /* removed: offset++ (was corrupting token starts) */
            }

            // printf("\n");

            // Flush last token at end-of-line
            if (len > 0) {                                           // FIX: capture trailing token
                args[count++] = strndup(line + offset, (size_t)len);
            }
            if (count != 0) {
                args[count] = NULL;
                argArr[pipe_commands++] = args;
                break;
            }
        }

        // printf("\n");
        // printf("came here for printing \n");
        // for (int i=0;i<pipe_commands;i++) {
        //     for (int j=0;argArr[i][j]!=NULL;j++) {
        //         printf("%s ",argArr[i][j]);
        //     }
        //     printf("\n");
        // }
        // printf("\n");
    


        char buffer[50];

        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local);
        command.start = malloc(strlen(buffer) + 1);
        strcpy(command.start, buffer);

        clock_gettime(CLOCK_MONOTONIC, &tstart);

        
        int n = pipe_commands;
        int pipes[n][2];  
        //Each Unix pipe consists of two file descriptors:

        // pipe[0] -> read end
        // pipe[1] -> write end

        

        for (int i = 0; i < n; ++i) {
            if (pipe(pipes[i]) == -1) { 
                printf("pipe error"); 
                exit(1);// exit status 1 denotes pipe error
            }
        }

        pid_t *pids = malloc(n * sizeof *pids); // stores all children pids(children refers to each individual command)
        if (!pids) { 
            printf("malloc error");
            continue; 
        }

        for (int i = 0; i < n; ++i) {
            pid_t cpid = fork();
            if (cpid < 0) { 
                printf("fork error");
                exit(1);
            }
            if (cpid == 0) {
                // Child i: hook up stdin/stdout as needed
                if (i > 0) { // not first: stdin <- previous pipe read end( read the output of the prev command)
                    if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) { 
                        printf("dup2 error"); 
                        exit(1); 
                    }
                }
                if (i < n - 1) {// not last: stdout -> current pipe write end( write the output of the current command into the pipe)
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) { 
                        printf("dup2 error"); 
                        exit(1); 
                    }
                }
                // Close all pipe fds in child
                for (int j = 0; j < n; ++j) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                // Exec this stage
                execvp(argArr[i][0], argArr[i]);
                printf("execvp error");
                exit(1); // exec failed
            }
            pids[i] = cpid;
        }

        // Parent: close all pipe fds (important for EOF)
        for (int j = 0; j < n; ++j) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }

        // Wait for all children; capture the LAST stage's status like a shell
        int status = 0, last_status = 0;
        pid_t last_pid = pids[n - 1];
        for (int i = 0; i < n; ++i) {
            pid_t w = waitpid(pids[i], &status, 0);
            if (w == -1) perror("waitpid");
            if (w == last_pid) last_status = status;
        }
        free(pids);

        // --- end time + history (kept intact) ---
        clock_gettime(CLOCK_MONOTONIC, &tend);

        now = time(NULL);
        local = localtime(&now);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local);
        command.end = malloc(strlen(buffer) + 1);
        strcpy(command.end, buffer);

        command.pid = last_pid;// store pid of last stage
        long long ms = (tend.tv_sec - tstart.tv_sec) * 1000LL + (tend.tv_nsec - tstart.tv_nsec) / 1000000LL;
        command.total_duration = ms;

        // Preserve your “skip on nonzero exit” behavior, but guard it correctly
        if (WIFEXITED(last_status) && WEXITSTATUS(last_status)) {
            continue;   // non-zero exit code from last command; skip history add
        }
        
        //WIFEXITED: 
        // Acts like a boolean:
        // Non-zero (true) → process exited normally using exit() or return
        // 0 (false) → process did not exit normally (e.g., killed by a signal)

        //WEXITSTATUS : it outputs the actual exit code of the process

        // the above two just decode the status integer

        if (WIFEXITED(last_status)) {
            prev_history = realloc(prev_history, (numCommands + 1) * sizeof(CommandInfo));
            if (!prev_history) { 
                printf("realloc error");
                continue; 
            }
            prev_history[numCommands++] = command;
        } 
        // we only store commands if they are executed successfully
   

    }
    return 0;
}