//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "command_parser.h"
#include "builtin.h"
#include "external_program.h"
#include "IO_redirection.h"
#include "command_check.h"


char **get_args(char *command, char **saved_str){
    unsigned long args_count = 2;
    char **args = (char**) malloc(sizeof(char*) * args_count);
    args[0] = (char*) malloc(sizeof(char)*(strlen(command)+1));
    strcpy(args[0], command);
    char *arg = NULL;
    int idx = 1;
    while (*saved_str && **saved_str != '\0' && **saved_str != '<' && **saved_str != '>'){
        arg = strtok_r(NULL, " ", saved_str);
        args_count++;
        args = (char **) realloc(args, sizeof(char *) * args_count);
        args[idx] = (char *) malloc(sizeof(char) * args_count);
        strcpy(args[idx], arg);
        idx++;
    }
    args[idx] = NULL;
    return args;
}


/**
 * To parse the input command and then execute
 *
 * @param input:: the whole input command
 * @return:: -1, if '''exit''' failed
 *            0, if '''exit''' succeeded
 *            1, if any error occurred
 */
int single_command_parser(char *input){
    char *saved_str = NULL;
    char *program = strtok_r(input, " ", &saved_str);
    char *program1 = isValidOtherProgram(program);
    if (program1){
        program = program1;
    }
//    program = locate_other_program(program);
    if (!program){
        fprintf(stderr, "Error: invalid program\n");
        return 1;
    }
    else{
        execute_other_program(program, &saved_str);
    }
    return 1;
}


