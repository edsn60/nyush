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


/** To find out if this command is a builtin command
 *
 * @param cmdname:: the command name
 * @return:: 0, if not builtin
 *           1, if builtin
 */
static int isBuiltin(char *cmdname){
    if (strcmp(cmdname, "cd") == 0 || strcmp(cmdname, "single_command_jobs") == 0 || strcmp(cmdname, "exit") == 0 || strcmp(cmdname, "fg") == 0){
        return 1;
    }
    return 0;
}


/** To check if the given command is in the form of an absolute path, for example, "/bin/ls"
 *
 * @param cmdname:: the command name
 * @return:: 0, if not absolute path
 *           1, if absolute path
 */
static int isValidAbsPath(char *cmdname){
    if (*cmdname == '/'){
        if (access(cmdname, X_OK) == 0){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}


/** To check if the given command is in the form of a relative path, for example, "dir1/dir2/program"
 *
 * @param cmdname:: the command name
 * @return:: 0, if not a relative path
 *           1, if a relative path
 */
static int isValidRelativePath(char * cmdname){
    char *relative_path = cmdname;
    while(*relative_path != '\0'){
        if (*relative_path == '/'){
            if (access(cmdname, X_OK) == 0){
                return 1;
            }
            else{
                return 0;
            }
        }
        relative_path++;
    }
    return 0;
}


/** To check if the given command needs to be located in "/bin" and "/usr/bin"
 *
 * @param cmdname:: the command name
 * @return:: NULL, not found in the above two directories
 *           the found absolute path, if found in the above two directories
 */
static char *isValidOtherProgram(char *cmdname){
    char *bin = (char *) malloc(sizeof(char) * (strlen(cmdname) + 6));

    strcpy(bin, "/bin/");
    strcat(bin, cmdname);

    if (access(bin, X_OK) == 0){
        return bin;
    }
    else{
        char *usrbin = (char *) malloc(sizeof(char) * (strlen(cmdname) + 10));
        strcpy(usrbin, "/usr/bin/");
        strcat(usrbin, cmdname);
        if (access(usrbin, X_OK) == 0){
            return usrbin;
        }
        else{
            return NULL;
        }
    }
}


/** To find out if the given cmdname is an executable file directly under current working directory, for example, "./loop"
 *
 * @param cmdname:: the command name
 * @return::  1, if is and permitted to run the file
 *           -1, if is but no permission to run the file
 *            0, if not
 */
static int isValidDirectProgram(char *cmdname){
    if (*cmdname == '.' && *(cmdname + 1) == '/'){
        if (access(cmdname, X_OK) == 0){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}


/** To check the if a filename, cmdname or arg is valid
 *
 * @param arg:: a filename, cmdname or arg
 * @return:: 1, if valid
 *           0, if not valid
 */
static int isValidArg_Filename_CmdName(char *arg){
    for (char *a = arg; *a != '\0' ; a++){
        if (*a == '>' || *a == '<' || *a == '|' || *a == '*' || *a == '!' || *a == '`' || *a == '\'' || *a == '"'){
            return 0;
        }
    }
    return 1;
}


/** To check the if the filename is valid and the file exists
 *
 * @param filename:: the name or path of the file
 * @param flag:: read/write flag, can be '''R_OK''' or '''W_OK'''
 * @param w_flag:: write mode flag, can be '''O_TRUNC''' or '''O_APPEND'''
 * @return:: -1, if the file does not exist or have no permission to read/write as required
 *            0, if filename is not valid
 *            1, if filename is valid and permitted to read/write
 */
static int check_file(char *filename, int flag, int w_flag){
    if (!filename || !isValidArg_Filename_CmdName(filename)){
        return 0;
    }
    if (access(filename, flag) != 0){
        if (flag == R_OK){
            return -1;
        }
        else {
            jobs_list_tail->output_fd = open(filename, O_WRONLY | O_CREAT | w_flag, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            return 1;
        }
    }
    else{
        if (flag == R_OK){
            jobs_list_tail->input_fd = open(filename, O_RDONLY);
        }
        else {
            jobs_list_tail->output_fd = open(filename, O_WRONLY | O_CREAT | w_flag, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        }
        return 1;
    }
}


/** Based on the given description about the grammar, this is the implementation of the terminate block
 *
 * @param command:: the input command
 * @param w_flag:: write mode flag, can be '''O_TRUNC''' or '''O_APPEND'''
 * @param isPipe:: a flag represents if the command is piped
 * @param allowed_input_redirect:: number of allowed input redirection
 * @return:: the results of the function '''check_file(...)'''
 */
static int terminate(char **command, int w_flag, const int *isPipe, int *allowed_input_redirect) {
    char *filename = strtok_r(NULL, " ", command);
    char *operator = strtok_r(NULL, " ", command);
    if (!operator){
        return check_file(filename, W_OK, w_flag);
    }
    else{
        if (strcmp(operator, "<") != 0){
            return 0;
        }
        else{
            if (*isPipe != 0 || *allowed_input_redirect != 1){
                return 0;
            }
            else{
                (*allowed_input_redirect)--;
                char *filename2 = strtok_r(NULL, " ", command);
                if (*command && strlen(*command) != 0){
                    return 0;
                }
                int checkfile1 = check_file(filename, W_OK, w_flag);
                if (checkfile1 != 1){
                    return checkfile1;
                }
                int checkfile2 = check_file(filename2, R_OK, 0);
                if (checkfile2 != 1){
                    return checkfile2;
                }
            }
        }
    }
    return 1;
}


/** Based on the given description about the grammar, this is the implementation of the cmd block.
 *      Within this block, the command will be processed into a double linked list (jobs list) with all the necessary information.
 *      Once this function is called, there will be a new node in the jobs list.
 *
 * @param command:: the input command
 * @param isPipe:: a flag represents if the command is piped
 * @param allowed_input_redirect:: number of allowed input redirection
 * @param allowed_output_redirect:: number of allowed output redirection
 * @return::  1, valid command
 *            0, invalid command
 *           -1, invalid file
 *           -2, invalid program
 */
static int cmd_(char **command, int *isPipe, int *allowed_input_redirect, int *allowed_output_redirect){
    if (!*command){
        return 0;
    }

    int cmd_return_value = 1;
    int terminate_return_value = 1;
    int check_file_status = 1;
    // create new node
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

    if (!cmdname || strlen(cmdname) == 0 || isBuiltin(cmdname) || !isValidArg_Filename_CmdName(cmdname)){   // check filename
        return 0;
    }


    current_job->args = (char**) malloc(sizeof(char*) * 2);
    current_job->args[0] = (char*) malloc(sizeof(char) * (strlen(cmdname) + 1));
    strcpy(current_job->args[0], cmdname);

    int args_count = 0;

    char *arg = strtok_r(NULL, " ", command);
    while (arg && strlen(arg) != 0){
        if (strcmp(arg, "<") == 0){ // if input redirection
            if (*isPipe != 0 || *allowed_input_redirect != 1){   // if not in the first subcommand
                return 0;
            }
            char *filename = strtok_r(NULL, " ", command);
            check_file_status = check_file(filename, R_OK, 0);
            (*allowed_input_redirect)--;
            char *operator = strtok_r(NULL, " ", command);
            if (!operator || strlen(operator) == 0){     // if terminated
                break;
            }
            if (strcmp(operator, "|") == 0){    // if pipe
                (*isPipe)++;
                cmd_return_value = cmd_(command, isPipe, allowed_input_redirect, allowed_output_redirect);
                break;
            }
            else if (strcmp(operator, ">") == 0 || strcmp(operator, ">>") == 0){    // if output redirection
                if (*allowed_output_redirect != 1){
                    return 0;
                } else{
                    (*allowed_output_redirect)--;
                }

                int write_flag;
                if (strcmp(operator, ">") == 0){
                    write_flag = O_TRUNC;
                }
                else{
                    write_flag = O_APPEND;
                }
                terminate_return_value = terminate(command ,write_flag, isPipe, allowed_input_redirect);
                if (terminate_return_value <= 0){
                    return terminate_return_value;
                }
                break;
            }
            else{
                return 0;
            }
        }
        else if (strcmp(arg, "|") == 0){    // if piped
            (*isPipe)++;
            cmd_return_value = cmd_(command, isPipe, allowed_input_redirect, allowed_output_redirect);
            break;
        }
        else if (strcmp(arg, ">") == 0 || strcmp(arg, ">>") == 0){    // if output redirection
            if (*allowed_output_redirect != 1){
                return 0;
            }
            (*allowed_output_redirect)--;
            int write_flag;
            if (strcmp(arg, ">") == 0){
                write_flag = O_TRUNC;
            }
            else{
                write_flag = O_APPEND;
            }
            terminate_return_value = terminate(command ,write_flag, isPipe, allowed_input_redirect);
            break;

        }
        else if(!isValidArg_Filename_CmdName(arg)){     // if invalid arg
            return 0;
        }
        args_count++;
        current_job->args = (char**) realloc(current_job->args, sizeof(char*) * (args_count + 2));
        current_job->args[args_count] = (char*) malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy(current_job->args[args_count], arg);
        arg = strtok_r(NULL, " ", command);
    }
    current_job->args[args_count+1] = NULL;


    if (cmd_return_value == 0 || terminate_return_value == 0 || check_file_status == 0){
        return 0;
    }
    if (cmd_return_value == -2){
        return -2;
    }
    // locate filename
    if (!isValidAbsPath(cmdname)){
        if (!isValidDirectProgram(cmdname)){
            if (!isValidRelativePath(cmdname)){
                if (!isValidOtherProgram(cmdname)){
                    return -2;
                }
                else{
                    cmdname = isValidOtherProgram(cmdname);
                }
            }
        }
    }
    if (cmd_return_value == -1 || terminate_return_value == -1 || check_file_status == -1){
        return -1;
    }

    current_job->cmdname = (char*) malloc(sizeof(char) * (strlen(cmdname) + 1));
    strcpy(current_job->cmdname, cmdname);
    return 1;
}


/** To check and process the input command
 *
 * @param command:: the input command
 * @param isPipe:: a flag represents if the command is piped
 * @return:: 0, if invalid
 *           1, if valid but not builtin command
 *           2, if valid and builtin command
 */
int isValidCommand(char *command, int *isPipe){

    int allowed_input_redirect = 1;
    int allowed_output_redirect = 1;

    char *copied_command = (char*) malloc(sizeof(char) * (strlen(command) + 1));
    strcpy(copied_command, command);
    char *saved_command = NULL;
    char *cmd = strtok_r(copied_command, " ", &saved_command);
    char *copied_command1 = (char*) malloc(sizeof(char) * (strlen(command) + 1));
    strcpy(copied_command1, command);
    if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "fg") == 0){  // first check if builtin command
        char *arg = strtok_r(NULL, " ", &saved_command);
        if (!arg || (saved_command && strcmp(saved_command, "") != 0)){
            fprintf(stderr, "Error: invalid command\n");
            return 0;
        }
        return 2;
    }
    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "jobs") == 0){
        if (saved_command && strcmp(saved_command, "") != 0){
            fprintf(stderr, "Error: invalid command\n");
            return 0;
        }
        return 2;
    }
    int cmd_value = cmd_(&copied_command1, isPipe, &allowed_input_redirect, &allowed_output_redirect);     // follow the given rule of the grammar
    if (cmd_value == 1){
        char *operator = strtok_r(NULL, " ", &copied_command1);
        if (!operator || strlen(operator) == 0){
            return 1;
        }
        else if (strcmp(operator, "<") == 0 && *isPipe == 0 && allowed_input_redirect == 1){
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