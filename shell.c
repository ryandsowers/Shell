// -------------------------------------------------------------
// File: shell.c
//
// Description: This program creates a command-line shell that 
//      resembles a typical command-line shell. It writes 
//      commands to a history file.
//
// Syntax: The program can be run by calling "./shell" on the 
//      command-line. The user will then see a "prompt> " after
//      commands can be given and executed as one would do with
//      a typical command-line.
//      
// -------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define COMMAND_LEN 80
#define MAX_ARGS 10
#define HISTORY_FILE "shell-history"
#define BUFSIZE 128
#define TRUE 1
#define TIMEOUT 30
#define ERROR -1

FILE *fd = NULL;

void alarm_handler(int signal);

int main(int argc, char const *argv[])
{
        int pid;
        int status;
        int stringLength;
        char *token = NULL;
        char str[COMMAND_LEN];
        char *array[MAX_ARGS+1];
        struct sigaction action;
        char exitString[] = "exit";
        char explodeString[] = "explode";
        

        errno = 0;
        fd = fopen(HISTORY_FILE, "a");  // open history file to be appended to
        if ((fd == NULL) || (errno)) {
                perror("ERROR. Tried to open history file in 'append' mode");
                exit(ERROR);
        }

        // set up signal handler
        action.sa_handler = alarm_handler;
        action.sa_flags = SA_RESTART;
        sigemptyset(&action.sa_mask);
        sigaction(SIGSEGV, &action, NULL);
        sigaction(SIGINT, &action, NULL);
        sigaction(SIGALRM, &action, NULL);
        
        // begin prompt loop
        while (TRUE) {
                printf("prompt> ");
                alarm(TIMEOUT);  // alarm condition
                fgets(str, sizeof(str), stdin);  // get user command
                fwrite(str, sizeof(char), strlen(str), fd);  // write to history file

                stringLength = strlen(str) - 1;
                str[stringLength] = '\0';
                token = strtok(str, " ");  // tokenize input
                if (token == NULL) {
                        continue;
                } else if (!strcmp(token, exitString)) {
                        break;
                } else if (!strcmp(token, explodeString)) {
                        int *bomb = NULL;
                        *bomb = 42;
                }

                // build token/command list
                int i = 0;
                while (token != NULL) {
                        array[i++] = token;
                        token = strtok(NULL, " ");
                }
                array[i] = NULL;

                errno = 0;
                fflush(fd);
                if (errno) {
                        perror("Error, flush failed");
                }

                // fork child process
                errno = 0;
                pid = fork();
                if (pid < 0) {
                        perror("Error, fork failed");
                } else if (pid == 0) {
                        // printf("child process running\n"); 
                        errno = 0;
                        execvp(str, array);  // exec commands
                        perror("ERROR");
                        if (fd != NULL) {
                                fclose(fd);
                                fd = NULL;
                        }
                        exit(ERROR);
                } else {
                        // printf("parent process running\n"); 
                        waitpid(pid, &status, 0);  // block until child terminates
                }
        }

        if (fd != NULL) {
                fclose(fd);
                fd = NULL;
        }


        return 0;
}

void alarm_handler(int signal)
{
        if (signal == SIGSEGV) {
                errno = 0;
                fprintf(stderr, "A segmentation fault has been detected.\nExiting...\n");
                fflush(fd);
                if (errno) {
                        perror("Error, flush failed");
                }
                if (fd != NULL) {
                        fclose(fd);
                        fd = NULL;
                }
                exit(-1);
        } else if (signal == SIGINT) {
                errno = 0;
                fprintf(stdout, "\nThe Interrupt signal has been caught.\nExiting...\n");
                fflush(fd);
                if (errno) {
                        perror("Error, flush failed");
                }
                if (fd != NULL) {
                        fclose(fd);
                        fd = NULL;
                }                
                exit(-2);
        } else if (signal == SIGALRM) {
                errno = 0;
                fprintf(stdout, "\nThe session has expired.\nExiting...\n");
                fflush(fd);
                if (errno) {
                        perror("Error, flush failed");
                }
                if (fd != NULL) {
                        fclose(fd);
                        fd = NULL;
                }                
                exit(-3);
        } else {
                fprintf(stderr, "Did not find your signal. %d\n", signal);
        }
}



















