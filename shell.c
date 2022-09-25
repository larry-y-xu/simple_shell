#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ARG 8
#define MAX_LINE 80
#define MAX_PROMPT 10
#define MAX_DIR 10

void remove_newline(char *str, size_t length) {
    if (str[length - 1] == '\n') {
        str[length - 1] = '\0';
    }
}

int split_commands(char second_input_arg[MAX_ARG][MAX_LINE], char *tmp, int two_pipes[]) {
    int num_first_arg = 0;
    //check that there are commands after "->"
    tmp = strtok(NULL, " ");
    while (tmp != NULL && num_first_arg < MAX_ARG) {
        if (strcmp(tmp, "->") == 0) {
            two_pipes[0] = 1;
            break;
        }
        strncpy(second_input_arg[num_first_arg], tmp, MAX_LINE);
        num_first_arg++;
        tmp = strtok(NULL, " ");
    }
            
    return num_first_arg;
}

//takes the array containing all the paths, the user input, as well as the maximum number of paths and returns the result in file_path
int check_path(char file_path[MAX_LINE], char path_array[MAX_DIR][MAX_LINE], char input_arg[MAX_ARG][MAX_LINE], int num_paths) {
    //update path array
    char new_path[MAX_DIR][MAX_LINE];
    for (int i = 0; i < num_paths; i++) {
        strncpy(new_path[i], path_array[i], MAX_LINE);
        strncat(new_path[i], "/", 5);
        strncat(new_path[i], input_arg[0], MAX_LINE);
        //printf("%s\n", new_path[i]);
    }

    //check which directory has the executable, if any
    int file_check = 0;
    for (int i = 0; i < num_paths; i++) {
        //if the for loop terminates normally, file_check should equal 0
        if (access(new_path[i], X_OK) == 0) {
            strncpy(file_path, new_path[i], MAX_LINE);
            file_check = 1;
            break;
        }
    }
    return file_check;
}


//takes the user input, the file path and the number of arguments and outputs the result in args
void create_execve_arg(char *args[MAX_ARG], char input_arg[MAX_ARG][MAX_LINE], char file_path[MAX_LINE], int num_first_arg) {
    //create execve vars
    args[0] = file_path;
    for (int i = 1; i < num_first_arg; i++) {
        args[i] = input_arg[i];
    }
    args[num_first_arg] = 0;
}

void normal_command(char input_arg[MAX_ARG][MAX_LINE], char file_path[MAX_LINE], int num_first_arg) {
    //create execve vars
    char *args[MAX_ARG];
    create_execve_arg(args, input_arg, file_path, num_first_arg);
    int pid;
    int status;
    char *envp[] = {NULL};
    //create child process
    if ((pid = fork()) == 0) {
        execve(args[0], args, envp);
    }
    waitpid(pid, &status, 0);

}

void OR(char input_arg[MAX_ARG][MAX_LINE], char file_path[MAX_LINE], int num_first_arg, char output_file[MAX_ARG][MAX_LINE]) {
    //create execve vars
    char *args[MAX_ARG];
    create_execve_arg(args, input_arg, file_path, num_first_arg);
    int pid;
    int status;
    char *envp[] = {NULL};

    int fd;
    int save_stdout;
    int save_stderr;

    fd = open(output_file[0], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "cannot open %s for writing\n", output_file[0]);
    }

    save_stdout = dup(1);
    save_stderr = dup(2);
    dup2(fd, 1);
    dup2(fd, 2);

    //create child process
    if ((pid = fork()) == 0) {
        execve(args[0], args, envp);
    }
    waitpid(pid, &status, 0);

    dup2(save_stdout, 1);
    dup2(save_stderr, 2);

    //close(fd);
}

