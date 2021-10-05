////
//// Created by Ysw on 2021/10/5.
////
//#include <unistd.h>
//
//#include "structs.h"
//
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