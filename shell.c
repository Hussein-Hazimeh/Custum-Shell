#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define HISTORY_SIZE 10

char *history[HISTORY_SIZE];
int history_count = 0;

// Signal handler for SIGINT (Ctrl+C)
void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nmyshell> ");
        fflush(stdout);
    }
}

// Function to add a command to history
void add_to_history(char *command) {
    if (history_count < HISTORY_SIZE) {
        history[history_count++] = strdup(command);
    } else {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history[HISTORY_SIZE - 1] = strdup(command);
    }
}

// Function to print command history
void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

// Function to read input
char *read_input() {
    char *input = malloc(BUFFER_SIZE * sizeof(char));
    if (!input) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    fgets(input, BUFFER_SIZE, stdin);
    input[strcspn(input, "\n")] = '\0';  // Remove newline
    return input;
}

// Function to parse input into arguments
char **parse_input(char *input) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(input, " ");
    while (token != NULL) {
        tokens[position++] = token;
        token = strtok(NULL, " ");
    }
    tokens[position] = NULL;
    return tokens;
}

// Function to execute built-in 'cd' command
int cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "myshell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("myshell");
        }
    }
    return 1;
}

// Function to handle input/output redirection
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;  // Remove the redirection part from args
        } else if (strcmp(args[i], ">>") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("myshell");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

// Function to execute the command
void execute(char **args, int background) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        handle_redirection(args);
        if (execvp(args[0], args) == -1) {
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("myshell");
    } else {
        // Parent process
        if (!background) {
            waitpid(pid, &status, 0);  // Wait for foreground process
        } else {
            printf("Process running in background with PID: %d\n", pid);
        }
    }
}

// Function to check if the command should run in the background
int check_background(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        args[i - 1] = NULL;  // Remove '&' from the argument list
        return 1;  // Run in background
    }
    return 0;  // Run in foreground
}

int main() {
    char *input;
    char **args;
    int background;

    // Set up signal handling
    signal(SIGINT, handle_signal);

    while (1) {
        // Get current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        // Display the prompt
        printf("%s@myshell:%s> ", getenv("USER"), cwd);

        // Read input
        input = read_input();
        if (strlen(input) == 0) continue;

        // Add command to history
        add_to_history(input);

        // Parse input
        args = parse_input(input);

        // Handle "exit" command
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // Handle "cd" command
        if (strcmp(args[0], "cd") == 0) {
            cd(args);
        }

        // Handle "history" command
        else if (strcmp(args[0], "history") == 0) {
            print_history();
        }

        // Execute other commands
        else {
            background = check_background(args);
            execute(args, background);
        }

        // Free memory
        free(input);
        free(args);
    }

    return 0;
}