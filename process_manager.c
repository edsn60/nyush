//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <termios.h>

#include "pipe_.h"
#include "nyush.h"

extern struct SuspendedJobs *jobs_list_head;
extern struct SuspendedJobs *jobs_list_tail;
extern char *current_job_command;
//extern int process_count;


void child_process_signal_handler(struct Pipe_job *pipejob, struct Single_Command_Jobs *singlejob){
//    if (infop->si_code == CLD_EXITED){
//        printf("[n]+  Exited    %s\n", current_job_command);
//    }
//    else if (infop->si_code == CLD_STOPPED){
    jobs_list_tail->next = (struct SuspendedJobs*) malloc(sizeof(struct SuspendedJobs));
    jobs_list_tail->next->pre = jobs_list_tail;
    jobs_list_tail->next->jobID = jobs_list_tail->jobID + 1;
    jobs_list_tail = jobs_list_tail->next;
    if (pipejob){
        jobs_list_tail->Pid = -1;
        jobs_list_tail->isPipe = 1;
        jobs_list_tail->Pgid = pipejob->PGID;
    } else{
        jobs_list_tail->Pid = singlejob->pid;
        jobs_list_tail->isPipe = 0;
        jobs_list_tail->Pgid = singlejob->pgid;
    }
    jobs_list_tail->status = CLD_STOPPED;
    jobs_list_tail->next = NULL;
    jobs_list_tail->command = (char*) malloc(sizeof(char) * (strlen(current_job_command) + 1));
    strcpy(jobs_list_tail->command, current_job_command);
    printf("[%d]+  Stopped    %s\n", jobs_list_tail->jobID, current_job_command);
//        process_count++;
//    }
}


void continued_job_handler(siginfo_t *infop, int jobid){
    struct SuspendedJobs *job = jobs_list_head->next;
    while (job){
        if (job->jobID == jobid){
            if (infop->si_code == CLD_EXITED){
                if (jobs_list_tail == job){
                    jobs_list_tail = jobs_list_tail->pre;
                }
                printf("[%d]  Exited   %s\n", job->jobID, job->command);
                job->pre->next = job->next;
                if (job->next) {
                    job->next->pre = job->pre;
                }
                free(job);
            }
            else if (infop->si_code == CLD_STOPPED){
                printf("[%d]  Stopped   %s\n", job->jobID, job->command);
            }
            break;
        }
        job = job->next;
    }
}