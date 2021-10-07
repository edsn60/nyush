//
// Created by Ysw on 2021/10/3.
//
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "process_manager.h"
#include "nyush.h"

extern struct SuspendedJobs *suspended_jobs_list_tail;

// CLD_STOPPED == 5
//CLD_EXITED == 1
//CLD_KILLED == 2


/** To execute the job command, no matter it is piped or not
 *
 * @param jobs:: the head node of a double linked list storing the whole command,
 *                  in which a node is a subcommand except the head
 * @param isPipe:: the flag representing if the current job command is a piped command, if 0 then not,
 *                  otherwise the number of the pipe
 */
void execute_command(struct Jobs *jobs, int isPipe){
    int pipe_fd[isPipe * 2];
    for (int i = 0; i < isPipe * 2; i = i + 2){
        pipe(pipe_fd+i);
    }

    siginfo_t infop;
    pid_t pgid = -1;    // process group ID, initialized as -1
    int idx = 0;
    struct Jobs *job = jobs;
    while(job){
        int input_fd;
        int output_fd;
        if (!isPipe){   // if not piped
            input_fd = job->input_fd;
            output_fd = job->output_fd;
        }
        else{   // if piped
            if (idx == 0){
                input_fd = job->input_fd;
                output_fd = pipe_fd[idx*2 + 1];
            }
            else if (!job->next){
                input_fd = pipe_fd[idx * 2 - 2];
                output_fd = job->output_fd;
            }
            else{
                input_fd = pipe_fd[idx * 2 - 2];
                output_fd = pipe_fd[idx * 2 + 1];
            }
        }

        idx++;
        pid_t child = fork();
        if (child == -1){
            fprintf(stderr, "Error: fork failed\n");
            return;
        }
        if (child == 0){
            if (pgid == -1){    // especially used for the piped command to store the process group ID
                pgid = getpid();
            }
            setpgid(getpid(), pgid);    // set all child processes to the same process group
            signal(SIGTSTP, SIG_DFL);   // restore the signal
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGSTOP, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            if (input_fd != STDIN_FILENO) {     // input redirection
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            if (output_fd != STDOUT_FILENO) {      // output redirection
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            for (int i = 0; i < isPipe * 2; i++){   // close pipe
                close(pipe_fd[i]);
            }
            execv(job->cmdname, job->args);
            exit(-1);
        }
        else{
            if (pgid == -1){    // especially used for the piped command to store the process group ID
                pgid = child;
                setpgid(child, pgid);
                tcsetpgrp(STDOUT_FILENO, pgid);     // switch foreground
                tcsetpgrp(STDIN_FILENO, pgid);
            }
            setpgid(child, pgid); // set all child processes to the same process group

        }
        job = job->next;
    }
    for (int i = 0; i < isPipe * 2; i++){
        close(pipe_fd[i]);
    }
    for (int i = 0; i < isPipe+1; i++){
        waitid(P_PGID, pgid, &infop, WEXITED | WSTOPPED);   // wait for any of the child process in a process group

        if (infop.si_code == CLD_EXITED){
            if (infop.si_status != 0 && infop.si_status != 2){  // if exited abnormally, send SIGKILL to the group
                kill(pgid, SIGKILL);
                tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
                tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
                break;
            }
        }
        else if (infop.si_code == CLD_STOPPED){    // if stopped, send SIGTSTP to the group and handle suspended job
            kill(pgid, SIGTSTP);
            tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
            tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
            child_process_signal_handler(pgid);
            break;
        }
    }
    // if exited normally, do nothing, just switch the foreground
    tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
    tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
}