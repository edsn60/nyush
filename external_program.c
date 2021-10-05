//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "IO_redirection.h"
#include "process_manager.h"
#include "command_parser.h"
#include "structs.h"

extern char *current_job_command;


void call_execv_(struct Single_Command_Jobs *singlecommandjobs, siginfo_t *infop, int child){
    child = fork();
    if (child == -1){
        fprintf(stderr, "Error: fork failed\n");
        return;
    }
    if (child == 0){
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        dup2(singlecommandjobs->input_fd, STDIN_FILENO);
        dup2(singlecommandjobs->output_fd, STDOUT_FILENO);
        setpgid(getpid(), getpid());
        execv(singlecommandjobs->program, singlecommandjobs->args);
        exit(-1);
    }
    else {
        setpgid(child, child);
        tcsetpgrp(STDOUT_FILENO, child);
        tcsetpgrp(STDIN_FILENO, child);
        waitid(P_PGID, child, infop, WEXITED | WSTOPPED);
        tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
        tcsetpgrp(STDOUT_FILENO, getpgid(getpid()));
    }
}


void execute_other_program(char *program, char **saved_str){
    char **args = get_args(program, saved_str);
    int redirected_input_fd = -1;
    int redirected_output_fd = -1;

    while (*saved_str && (**saved_str == '<' || **saved_str == '>')) {
        if (**saved_str == '<') {
            char *input_redirection_operator = strtok_r(NULL, " ", saved_str);
            char *filename = strtok_r(NULL, " ", saved_str);
            if (!filename || strcmp(input_redirection_operator, "<") != 0) {
                fprintf(stderr, "Error: invalid file\n");
                return;
            }
            redirected_input_fd = input_redirection(filename);
            if (redirected_input_fd == -1) {
                return;
            }

//            isInputRedirected = 1;
//            input_newfd = dup2(redirected_input_fd, STDIN_FILENO);
//            close(redirected_input_fd);
        } else if (**saved_str == '>') {
            char *output_redirection_operator = strtok_r(NULL, " ", saved_str);
            char *filename = strtok_r(NULL, " ", saved_str);
            int mode;
            if (!filename || strcmp(output_redirection_operator, ">") != 0) {
                if (strcmp(output_redirection_operator, ">>") != 0) {
                    fprintf(stderr, "Error: invalid file\n");
                    return;
                } else {
                    mode = 1;
                }
            } else {
                mode = 0;
            }

            redirected_output_fd = output_redirection(filename, mode);
//            isOutputRedirected = 1;
//            output_newfd = dup2(redirected_output_fd, STDOUT_FILENO);
//            close(redirected_output_fd);
        }
    }
    struct Single_Command_Jobs * new_single_command_job = (struct Single_Command_Jobs *) malloc(sizeof(struct Single_Command_Jobs));
    new_single_command_job->program = program;
    new_single_command_job->command = current_job_command;
    new_single_command_job->args = args;
    new_single_command_job->input_fd = redirected_input_fd;
    new_single_command_job->output_fd = redirected_output_fd;

    int child = 0;
    siginfo_t *infop = (siginfo_t*) malloc(sizeof(siginfo_t));

    call_execv_(new_single_command_job, infop, child);
    new_single_command_job->pid = child;
    new_single_command_job->pgid = child;
    if (infop->si_code == CLD_STOPPED){
        child_process_signal_handler(NULL, new_single_command_job);
    }
    else if (infop->si_code == CLD_EXITED){
        printf("[n]+  Exited    %s\n", current_job_command);
    }



    free(new_single_command_job);
    free(infop);
}


/**
 * Find the location of the program based on relative path, absolute path or cwd
 *
 * @param program:: The name of the program
 * @return:: NULL, if the program cannot be located
 *           the path of the program, if the program can be located, it can be absolute path or relative path
 */
char *locate_other_program(char *program){
    if (*program == '.'){   // check if is under cwd
        char *tmp = program;
        tmp++;
        if(*tmp == '/'){
            tmp++;
            if (access(tmp, 1) != 0){   // not found under cwd
                return NULL;
            }
            else{
                return program;
            }
        }
    }
    else if (*program == '/') {    // check if is an abs_path
        if (access(program, 1) != 0){
            return NULL;
        }
        else{   // is an abs_path
            return program;
        }
    }
    else{   // neither, then check /bin and /usr/bin
        char *bin = (char*) malloc(sizeof(char) * (5 + strlen(program) + 1));
        strcpy(bin, "/bin/");
        strcat(bin, program);
        if (access(bin, 1) != 0){
            char *usrbin = (char*) malloc(sizeof(char) * (9 + strlen(program) + 1));
            strcpy(usrbin, "/usr/bin/");
            if (access(strcat(usrbin, program), 1) != 0){   // not found under /bin and /usr/bin
                return NULL;
            }
            else{   // under /usr/bin
                return usrbin;
            }
        }
        else{   // under /bin
            return bin;
        }
    }
    return NULL;
}


