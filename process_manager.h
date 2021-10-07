//
// Created by Ysw on 2021/10/2.
//

#ifndef OS_LAB2_SIGNAL_HANDLER_H
#define OS_LAB2_SIGNAL_HANDLER_H

#endif //OS_LAB2_SIGNAL_HANDLER_H

void child_process_signal_handler(pid_t pgid);
void continued_job_handler(siginfo_t *infop, pid_t pgid);