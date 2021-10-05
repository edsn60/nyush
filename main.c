//#include <stdio.h>
//#include <string.h>
//#include <signal.h>
//#include <dirent.h>
//#include <unistd.h>
//#include <libgen.h>
//#include <sys/stat.h>
//#include <stdlib.h>
//#include <sys/fcntl.h>
//#include <ctype.h>
//#include <termios.h>
//
//void cd_(char **saved_str);
//void fg_(char **saved_str);
//void exit_(char **saved_str);
//void jobs_(char **saved_str);
//int input_redirection(char *filename);
//int output_redirection(char *filename, int mode);
//
//
//typedef struct SuspendedJobs{
//    pid_t Pid;
//    char *command;
//    struct SuspendedJobs *next;
//    struct SuspendedJobs *pre;
//}processes;
//
//processes *process_list_head = NULL;
//processes *process_list_tail = NULL;
//
//char *stp_command = NULL;


//int isValidSubcommand(char *program, char** saved_program, int command_size, int current){
//    if (*program == '\0'){
//        return -1;
//    }
//    int input;
//    int output;
//    if (command_size == 1){
//        input = 1;
//        output = 1;
//    }
//    else if (current == 1){
//        input = 1;
//        output = 0;
//    }
//    else if (current == command_size){
//        input = 0;
//        output = 1;
//    }
//    else{
//        input = 0;
//        output = 0;
//    }
//
//    char *saved_arg = NULL;
//    char *arg = strtok_r(program, " ", &saved_arg);
//    while (arg){
//        if (strcmp(arg, "<") == 0){
//            if (!saved_arg || *saved_arg == '<' || *saved_arg == '>'){
//                return -1;
//            }
//            if (input == 1){
//                input--;
//            }
//            else{
//                return -1;
//            }
//        }
//        else if (strcmp(arg, ">") == 0 || strcmp(arg, ">>") == 0){
//            if (!saved_arg || *saved_arg == '<' || *saved_arg == '>'){
//                return -1;
//            }
//            if (output == 1){
//                output --;
//            }
//            else{
//                return -1;
//            }
//        }
//        else if (strcmp(arg, "<<") == 0){
//            return -1;
//        }
//        arg = strtok_r(NULL, " ", &saved_arg);
//    }
//    return 0;
//}
//
//
//int isValidCommand(char* command){
//    char check_command[1001] = {};
//    strcpy(check_command, command);
//    int count_programs = 1;
//    for (char *program = check_command; *program; program++){
//        if (*program == '|'){
//            count_programs++;
//        }
//    }
//    char **programs = (char**) malloc(sizeof(char*) * (count_programs+1));
//    char *saved_program = NULL;
//    char *program = strtok_r(check_command, "|", &saved_program);
//    char **program_ptr = programs;
//    int current = 1;
//    while (program){
//        if (isValidSubcommand(program, &saved_program, count_programs, current) == -1){
//            return -1;
//        }
//        current++;
//        program = strtok_r(NULL, "|", &saved_program);
//    }
//    if (current <= count_programs){
//        return -1;
//    }
//    return 0;
//}


//void child_process_signal_handler(pid_t pid, int status){
//    if (WIFEXITED(status)){
//        printf("child exited: %d\n", WEXITSTATUS(status));
//    }
//    else if (WIFSIGNALED(status)){
//        printf("child signaled: %d", WTERMSIG(status));
//    }
//    else if (WIFSTOPPED(status)){
//        if (!process_list_head){
//            process_list_head = (processes*) malloc(sizeof(processes));
//            process_list_head->Pid = pid;
//            process_list_head->pre = NULL;
//            process_list_head->next = NULL;
//            process_list_head->command = (char*) malloc(sizeof(char) * (strlen(stp_command)+1));
//            strcpy(process_list_head->command, stp_command);
//            process_list_tail = process_list_head;
//        }
//        else{
//            process_list_tail->next = (processes*) malloc(sizeof(processes));
//            process_list_tail->next->pre = process_list_tail;
//            process_list_tail = process_list_tail->next;
//            process_list_tail->Pid = pid;
//            process_list_tail->next = NULL;
//            process_list_tail->command = (char*) malloc(sizeof(char) * (strlen(stp_command)+1));
//            strcpy(process_list_tail->command, stp_command);
//        }
//        printf("child stopped: %d", WSTOPSIG(status));
//    }
//    // TODO:: Add suspended single_command_jobs to processes list
//}


