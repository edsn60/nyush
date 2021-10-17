//
// Created by Ysw on 2021/10/2.
//

#ifndef OS_LAB2_NYUSH_H
#define OS_LAB2_NYUSH_H

#endif //OS_LAB2_NYUSH_H


struct SuspendedJobs{
    int jobID;
    pid_t Pgid;
    char *command;
    struct SuspendedJobs *next;
    struct SuspendedJobs *pre;
};


struct Jobs{
    char *cmdname;
    char **args;
    int input_fd;
    int output_fd;
    struct Jobs *next;
    struct Jobs *pre;
};