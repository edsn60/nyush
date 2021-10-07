//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <signal.h>

#include "nyush.h"

extern struct SuspendedJobs *suspended_jobs_list_head;
extern struct SuspendedJobs *suspended_jobs_list_tail;
extern char *current_job_command;


/** Handle the suspended jobs
 *
 * @param pgid:: the ID of the process group that needs to be handled
 */
void child_process_signal_handler(pid_t pgid){

    suspended_jobs_list_tail->next = (struct SuspendedJobs*) malloc(sizeof(struct SuspendedJobs));
    suspended_jobs_list_tail->next->pre = suspended_jobs_list_tail;
    suspended_jobs_list_tail->next->jobID = suspended_jobs_list_tail->jobID + 1;
    suspended_jobs_list_tail = suspended_jobs_list_tail->next;

    suspended_jobs_list_tail->Pgid = pgid;

    suspended_jobs_list_tail->status = CLD_STOPPED;
    suspended_jobs_list_tail->next = NULL;
    suspended_jobs_list_tail->command = (char*) malloc(sizeof(char) * (strlen(current_job_command) + 1));
    strcpy(suspended_jobs_list_tail->command, current_job_command);
    printf("[%d]+  Stopped    %s\n", suspended_jobs_list_tail->jobID, current_job_command);

}


/** Handle the suspended but then continued jobs
 *
 * @param infop:: the struct '''siginfo_t'''
 * @param jobid:: the id of the suspended job
 */
void continued_job_handler(siginfo_t *infop, int jobid){
    struct SuspendedJobs *suspended_job = suspended_jobs_list_head->next;
    while (suspended_job){
        if (suspended_job->jobID == jobid){ // find the job
            printf("si_code: %d, si_signo: %d, si_status: %d, si_pid: %d, pgid: %d\n", infop->si_code, infop->si_signo, infop->si_status, infop->si_pid, suspended_job->Pgid);
            if (infop->si_code == CLD_EXITED || infop->si_code == CLD_KILLED){  // if exited or killed
                kill(-(suspended_job->Pgid), SIGKILL);
                if (suspended_jobs_list_tail == suspended_job){
                    suspended_jobs_list_tail = suspended_jobs_list_tail->pre;
                }
                printf("[%d]  Exited   %s\n", suspended_job->jobID, suspended_job->command);
                suspended_job->pre->next = suspended_job->next;
                if (suspended_job->next) {
                    suspended_job->next->pre = suspended_job->pre;
                }
                free(suspended_job);
            }
            else if (infop->si_code == CLD_STOPPED){    // if stopped again
                kill(-(suspended_job->Pgid), SIGTSTP);
                printf("[%d]  Stopped   %s\n", suspended_job->jobID, suspended_job->command);
            }
            break;
        }
        suspended_job = suspended_job->next;
    }
}