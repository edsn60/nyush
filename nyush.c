//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <termios.h>

#include "nyush.h"
#include "command_parser.h"
#include "command_check.h"
#include "pipe_.h"
#include "builtin.h"


//struct Pipe_Subcommand {
//    int subcommand_idx;
//    char *program;
//    char **args;
//    int input_fd;
//    int output_fd;
//    struct Pipe_Subcommand *next;
//    struct Pipe_Subcommand *pre;
//};
//
//
//struct Single_Command_Jobs{
//    char *program;
//    char *command;
//    char **args;
//    int input_fd;
//    int output_fd;
//    int pid;
//    int pgid;
//};
//
//
//struct SuspendedJobs{
//    int jobID;
//    pid_t Pid;
//    pid_t Pgid;
//    int isPipe;
//    int status;
//    char *command;
//    struct SuspendedJobs *next;
//    struct SuspendedJobs *pre;
//};


struct SuspendedJobs *jobs_list_head = NULL;
struct SuspendedJobs *jobs_list_tail = NULL;
char *current_job_command = NULL;
//int process_count = 0;

/**
 * The shell starts here. First ignore some signals and then check cwd, then prompt the user to input command
 *      The command will be first preprocessed to delete all leading whitespaces and check if is blank line
 * @return
 */
int main() {
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGSTOP, SIG_DFL);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    jobs_list_head = (struct SuspendedJobs*) malloc(sizeof(struct SuspendedJobs));
    jobs_list_head->jobID = 0;
    jobs_list_head->pre = NULL;
    jobs_list_head->next = NULL;
    jobs_list_tail = jobs_list_head;
    printf("The default interactive shell is now nyush.\n");
    char cwd[300] = {};
    current_job_command = (char*) malloc(sizeof(char) * 1001);

    char input[1001] = {};
    while(1){
        getcwd(cwd, sizeof(cwd));
        char *basename1 = basename(cwd);
        tcflush(STDERR_FILENO, TCOFLUSH);
        printf("[nyush %s]$ ", basename1);
        gets(input);

        char *whitespace = input;

        while(*whitespace == ' '){
            whitespace++;
        }
        if (strlen(whitespace) == 0){
            continue;
        }
        strcpy(current_job_command, input);
        int is_Valid_or_Piped = isValidCommand(input);
        if (is_Valid_or_Piped == -1){   // invalid
            continue;
        }
        else if (is_Valid_or_Piped == 1){   // piped
            pipe_parser(input);
            continue;
        }
        else if (is_Valid_or_Piped == 2){   // builtin
            if (builtin_handler(input) == 0){
                free(jobs_list_head);
                free(current_job_command);
                return 0;
            }
        }
        else{
            single_command_parser(input);
        }
////        if (single_command_parser(input) == 0){
////            free(jobs_list_head);
////            free(stp_command);
////            return 0;
////        }
    }
}