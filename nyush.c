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
#include "execute_command.h"
#include "builtin.h"
#include "command_parser.h"


struct SuspendedJobs *suspended_jobs_list_head = NULL;
struct SuspendedJobs *suspended_jobs_list_tail = NULL;
struct Jobs *jobs_list_head = NULL;
struct Jobs *jobs_list_tail = NULL;
char *current_job_command = NULL;


int isPipe = 0; // pipe flag


/** Empty and free the "jobs_list" except the head node
 *
 */
static void free_jobs(){
    while (jobs_list_tail->pre){
        jobs_list_tail = jobs_list_tail->pre;
        free(jobs_list_tail->next);
        jobs_list_tail->next = NULL;
    }
}


/** The shell starts here. First ignore some signals and then check cwd, then prompt the user to input command
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

    suspended_jobs_list_head = (struct SuspendedJobs*) malloc(sizeof(struct SuspendedJobs));
    suspended_jobs_list_head->jobID = 0;
    suspended_jobs_list_head->pre = NULL;
    suspended_jobs_list_head->next = NULL;
    suspended_jobs_list_tail = suspended_jobs_list_head;

    char *cwd = (char*) malloc(sizeof(char) * 400);
    char *input = (char*) malloc(sizeof(char) * 1001);
    current_job_command = (char*) malloc(sizeof(char) * 1001);
    size_t len = sizeof(char) * 1001;

    jobs_list_head = (struct Jobs*) malloc(sizeof(struct Jobs));
    jobs_list_tail = jobs_list_head;
    jobs_list_tail->next = NULL;
    jobs_list_tail->pre = NULL;
    jobs_list_tail->cmdname = NULL;
    jobs_list_tail->args = NULL;
    while(1){

        getcwd(cwd, sizeof(char) * 400);
        char *basename1 = basename(cwd);
        printf("[nyush %s]$ ", basename1);
        fflush(stdout);
        size_t read_size = getline(&input, &len, stdin);
        if (feof(stdin)){
            printf("\n");
            exit(0);
        }
        if (input[read_size-1] == '\n'){
            input[read_size-1] = '\0';
        }
        char *whitespace = input;

        while(*whitespace == ' '){
            whitespace++;
        }
        if (strlen(whitespace) == 0){
            continue;
        }

        strcpy(current_job_command, input);
        int isValid_and_CommandType = isValidCommand(current_job_command, &isPipe);
        if (isValid_and_CommandType == 2){
            if (builtin_handler(input) == 0){
                free_jobs();
                free(suspended_jobs_list_head);
                free(current_job_command);
                return 0;
            }
        }
        else if (isValid_and_CommandType == 1){
            execute_command(jobs_list_head->next, isPipe);
        }
        free_jobs();
        isPipe = 0;
    }
}