//void fg_(char **saved_str){
//    if (!*saved_str){
//        fprintf(stderr, "Error: invalid command\n");
//        return;
//    }
//    int idx = 0;
//    char *str_idx = strtok_r(NULL, " ", saved_str);
//    for (char *c = str_idx; *c; c++){
//        if (isdigit(*c) == 0){
//            fprintf(stderr, "Error: invalid suspendedjobs\n");
//            return;
//        }
//        idx = idx * 10 + (int)*c;
//    }
//    if (idx == 0){
//        fprintf(stderr, "Error: invalid suspendedjobs\n");
//        return;
//    }
//    processes *processes = process_list_head;
//    while (processes && idx > 1){
//        processes = processes->next;
//        idx--;
//    }
//    if (!processes){
//        fprintf(stderr, "Error: invalid suspendedjobs\n");
//        return;
//    }
//    kill(processes->Pid, SIGCONT);
//    int status;
//    waitpid(processes->Pid, &status, WUNTRACED);
//    // TODO:: what if suspended again?
//}
//
//
//void exit_(char **saved_str){
//    if (*saved_str){
//        fprintf(stderr, "Error: invalid command");
//        return;
//    }
//    if (process_list_head){
//        fprintf(stderr, "Error: there are suspended single_command_jobs\n");
//        return;
//    }
//}
//
//
//void jobs_(char **saved_str){
//    if (*saved_str){
//        fprintf(stderr, "Error: invalid command\n");
//        return;
//    }
//    processes *processes = process_list_head;
//    int idx = 1;
//    while(processes){
//        printf("[%d] %s", idx, processes->command);
//        processes = processes->next;
//    }
//}
//
//
//void cd_(char **saved_str){
//    char *dest = strtok_r(NULL, " ", saved_str);
//    if(!dest || strtok_r(NULL, " ", saved_str)){
//        fprintf(stderr, "Error: invalid command\n");
//        return;
//    }
//    if(access(dest, 0) == -1){
//        fprintf(stderr, "Error: invalid directory\n");
//        return;
//    }
//    else{
//        struct stat buf;
//        int flag = open(dest, O_RDONLY);
//        fstat(flag, &buf);
//        if(!S_ISDIR(buf.st_mode)){
//            fprintf(stderr, "Error: invalid directory\n");
//            return;
//        }
//        else{
//            chdir(dest);
//            return;
//        }
//    }
//}


//void execute_other_program(char *command, char **saved_str){
//    char **args = NULL;
//    int size = strlen(command)+2;
//    args = (char**)malloc(sizeof(char*) * size);
//    args[0] = (char*) malloc(sizeof(char) * (strlen(command)+1));
//    strcpy(args[0], command);
//    char *arg = NULL;
//    int idx = 1;
//    int isOutputRedirected = 0;
//    int isInputRedirected = 0;
//    int isPipe = 0;
//    int redirected_input_fd = -1;
//    int redirected_output_fd = -1;
//    int stdout_fileno = dup(STDOUT_FILENO);
//    int stdin_fileno = dup(STDIN_FILENO);
//    int input_newfd = stdin_fileno;
//    int output_newfd = stdout_fileno;
//    while (*saved_str && **saved_str != '<' && **saved_str != '>' && **saved_str != '|') {
//        arg = strtok_r(NULL, " ", saved_str);
//        size = size + strlen(arg) + 1;
//        args = (char **) realloc(args, sizeof(char *) * size);
//        args[idx] = (char *) malloc(sizeof(char) * size);
//        strcpy(args[idx], arg);
//        idx++;
////        if (!*saved_str || **saved_str == '<' || **saved_str == '>' || **saved_str == '|') {
////            break;
////        }
//    }
//    while (*saved_str && (**saved_str == '<' || **saved_str == '>')) {
//        if (**saved_str == '<') {
//            char *input_redirection_operator = strtok_r(NULL, " ", saved_str);
//            char *filename = strtok_r(NULL, " ", saved_str);
//            if (!filename || strcmp(input_redirection_operator, "<") != 0) {
//                fprintf(stderr, "Error: invalid file\n");
//                return;
//            }
//            redirected_input_fd = input_redirection(filename);
//            if (redirected_input_fd == -1) {
////                fprintf(stderr, "Error: invalid file\n");
//                return;
//            }
//
//            isInputRedirected = 1;
//            input_newfd = dup2(redirected_input_fd, STDIN_FILENO);
//        } else if (**saved_str == '>') {
//            char *output_redirection_operator = strtok_r(NULL, " ", saved_str);
//            char *filename = strtok_r(NULL, " ", saved_str);
//            int mode;
//            if (!filename || strcmp(output_redirection_operator, ">") != 0) {
//                if (strcmp(output_redirection_operator, ">>") != 0) {
//                    fprintf(stderr, "Error: invalid file\n");
//                    return;
//                } else {
//                    mode = 1;
//                }
//            } else {
//                mode = 0;
//            }
//
//            redirected_output_fd = output_redirection(filename, mode);
//            isOutputRedirected = 1;
//            output_newfd = dup2(redirected_output_fd, STDOUT_FILENO);
//        }
//    }
//
//    args[idx] = NULL;
//    int status;
//    int child = fork();
//    if (child == 0){
////        setpgid(getpid(), getpid());
//        execv(command, args);
//        exit(-1);
//    }
//    else{
//        setpgid(child, child);
//        tcsetpgrp(STDOUT_FILENO | STDIN_FILENO, child);
////        tcsetpgrp(output_newfd, child);
//        waitpid(child, &status, WUNTRACED);
//        tcsetpgrp(STDIN_FILENO | STDOUT_FILENO, getpid());
//
//        if (isOutputRedirected) {
//            close(redirected_output_fd);
//            dup2(stdout_fileno, output_newfd);
//        }
//        if (isInputRedirected) {
//            close(redirected_input_fd);
//            dup2(stdin_fileno, input_newfd);
//        }
//    }
//}
//
//
//int locate_other_program(char *command, char **saved_str){
//    if (*command == '.'){   // check if is under cwd
//        char *tmp = command;
//        tmp++;
//        if(*tmp == '/'){
//            tmp++;
//            if (access(tmp, 1) != 0){   // not found under cwd
//                fprintf(stderr, "Error: invalid program\n");
//                return 1;
//            }
//            else{
//                execute_other_program(command, saved_str);  // found under cwd
//            }
//        }
//    }
//    else if (*command == '/') {    // check if is an abs_path
//        if (access(command, 1) != 0){
//            fprintf(stderr, "Error: invalid program\n");
//            return 1;
//        }
//        else{   // is an abs_path
//            execute_other_program(command, saved_str);
//        }
//    }
//    else{   // neither, then check /bin and /usr/bin
//        char *bin = (char*) malloc(sizeof(char) * (5 + strlen(command) + 1));
//        strcpy(bin, "/bin/");
//        strcat(bin, command);
//        if (access(bin, 1) != 0){
//            char *usrbin = (char*) malloc(sizeof(char) * (9 + strlen(command) + 1));
//            strcpy(usrbin, "/usr/bin/");
//            if (access(strcat(usrbin, command), 1) != 0){
//                fprintf(stderr, "Error: invalid program\n");    // not found under /bin and /usr/bin
//                return 1;
//            }
//            else{   // under /usr/bin
//                execute_other_program(usrbin, saved_str);
//            }
//        }
//        else{   // under /bin
//            execute_other_program(bin, saved_str);
//        }
//    }
//    return 1;
//}


