//
// Created by Ysw on 2021/10/5.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/fcntl.h>
#include "nyush.h"

extern struct Jobs *jobs_list_head;
extern struct Jobs *jobs_list_tail;


extern int isPipe;


int isBuiltin(char *program){
    if (strcmp(program, "cd") == 0 || strcmp(program, "single_command_jobs") == 0 || strcmp(program, "exit") == 0 || strcmp(program, "fg") == 0){
        return 1;
    }
    return 0;
}


int isValidAbsPath(char *program){
    if (*program == '/'){
        if (access(program, X_OK) == 0){
            return 1;
        }
        else{
            return -1;
        }
    }
    else{
        return 0;
    }
}

int isValidRelativePath(char * program){
    char *relative_path = program;
    while(*relative_path != '\0'){
        if (*relative_path == '/'){
            if (access(program, X_OK) == 0){
                return 1;
            }
            else{
                return -1;
            }
        }
        relative_path++;
    }
    return 0;
}


char *isValidOtherProgram(char *program){
    char *bin = (char *) malloc(sizeof(char) * (strlen(program) + 6));

    strcpy(bin, "/bin/");
    strcat(bin, program);

    if (access(bin, X_OK) == 0){
        return bin;
    }
    else{
        char *usrbin = (char *) malloc(sizeof(char) * (strlen(program) + 10));
        strcpy(usrbin, "/usr/bin/");
        strcat(usrbin, program);
        if (access(usrbin, X_OK) == 0){
            return usrbin;
        }
        else{
            return NULL;
        }
    }
}

int isValidDirectProgram(char *program){
    if (*program == '.' && *(program+1) == '/'){
        if (access(program, X_OK) == 0){
            return 1;
        }
        else{
            return -1;
        }
    }
    else{
        return 0;
    }
}


int isValidArg_Filename_CmdName(char *arg){
    for (char *a = arg; *a != '\0' ; a++){
        if (*a == '>' || *a == '<' || *a == '|' || *a == '*' || *a == '!' || *a == '`' || *a == '\'' || *a == '"'){
            return 0;
        }
    }
    return 1;
}

int check_file(char *filename, int flag, int w_flag){
    if (!filename || !isValidArg_Filename_CmdName(filename)){
        return 0;
    }
    if (access(filename, flag) == 0){
        if (flag == R_OK){
            jobs_list_tail->input_fd = open(filename, O_RDONLY);
        }
        else {
            jobs_list_tail->output_fd = open(filename, O_WRONLY | O_CREAT | w_flag, S_IRUSR | S_IWUSR | S_IXUSR);
        }
        return 1;
    }
    else{
        return -1;
    }
}


int terminate(char **command, int w_flag) {
    char *filename = strtok_r(NULL, " ", command);
    return check_file(filename, W_OK, w_flag);;
}


int cmd_(char **command){
    jobs_list_tail->next = (struct Jobs*) malloc(sizeof(struct Jobs));
    jobs_list_tail->next->pre =jobs_list_tail;
    jobs_list_tail = jobs_list_tail->next;
    jobs_list_tail->next = NULL;
    jobs_list_tail->input_fd = STDIN_FILENO;
    jobs_list_tail->output_fd = STDOUT_FILENO;
    jobs_list_tail->cmdname = NULL;
    jobs_list_tail->args = NULL;

    struct Jobs *current_job = jobs_list_tail;

    char *cmdname = strtok_r(NULL, " ", command);

    if (!cmdname || isBuiltin(cmdname) || !isValidArg_Filename_CmdName(cmdname)){
        return 0;
    }
    if (!isValidAbsPath(cmdname)){
        if (!isValidRelativePath(cmdname)){
            if (!isValidDirectProgram(cmdname)){
                if (!isValidOtherProgram(cmdname)){
                    return -2;
                }
                else{
                    cmdname = isValidOtherProgram(cmdname);
                }
            }
        }
    }

    current_job->cmdname = (char*) malloc(sizeof(char) * (strlen(cmdname) + 1));
    strcpy(current_job->cmdname, cmdname);
    current_job->args = (char**) malloc(sizeof(char*) * 2);
    current_job->args[0] = (char*) malloc(sizeof(char) * (strlen(cmdname) + 1));
    strcpy(current_job->args[0], cmdname);
    int args_count = 0;

    char *arg = strtok_r(NULL, " ", command);
    while (arg){
        if (strcmp(arg, "<") == 0){
            if (isPipe != 0){
                return 0;
            }
            char *filename = strtok_r(NULL, " ", command);
            int check_file_status = check_file(filename, R_OK, 0);
            if (check_file_status <= 0){
                return check_file_status;
            }
            char *operator = strtok_r(NULL, " ", command);
            if (!operator){
                return 1;
            }
            if (strcmp(operator, "|") == 0){
                isPipe++;
                return cmd_(command);
            }
            else if (strcmp(operator, ">") == 0 || strcmp(operator, ">>") == 0){
                int write_flag;
                if (strcmp(operator, ">") == 0){
                    write_flag = O_TRUNC;
                }
                else{
                    write_flag = O_APPEND;
                }
                return terminate(command ,write_flag);
            }
        }
        else if (strcmp(arg, "|") == 0){
            isPipe++;
            return cmd_(command);
        }
        else if (strcmp(arg, ">") == 0 || strcmp(arg, ">>") == 0){
            int write_flag;
            if (strcmp(arg, ">") == 0){
                write_flag = O_TRUNC;
            }
            else{
                write_flag = O_APPEND;
            }
            return terminate(command ,write_flag);
        }
        else if(!isValidArg_Filename_CmdName(arg)){
            return 0;
        }
        args_count++;
        current_job->args = (char**) realloc(current_job->args, sizeof(char*) * (args_count + 2));
        current_job->args[args_count] = (char*) malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy(current_job->args[args_count], arg);
        arg = strtok_r(NULL, " ", command);
    }
    current_job->args[args_count+1] = NULL;

    return 1;
}


int isValidCommand_(char *command){
    char *copied_command = (char*) malloc(sizeof(char) * (strlen(command) + 1));
    strcpy(copied_command, command);
    char *saved_command = NULL;
    char *cmd = strtok_r(copied_command, " ", &saved_command);
    char *copied_command1 = (char*) malloc(sizeof(char) * (strlen(command) + 1));
    strcpy(copied_command1, command);
    if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "fg") == 0){
        char *arg = strtok_r(NULL, " ", &saved_command);
        if (!arg || saved_command){
            fprintf(stderr, "Error: invalid command\n");
            return 0;
        }
        return 2;
    }
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "jobs") == 0){
        if (saved_command){
            fprintf(stderr, "Error: invalid command\n");
            return 0;
        }
        return 2;
    }
    int cmd_value = cmd_(&copied_command1);
    if (cmd_value == 1){
        char *operator = strtok_r(NULL, " ", &copied_command1);
        if (!operator){
            return 1;
        }
        else if (strcmp(operator, "<") == 0 && isPipe == 0){
            char *filename = strtok_r(NULL, " ", &copied_command1);
            return check_file(filename, R_OK, 0);
        }
        else{
            return 0;
        }
    }
    else {
        if (cmd_value == 0){
            fprintf(stderr, "Error: invalid command\n");
        }
        else if (cmd_value == -1){
            fprintf(stderr, "Error: invalid file\n");
        }
        else if (cmd_value == -2){
            fprintf(stderr, "Error: invalid program\n");
        }
        return cmd_value;
    }
}