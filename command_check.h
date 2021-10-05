//
// Created by Ysw on 2021/10/3.
//

#ifndef OS_LAB2_COMMAND_CHECK_H
#define OS_LAB2_COMMAND_CHECK_H

#endif //OS_LAB2_COMMAND_CHECK_H
int isValidSubcommand(char *subcommand, int subcommand_size, int current_subcommand_index);
int isValidCommand(char* command);
int isBuiltin(char *program);
int isValidAbsPath(char *program);
int isValidRelativePath(char * program);
char *isValidOtherProgram(char *program);
int isValidDirectProgram(char *program);