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


void execute_command(struct Jobs *jobs, int isPipe){
    int pipe_fd[isPipe * 2];
    for (int i = 0; i < isPipe * 2; i = i + 2){
        pipe(pipe_fd+i);
    }

    siginfo_t infop;
    pid_t pgid = -1;
    int idx = 0;
    struct Jobs *job = jobs;
    while(job){
        int input_fd;
        int output_fd;
        if (!isPipe){
            input_fd = job->input_fd;
            output_fd = job->output_fd;
        }
        else{
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
            if (pgid == -1){
                pgid = getpid();
            }
            setpgid(getpid(), pgid);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGSTOP, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            if (input_fd != STDIN_FILENO) {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            if (output_fd != STDOUT_FILENO) {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            for (int i = 0; i < isPipe * 2; i++){
                close(pipe_fd[i]);
            }
            execv(job->cmdname, job->args);
            exit(-1);
        }
        else{
            if (pgid == -1){
                pgid = child;
                setpgid(child, pgid);
                tcsetpgrp(STDOUT_FILENO, pgid);
                tcsetpgrp(STDIN_FILENO, pgid);
            }
            setpgid(child, pgid);

        }
        job = job->next;
    }
    for (int i = 0; i < isPipe * 2; i++){
        close(pipe_fd[i]);
    }
    for (int i = 0; i < isPipe+1; i++){
        waitid(P_PGID, pgid, &infop, WEXITED | WSTOPPED);

        if (infop.si_code == CLD_EXITED){
            if (infop.si_status != 0 && infop.si_status != 2){
                kill(pgid, SIGKILL);
                tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
                tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
                break;
            }
        }
        else if (infop.si_code == CLD_STOPPED){
            kill(pgid, SIGTSTP);
            tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
            tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
            child_process_signal_handler(pgid);
            break;
        }
    }
    tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
    tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
}