//int input_redirection(char *filename){
//    if (!filename || access(filename, 4) == -1){
//        fprintf(stderr, "Error: invalid file\n");
//        return -1;
//    }
//    int fd = open(filename, O_RDONLY);
////    FILE *fp = fopen(filename, "r");
//    return fd;
//}
//
//
//int output_redirection(char *filename, int mode){
//    int fd;
//    if (mode == 0){    // trunc
//        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
//        fd = dup(fd);
//    }
//    else{   // append
//        fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
//    }
//
//    return fd;
//}


//int builtin_handler(char *command, char **saved_str){
//    if (strcmp(command, "cd") == 0){
//        cd_(saved_str);
//    }
//    else if (strcmp(command, "single_command_jobs") == 0){
//        jobs_(saved_str);
//    }
//    else if (strcmp(command, "exit") == 0){
//        exit_(saved_str);
//        return 0;
//    }
//    else if (strcmp(command, "fg") == 0){
//        fg_(saved_str);
//    }
//    else{
//        return -1;
//    }
//    return 1;
//}


//int command_parser(char *input){
//    char *saved_str = NULL;
//    char *command = strtok_r(input, " ", &saved_str);
//    int status = builtin_handler(command, &saved_str);
//    if (status != -1) {
//        return status;
//    }
//    else{
//        int located = locate_other_program(command, &saved_str);
//    }
//    return 1;
//}


//int main() {
//    signal(SIGINT, SIG_IGN);
//    signal(SIGQUIT, SIG_IGN);
//    signal(SIGTERM, SIG_IGN);
//    signal(SIGSTOP, SIG_DFL);
//    signal(SIGTSTP, SIG_IGN);
//    signal(SIGTTIN, SIG_IGN);
//    signal(SIGTTOU, SIG_IGN);
//    printf("The default interactive shell is now nyush.\n");
//    char cwd[300] = {};
//    stp_command = (char*) malloc(sizeof(char)*1001);
//
//    char input[1001] = {};
//    while(1){
//        getcwd(cwd, sizeof(cwd));
//        char *basename1 = basename(cwd);
//        tcflush(STDERR_FILENO, TCOFLUSH);
//        printf("[nyush %s]$ ", basename1);
//        gets(input);
//
//        char *whitespace = input;
//
//        while(*whitespace == ' '){
//            whitespace++;
//        }
//        if (strlen(whitespace) == 0){
//            continue;
//        }
//        if (isValidCommand(input) == -1){
//            fprintf(stderr, "Error: invalid command\n");
//            continue;
//        }
//        strcpy(stp_command, input);
//        if (command_parser(input) == 0){
//            return 0;
//        }
//
//    }
//
//    return 0;
//}
