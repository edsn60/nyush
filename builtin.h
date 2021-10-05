//
// Created by Ysw on 2021/10/2.
//

#ifndef OS_LAB2_BUILTIN_H
#define OS_LAB2_BUILTIN_H

#endif //OS_LAB2_BUILTIN_H

void cd_(char **saved_str);
void fg_(char **saved_str);
int exit_(char **saved_str);
void jobs_(char **saved_str);
int builtin_handler(char *command);