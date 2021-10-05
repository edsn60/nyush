//
// Created by Ysw on 2021/10/3.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


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


/**
 * To check if each subcommand is valid
 *
 * @param subcommand:: the whole subcommand needs to be checked
 * @param subcommand_size:: the amount of subcommands in the whole command, each is split by '|'
 * @param current_subcommand_index:: the index of the current subcommand being checked
 * @return:: 0, if valid
 *          -1, if invalid command
 *          -2, if invalid program
 *          -3, if invalid IO redirection file
 */
int isValidSubcommand(char *subcommand, int permitted_input_redirection, int permitted_output_redirection){
    if (*subcommand == '\0'){
        return -1;
    }
    char *saved_arg = NULL;
    char *program = strtok_r(subcommand, " ", &saved_arg);
    // TODO:: is '''< 123''' invalid program or invalid command?
//    if ()
    if (isBuiltin(program)){
        return -1;
    }
    if (!isValidAbsPath(program)){
        if (!isValidRelativePath(program)){
            if (!isValidDirectProgram(program)){
                if (!isValidOtherProgram(program)){
                    return -2;
                }
            }
        }
    }

    char *arg = strtok_r(NULL, " ", &saved_arg);
    while (arg){
        if (*arg == '<'){
            if (strcmp(arg, "<") != 0){
                return -1;
            }
            if (!saved_arg || *saved_arg == '<' || *saved_arg == '>'){
                return -1;
            }
            if (permitted_input_redirection == 1){
                permitted_input_redirection--;
            }
            else{
                return -1;
            }
            arg = strtok_r(NULL, " ", &saved_arg);
            if (access(arg, R_OK) == -1){
                if (!saved_arg || *saved_arg == '>'){
                    return -3;
                }
                else{
                    return -1;
                }
            }
            else{
                if (saved_arg && *saved_arg != '>'){
                    return -1;
                }
            }

        }
        else if (*arg == '>'){
            if (strcmp(arg, ">") != 0 && strcmp(arg, ">>") != 0)
            if (!saved_arg || *saved_arg == '<' || *saved_arg == '>'){
                return -1;
            }
            if (permitted_output_redirection == 1){
                permitted_output_redirection --;
            }
            else{
                return -1;
            }
            arg = strtok_r(NULL, " ", &saved_arg);
            if (!arg){
                return -1;
            }
            if (saved_arg && *saved_arg != '<'){
                return -1;
            }

        }
//        else if (strcmp(arg, "<<") == 0){
//            return -1;
//        }
        arg = strtok_r(NULL, " ", &saved_arg);
    }
    return 0;
}


/**
 * To check if the command is valid in general
 *
 * @param command:: the whole input command
 * @return:: 2, if builtin
 *           1, if valid and pipe
 *           0, if valid and not pipe
 *          -1, if invalid
 */
int isValidCommand(char* command){
    char copied_command[1001] = {};
    strcpy(copied_command, command);
    int count_subcommands = 1;
    for (char *program = copied_command; *program; program++){
        if (*program == '|'){
            count_subcommands++;
        }
    }

    if (count_subcommands == 1){
        char *saved_args;
        char copied_single_command[1001] = {};
        strcpy(copied_single_command, copied_command);
        char *program = strtok_r(copied_single_command, " ", &saved_args);
        if (isBuiltin(program)){
            return 2;
        }
        int isValid = isValidSubcommand(copied_command, 1, 1);
        if (isValid == -1){
            fprintf(stderr, "Error: invalid command\n");
            return -1;
        }
        else if (isValid == -2){
            fprintf(stderr, "Error: invalid program\n");
            return -1;
        }
        else if (isValid == -3){
            fprintf(stderr, "Error: invalid file\n");
            return -1;
        }
        else{
            return 0;
        }
    }

    char *saved_subcommands = NULL;
    char *subcommand = strtok_r(copied_command, "|", &saved_subcommands);
    int current = 1;
    while (subcommand){
        int permitted_input_redirection;
        int permitted_output_redirection;
        if (current == 1){
            permitted_input_redirection = 1;
            permitted_output_redirection = 0;
        }
        else if (!saved_subcommands){
            permitted_input_redirection = 0;
            permitted_output_redirection = 1;
        }
        else{
            permitted_input_redirection = 0;
            permitted_output_redirection = 0;
        }
        int isValid = isValidSubcommand(subcommand, permitted_input_redirection, permitted_output_redirection);
        if (isValid == -1){
            fprintf(stderr, "Error: invalid command\n");
            return -1;
        }
        else if (isValid == -2){
            fprintf(stderr, "Error: invalid program\n");
            return -1;
        }
        else if (isValid == -3){
            fprintf(stderr, "Error: invalid file\n");
            return -1;
        }
        current++;
        subcommand = strtok_r(NULL, "|", &saved_subcommands);
    }
    if (current <= count_subcommands){
        fprintf(stderr, "Error: invalid command\n");
        return -1;
    }
    else{
        return 1;
    }
}