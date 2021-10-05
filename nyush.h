//
// Created by Ysw on 2021/10/2.
//

#ifndef OS_LAB2_NYUSH_H
#define OS_LAB2_NYUSH_H

#endif //OS_LAB2_NYUSH_H


//typedef struct Pipe_Subcommand {
//    int subcommand_idx;
//    char *program;
//    char **args;
//    int input_fd;
//    int output_fd;
//    struct Pipe_Subcommand *next;
//    struct Pipe_Subcommand *pre;
//}pipe_subcommand;
//
//
//typedef struct Single_Command_Jobs{
//    char *program;
//    char *command;
//    char **args;
//    int input_fd;
//    int output_fd;
//    int pid;
//    int pgid;
//}single_command_jobs;
//
//
//typedef struct SuspendedJobs{
//    int jobID;
//    pid_t Pid;
//    pid_t Pgid;
//    int isPipe;
//    int status;
//    char *command;
//    struct SuspendedJobs *next;
//    struct SuspendedJobs *pre;
//}suspendedjobs;


struct Pipe_Subcommand {
    int subcommand_idx;
    char *program;
    char **args;
    int input_fd;
    int output_fd;
    struct Pipe_Subcommand *next;
    struct Pipe_Subcommand *pre;
};


struct Single_Command_Jobs{
    char *program;
    char *command;
    char **args;
    int input_fd;
    int output_fd;
    int pid;
    int pgid;
};


struct SuspendedJobs{
    int jobID;
    pid_t Pid;
    pid_t Pgid;
    int isPipe;
    int status;
    char *command;
    struct SuspendedJobs *next;
    struct SuspendedJobs *pre;
};

//struct Pipe_Subcommand;
//struct Single_Command_Jobs;
//struct SuspendedJobs;
