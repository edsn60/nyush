//
// Created by Ysw on 2021/10/2.
//

#ifndef OS_LAB2_IO_REDIRECTION_H
#define OS_LAB2_IO_REDIRECTION_H

#endif //OS_LAB2_IO_REDIRECTION_H

int input_redirection(char *filename);
int output_redirection(char *filename, int mode);
int pipe_first_input_redirection(char **saved_subcommand);
int pipe_last_output_redirection(char **saved_subcommand);