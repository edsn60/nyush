//
// Created by Ysw on 2021/10/3.
//

#ifndef OS_LAB2_PIPE__H
#define OS_LAB2_PIPE__H

#endif //OS_LAB2_PIPE__H

void pipe_parser(char *input);

struct Pipe_job{
    char job_command[1001];
    struct Pipe_Subcommand *subcommand_head;
    int subcommand_count;
    int PGID;
};