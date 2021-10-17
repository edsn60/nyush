# NYUSH

This is a brief description of the project.

## Project Structure
```
nyush
├── CMakeLists.txt
├── README.md
├── builtin.c
├── builtin.h
├── command_parser.c
├── command_parser.h
├── execute_command.c
├── execute_command.h
├── nyush.c
├── nyush.h
├── process_manager.c
├── process_manager.h
└── Call_Graph.png
```

## Project File Description
> ```nyush.c```
> 
> The shell starts here. This file contains some initialization code when shell starts.
>
> ```nyush.h```
>
> Some definition of structs including
> ```c
> struct SuspendedJobs;    // double linked list to store all suspended jobs
> struct Jobs;     // double linked list to store the current job after each input
>```

> ```command_parser.c```
> 
> This file contains the command parser, it parses the command following the rules given in the lab description.
> 
> Plus, it will preprocess the input command for later execution.
>
>

> ```builtin.c```
>
> This contains the implementation of four builtin commands ```cd```, ```jobs```, ```fg```, and ```exit```, 
> plus a ```builtin_handler()```
>

> ```execute_command.c```
> 
> This code in this file will be responsible for executing non-builtin commands including pipes.
>


> ```process_manager.c```
>
> This is the process manager of the shell, it handles all suspended jobs and those suspended before but continued later.

## Call Graph

![](https://github.com/edsn60/nyush/blob/main/Call_Graph.png)