void PP(char file_path[MAX_LINE], char input_arg[MAX_ARG][MAX_LINE], int num_first_arg, char second_path[MAX_LINE], char second_input_arg[MAX_ARG][MAX_LINE], int num_second) {
    //create execve vars
    int pid_head, pid_tail;
    int fd[2];
    int status;
    char *envp[] = {NULL};
    char *args[MAX_ARG];
    char *second[MAX_ARG];
    create_execve_arg(args, input_arg, file_path, num_first_arg);
    create_execve_arg(second, second_input_arg, second_path, num_second);

    pipe(fd);

    //create child processes
    if ((pid_head = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        execve(args[0], args, envp);
    }

    if ((pid_tail = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[1]);
        execve(second[0], second, envp);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid_head, &status, 0);
    waitpid(pid_tail, &status, 0);

}

void PP3(char file_path[MAX_LINE], char input_arg[MAX_ARG][MAX_LINE], int num_first_arg, char second_path[MAX_LINE], char second_input_arg[MAX_ARG][MAX_LINE], int num_second, char third_path[MAX_LINE], char third_input_arg[MAX_ARG][MAX_LINE], int num_third_arg) {
    //create execve vars
    int pid_one, pid_two, pid_three;
    int fd_one[2];
    int fd_two[2];
    int status;
    char *envp[] = {NULL};
    char *args[MAX_ARG];
    char *second[MAX_ARG];
    char *third[MAX_ARG];
    create_execve_arg(args, input_arg, file_path, num_first_arg);
    create_execve_arg(second, second_input_arg, second_path, num_second);
    create_execve_arg(third, third_input_arg, third_path, num_third_arg);

    pipe(fd_one);
    pipe(fd_two);

    //create child processes
    if ((pid_one = fork()) == 0) {
        dup2(fd_one[1], 1);
        close(fd_one[0]);
        close(fd_two[0]);
        close(fd_two[1]);
        execve(args[0], args, envp);
    }

    if ((pid_two = fork()) == 0) {
        dup2(fd_one[0], 0);
        close(fd_one[1]);
        dup2(fd_two[1], 1);
        close(fd_two[0]);
        execve(second[0], second, envp);
    }

    if ((pid_three = fork()) == 0) {
        dup2(fd_two[0], 0);
        close(fd_two[1]);
        close(fd_one[0]);
        close(fd_one[1]);
        execve(third[0], third, envp);
    }

    close(fd_one[0]);
    close(fd_one[1]);
    close(fd_two[0]);
    close(fd_two[1]);

    waitpid(pid_one, &status, 0);
    waitpid(pid_two, &status, 0);
    waitpid(pid_three, &status, 0);

}


int main(int argc, char *argv[]){
    //obtain prompt
    FILE *rc = fopen("sh360-b", "r");
    char prompt[MAX_PROMPT];
    fgets(prompt, MAX_PROMPT, rc);
    remove_newline(prompt, strlen(prompt));

    //obtain path array
    char path_array[MAX_DIR][MAX_LINE];
    int num_paths = 0;
    char temp[MAX_LINE];
    while (fgets(temp, MAX_LINE, rc) != NULL) {
        remove_newline(temp, strlen(temp));
        strncpy(path_array[num_paths], temp, MAX_LINE);
        num_paths++;
    }


    for(;;) {
        char user_input[MAX_LINE];
        printf("%s", prompt);

        //get user command
        fgets(user_input, MAX_LINE, stdin);
        remove_newline(user_input, strlen(user_input));

        //check if user wanted to exit
        if (strcmp(user_input, "exit") == 0) {
            break;
        }


        //at this point we know the user has entered a command other than exit, and we attempt to parse it
        int num_first_arg = 0; //stores the number of arguments
        int cmd_type = 0; //0 is normal, 1 is output to file, 2 is pipe
        int check_redirect = 0;
        int too_many = 0;
        char *tmp;
        char input_arg[MAX_ARG][MAX_LINE]; //stores the arguments
        tmp = strtok(user_input, " ");
        //the moment we encounter a -> we know to pay attention to the argument(s) following it
        while (tmp != NULL) {
            //check if first token is OR
            if (num_first_arg == 0 && strcmp(tmp, "OR") == 0) {
                //output to file
                cmd_type = 1;
                //skip first token, we already know it is "OR"
                tmp = strtok(NULL, " ");
                continue;
            }
            else if (num_first_arg == 0 && strcmp(tmp, "PP") == 0) {
                //output to pipe
                cmd_type = 2;
                //skip first token, we already know it is "PP"
                tmp = strtok(NULL, " ");
                continue;
            }

            //hit keyword to redirect output
            if (strcmp(tmp, "->") == 0) {
                check_redirect = 1;
                break;
            }

            //actually store the input into an array
            strncpy(input_arg[num_first_arg], tmp, MAX_LINE);
            num_first_arg++;
            tmp = strtok(NULL, " ");

            if (num_first_arg >= MAX_ARG) {
                too_many = 1;
            }
        }

        if (too_many == 1) {
            fprintf(stderr, "too many arguments\n");
            continue;
        }

        if ((cmd_type == 1 || cmd_type == 2) && check_redirect == 0) {
            fprintf(stderr, "did not include ->\n");
            continue;
        }


        int num_second_arg;
        int num_third_arg;
        int two_pipes[] = {0};
        char second_input_arg[MAX_ARG][MAX_LINE];
        char third_input_arg[MAX_ARG][MAX_LINE];
        if (cmd_type == 1 || cmd_type == 2 ) {
            num_second_arg = split_commands(second_input_arg, tmp, two_pipes);
            if (num_second_arg == 0) {
                fprintf(stderr, "improperly formatted command\n");
                continue;
            }
            else if (num_first_arg + num_second_arg >= MAX_ARG) {
                fprintf(stderr, "too many arguments\n");
            }

            if (two_pipes[0] == 1) {
                num_third_arg = split_commands(third_input_arg, tmp, two_pipes);

                if (num_third_arg == 0) {
                    fprintf(stderr, "improperly formatted PP command\n");
                    continue;
                }
                else if (num_first_arg + num_second_arg + num_third_arg >= MAX_ARG) {
                    fprintf(stderr, "too many arguments\n");
                    continue;   
                }
            }


        }



        char file_path[MAX_LINE];
        int file_check = check_path(file_path, path_array, input_arg, num_paths);

        //fail gracefully
        if (file_check == 0) {
            fprintf(stderr, "command not found\n");
            continue;
        }


        //create and execute child processes based on input
        if (cmd_type == 0) {
            normal_command(input_arg, file_path, num_first_arg);
        }

        else if (cmd_type == 1) {
            //OR(input_arg, file_path, num_first_arg, output_file);
            OR(input_arg, file_path, num_first_arg, second_input_arg);
        }

        char second_path[MAX_LINE];
        int second_check = 0;
        char third_path[MAX_LINE];
        int third_check = 0;
        if (cmd_type == 2) {
            second_check = check_path(second_path, path_array, second_input_arg, num_paths);
            if (second_check == 0) {
                fprintf(stderr, "second command not found\n");
                continue;
            }
            if (two_pipes[0] == 0) {
                PP(file_path, input_arg, num_first_arg, second_path, second_input_arg, num_second_arg);
            }
            else if ((two_pipes[0]) == 1) {
                third_check = check_path(third_path, path_array, third_input_arg, num_paths);
                if (third_check == 0) {
                    fprintf(stderr, "third command not found\n");
                    continue;
                }
                PP3(file_path, input_arg, num_first_arg, second_path, second_input_arg, num_second_arg, third_path, third_input_arg, num_third_arg);
            }
        }

    }
}
