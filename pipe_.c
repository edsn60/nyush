//
// Created by Ysw on 2021/10/3.
//
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "external_program.h"
#include "IO_redirection.h"
#include "command_parser.h"
#include "command_check.h"
#include "process_manager.h"

extern struct SuspendedJobs *jobs_list_tail;

// CLD_STOPPED == 5
//CLD_EXITED == 1
//CLD_KILLED == 2


void execute_(struct Pipe_job *job, struct Pipe_Subcommand *subcommands){
    int pipe_fd[(job->subcommand_count - 1) * 2];
    for (int i = 0; i < (job->subcommand_count - 1) * 2; i = i + 2){
        pipe(pipe_fd+i);
    }
    siginfo_t infop;
    int pgid = -1;
    int idx = 0;
    struct Pipe_Subcommand *subcommand = subcommands;
    while(subcommand){
        int input_fd;
        int output_fd;
        if (idx == 0){
            input_fd = subcommand->input_fd;
            output_fd = pipe_fd[idx*2 + 1];
        }
        else if (!subcommand->next){
            input_fd = pipe_fd[idx * 2 - 2];
            output_fd = subcommand->output_fd;
        }
        else{
            input_fd = pipe_fd[idx * 2 - 2];
            output_fd = pipe_fd[idx * 2 + 1];
        }
        idx++;
        int child = fork();
        if (child == -1){
            fprintf(stderr, "Error: fork failed\n");
            return;
        }
        if (child == 0){
            if (pgid == -1){
                pgid = getpgid(getpid());
            }
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
            for (int i = 0; i < (job->subcommand_count - 1) * 2; i++){
                close(pipe_fd[i]);
            }
            setpgid(getpid(), pgid);
            execv(subcommand->program, subcommand->args);
            exit(-1);
        }
        else{
            if (pgid == -1){
                pgid = getpgid(child);
                setpgid(child, pgid);
                tcsetpgrp(STDOUT_FILENO, pgid);
                tcsetpgrp(STDIN_FILENO, pgid);
            }
            setpgid(child, pgid);

        }
        subcommand = subcommand->next;
    }
    for (int i = 0; i < (job->subcommand_count - 1) * 2; i++){
        close(pipe_fd[i]);
    }
    for (int i = 0; i < job->subcommand_count; i++){
        waitid(P_PGID, pgid, &infop, WEXITED | WSTOPPED);
        tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
        tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
        if (infop.si_code == CLD_EXITED){
            if (infop.si_status != 0 && infop.si_status != 2){
                kill(pgid, SIGKILL);
            }
        }
        else if (infop.si_code == CLD_STOPPED){
            kill(pgid, SIGTSTP);
            job->PGID = pgid;
            child_process_signal_handler(job, NULL);
            break;
        }
    }
}



void pipe_parser(char *input){
    struct Pipe_Subcommand *new_pipe_head = (struct Pipe_Subcommand*) malloc(sizeof(struct Pipe_Subcommand));
    struct Pipe_Subcommand *pipe_tail = new_pipe_head;
    new_pipe_head->next = NULL;
    new_pipe_head->pre = NULL;
    struct Pipe_job *new_pipe_job = (struct Pipe_job*) malloc(sizeof(struct Pipe_job));
    strcpy(new_pipe_job->job_command, input);

    int idx = 1;

    int flag = 1;

    int redirected_input_fd = 0;
    int redirected_output_fd = 1;

    char *saved_input = NULL;
    char *subcommand = strtok_r(input, "|", &saved_input);

    while (subcommand){
        char * savedcommand = (char*) malloc(sizeof(char)*(strlen(subcommand)+1));
        strcpy(savedcommand, subcommand);
        char *saved_subcommand = NULL;
        char *program = strtok_r(subcommand, " ", &saved_subcommand);
        char *program1 = isValidOtherProgram(program);
        if (program1){
            program = program1;
        }
        char **args = get_args(program, &saved_subcommand);
        if (idx == 1){
            redirected_input_fd = pipe_first_input_redirection(&saved_subcommand);
            if (redirected_input_fd == -1){
                fprintf(stderr, "Error: invalid file\n");
                return;
            }
        }
        else if (!saved_input && flag == 1){
            redirected_input_fd = STDIN_FILENO;
            redirected_output_fd = pipe_last_output_redirection(&saved_subcommand);
            if (redirected_output_fd == -1){
                fprintf(stderr, "Error: invalid file\n");
                return;
            }
        }
        else if (!saved_input){
            redirected_output_fd = pipe_last_output_redirection(&saved_subcommand);
            if (redirected_output_fd == -1){
                fprintf(stderr, "Error: invalid file\n");
                return;
            }
        }
        else if (flag == 1){
            redirected_input_fd = STDIN_FILENO;
            flag--;
        }

        struct Pipe_Subcommand *new_subcommand = (struct Pipe_Subcommand*) malloc(sizeof(struct Pipe_Subcommand));
        new_subcommand->next = NULL;
        new_subcommand->pre = pipe_tail;
        pipe_tail->next = new_subcommand;
        new_subcommand->program = program;
        new_subcommand->args = args;
        new_subcommand->input_fd = redirected_input_fd;
        new_subcommand->output_fd = redirected_output_fd;
        new_subcommand->subcommand_idx = idx;
        pipe_tail = pipe_tail->next;
        idx++;
        subcommand = strtok_r(NULL, "|", &saved_input);
    }
    new_pipe_job->subcommand_count = idx-1;
    new_pipe_job->subcommand_head = new_pipe_head -> next;
    execute_(new_pipe_job, new_pipe_head->next);
}