//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include<sys/wait.h>

#include "builtin.h"
#include "process_manager.h"
#include "nyush.h"

extern struct SuspendedJobs *suspended_jobs_list_head;


/** builtin command, to continue a suspended job by sending SIGCONT to its process group
 *
 * @param saved_str:: Storing the remaining command, used in '''strtok_r()'''
 */
static void fg_(char **saved_str){
    int idx = 0;
    char *str_idx = strtok_r(NULL, " ", saved_str);
    for (char *c = str_idx; *c && *c != '\0'; c++){
        if (isdigit(*c) == 0){
            fprintf(stderr, "Error: invalid job\n");
            return;
        }
        idx = idx * 10 + ((int)*c) - 48;
    }
    if (idx == 0){
        fprintf(stderr, "Error: invalid job\n");
        return;
    }
    struct SuspendedJobs *suspended_job = suspended_jobs_list_head->next;
    while (suspended_job){
        if (suspended_job->jobID == idx){   // find the job
            break;
        }
        suspended_job = suspended_job->next;
    }
    if (!suspended_job){
        fprintf(stderr, "Error: invalid job\n");
        return;
    }
    tcsetpgrp(STDOUT_FILENO, suspended_job->Pgid);
    tcsetpgrp(STDIN_FILENO, suspended_job->Pgid);
    kill(-(suspended_job->Pgid), SIGCONT);  // continue the job
    siginfo_t *infop = (siginfo_t*) malloc(sizeof(siginfo_t));
    int exit_status = 0;
    while (waitid(P_PGID, suspended_job->Pgid, infop, WEXITED | WSTOPPED) != -1){
        if (infop->si_code == CLD_STOPPED){    // stopped again
            kill(-(suspended_job->Pgid), SIGTSTP);
            exit_status = 1;
            break;
        }
        else if (infop->si_code == CLD_KILLED || infop->si_code == CLD_DUMPED){    // killed or terminated abnormally
            kill(-(suspended_job->Pgid), SIGKILL);
            break;
        }
    }
    if (exit_status == 0){
        continued_job_handler(suspended_job->jobID);    // if killed or terminated abnormally, delete the job
    }
    tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
    tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
}


/** Check if there's' any currently suspended suspended jobs before exit
 *
 * @return:: -1, if there's any currently suspended suspended jobs
 *            0, if no suspended single_command_jobs
 */
static int exit_(){
    if (suspended_jobs_list_head->next){
        fprintf(stderr, "Error: there are suspended jobs\n");
        return -1;
    }
    return 0;   // exit successfully
}


/** Print all the suspended jobs
 *
 */
static void jobs_(){
    struct SuspendedJobs *job = suspended_jobs_list_head->next;
    while(job){
        printf("[%d] %s\n", job->jobID, job->command);
        job = job->next;
    }
}

/** Implementation of builtin command '''cd''', call '''chdir()''' to see if it is a valid directory
 *
 * @param saved_str:: Storing the remaining command, used in '''strtok_r()'''
 */
static void cd_(char **saved_str){
    char *dest = strtok_r(NULL, " ", saved_str);
    if (chdir(dest) == -1){
        fprintf(stderr, "Error: invalid directory\n");
        return;
    }
}

/**
 * Function used to handle builtin commands, including '''cd, exit, fg, single_command_jobs'''
 *
 * @param program:: The name of the cmdname
 * @param saved_str:: Storing the remaining command, used in '''strtok_r()'''
 * @return::  1, executed builtin command, not matter a success or a failure
 *            0, if exit successfully
 *           -1, if exit failed.
 */
int builtin_handler(char *command){
    char *saved_str = NULL;
    char *program = strtok_r(command, " ", &saved_str);
    if (strcmp(program, "cd") == 0){
        cd_(&saved_str);
    }
    else if (strcmp(program, "jobs") == 0){
        jobs_();
    }
    else if (strcmp(program, "exit") == 0){
        if (exit_() == 0){
            return 0;   // exit successfully
        }
        return -1;  // exit failed
    }
    else if (strcmp(program, "fg") == 0){
        fg_(&saved_str);
    }
    return 1;
}