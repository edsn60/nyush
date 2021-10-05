//
// Created by Ysw on 2021/10/2.
//

#include "pipe_.h"
#include "nyush.h"

#ifndef OS_LAB2_SIGNAL_HANDLER_H
#define OS_LAB2_SIGNAL_HANDLER_H

#endif //OS_LAB2_SIGNAL_HANDLER_H
void child_process_signal_handler(struct Pipe_job *pipejob, struct Single_Command_Jobs *singlejob);
void continued_job_handler(siginfo_t *infop, pid_t pgid);