//
// Created by Ysw on 2021/10/2.
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include "IO_redirection.h"


/**
 * Get the file descriptor
 *
 * @param filename:: the path of the file
 * @return:: -1, if the file does not exist or read permission denied
 *           other, if the file exists and permits the user to read.
 */
int input_redirection(char *filename){
    if (!filename || access(filename, 4) == -1){
        fprintf(stderr, "Error: invalid file\n");
        return -1;
    }
    int fd = open(filename, O_RDONLY);
    return fd;
}


/**
 * Get the file descriptor, if the file does not exist, create first
 *
 * @param filename:: the path of the file
 * @param mode:: write mode, 1 if append, 0 if truncated
 * @return:: the file descriptor
 */
int output_redirection(char *filename, int mode){
    int fd;
    if (mode == 0){    // trunc
        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
        fd = dup(fd);
    }
    else{   // append
        fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    }

    return fd;
}


int pipe_first_input_redirection(char **saved_subcommand){
    int fd = STDIN_FILENO;
    char *redirected_input_operator = strtok_r(NULL, " ", saved_subcommand);
    if (redirected_input_operator && strcmp(redirected_input_operator, "<") == 0){
        char *filename = strtok_r(NULL, " ", saved_subcommand);
        fd = input_redirection(filename);
    }
    return fd;
}


int pipe_last_output_redirection(char **saved_subcommand){
    int fd = STDOUT_FILENO;
    char *redirected_output_operator = strtok_r(NULL, " ", saved_subcommand);
    if (redirected_output_operator){
        char *filename = strtok_r(NULL, " ", saved_subcommand);
        if (strcmp(redirected_output_operator, ">") == 0) {
            fd = output_redirection(filename, 0);
        }
        else{
            fd = output_redirection(filename, 1);
        }
    }
    return fd;
}