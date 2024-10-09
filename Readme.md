# **Creating a Custom Shell in C: A Step-by-Step Tutorial**

Have you ever wondered how a shell like Bash or Zsh works behind the scenes? Shells are the command interpreters that allow us to interact with the operating system by running commands, managing processes, and automating tasks. In this article, I’ll walk you through the process of creating a simple custom shell in C.

## **What is a Shell?**

A shell is a command-line interface (CLI) that accepts user commands and interprets them into actions that the operating system can execute. Shells like Bash (Bourne Again SHell) and Zsh (Z Shell) provide a user-friendly way to interact with the system and control processes.
### **Why Build a Shell?**

1. **Understanding System Calls**: At the heart of any shell is its ability to communicate with the operating system through system calls. By developing our shell, we learn how to use these calls for tasks like process creation, input/output handling, and error management. This knowledge is crucial for understanding how programs interact with the OS.

2. **Learning Process Management**: The shell manages the execution of processes, which includes creating new processes, waiting for them to complete, and handling signals. Understanding this process management is vital for system-level programming and applications that require multitasking.

3. **Enhancing Problem-Solving Skills**: Building a shell is a complex task that involves various programming concepts such as memory management, string manipulation, and error handling. It challenges developers to think critically about how to handle input, parse commands, and execute them efficiently.

4. **Customization and Features**: A custom shell allows developers to implement features tailored to their needs, such as command history, input/output redirection, and job control for background processes. This flexibility enables users to enhance their productivity and customize their command-line environment.

5. **Foundation for Learning Advanced Topics**: By creating a simple shell, we lay the groundwork for more advanced topics in operating systems, such as job scheduling, pipelines, and process synchronization. These concepts are integral to modern computing and understanding them helps in tackling real-world system programming challenges.

Here’s an article you can use for your shell project, explaining the key components and the thought process behind its creation.

---


## **Getting Started with the Shell**

Sure! Let's dive deeper into the key sections of your shell code, explaining not just how each piece works, but why they are written that way and how they relate to the overall functionality of the shell.

### **1. Reading Input (`read_input()`)**

#### **Purpose**:
This function is responsible for reading the user input from the command line. It captures a command that the user types and stores it in memory for further processing.

#### **Code Breakdown**:
```c
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
```

- **Memory Allocation**:
    - `malloc(BUFFER_SIZE * sizeof(char))`: We allocate memory to store the user input. The size of this memory block is based on a constant `BUFFER_SIZE` (set to 1024 in this case). This is because we don't know in advance how long the user input will be, so we pre-allocate enough memory to handle typical input lengths.
    - If `malloc` fails (e.g., due to insufficient memory), it returns `NULL`, and we handle this by printing an error message and exiting the shell.

- **Reading the Input**:
    - `fgets(input, BUFFER_SIZE, stdin)`: This function reads a line of input from `stdin` (the terminal) into the `input` buffer, up to `BUFFER_SIZE` characters.
    - `input[strcspn(input, "\n")] = '\0';`: After capturing the input, we strip out the newline character (`'\n'`) that `fgets()` captures when the user presses `Enter`. This is necessary because a command like `ls` should not include the newline when it's being processed.

This function is simple but essential. Without capturing user input, the shell cannot interpret or execute any commands. The reason we use `fgets()` over functions like `scanf()` is that `fgets()` is safer—it ensures that input doesn't overflow the buffer by limiting the number of characters it reads.

---

### **2. Parsing Input (`parse_input()`)**

#### **Purpose**:
Once the input is read, it needs to be broken down into individual tokens or arguments. For example, if the user types `ls -l /home`, this function breaks it into three separate tokens: `ls`, `-l`, and `/home`. These tokens are used later to execute the command.

#### **Code Breakdown**:
```c
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
```

- **Tokenization**:
    - `strtok(input, " ")`: This function splits the input string into tokens using spaces (`" "`) as the delimiter. The first call returns the first token, and subsequent calls (inside the loop) return the next tokens.
    - Tokens are individual pieces of the input command, like `ls`, `-l`, `/home`.

- **Dynamic Array for Tokens**:
    - We allocate an array of strings (`tokens`) to hold each token from the input. The size of this array is initially set to `bufsize` (64). This size is chosen as an approximation for typical command length, but if commands are more complex, we might need to resize the array in future versions.

- **Null Termination**:
    - After all tokens are stored, we terminate the array with `tokens[position] = NULL`. This is important because functions like `execvp()` (which runs commands) expect the list of arguments to be NULL-terminated.

Command-line arguments are always separated by spaces, so breaking the input into individual tokens is crucial for processing the command. We use `strtok()` because it efficiently tokenizes strings in C. The `tokens` array allows us to store each part of the command (e.g., command name and its arguments) for further execution.

---

### **3. Handling Built-in Commands (`cd`, `history`)**

#### **Purpose**:
Some commands like `cd` (change directory) and `history` are built-in commands, meaning they don't need to be executed by creating a new process or calling external programs. The shell itself must handle them.

#### **`cd` Implementation**:
```c
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
```

- **Checking Arguments**:
    - The command `cd` requires one argument (the directory to change to). If the user doesn't provide an argument (i.e., `args[1]` is `NULL`), we display an error message.
    - `args[0]` is the command itself (`cd`), and `args[1]` should be the target directory.

- **Changing Directory**:
    - `chdir(args[1])`: This system call changes the current working directory to the one specified by `args[1]`. If the directory does not exist, `chdir()` returns `-1`, and we handle the error using `perror()`.

#### **`history` Implementation**:
```c
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

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}
```

- **Adding to History**:
    - Every time a user enters a command, we store it in a history buffer (with a fixed size of 10 commands). If the history buffer is full, the oldest command is removed, and new commands are added at the end.
    - This is done by shifting all previous commands up in the array and freeing the memory for the oldest command.

- **Printing History**:
    - The `print_history()` function prints all stored commands, displaying the history index (e.g., `1`, `2`, `3`) followed by the command itself.

- The `cd` command is unique because it changes the environment of the shell process itself (changing the current directory), which cannot be done by forking a child process. This is why `cd` is a built-in command.
- Managing history is a typical shell feature, allowing users to view previous commands and potentially repeat them.

---

### **4. Handling Input/Output Redirection (`handle_redirection()`)**

#### **Purpose**:
In Unix-like systems, users often want to redirect the input or output of a command. For instance, `ls > file.txt` writes the output of `ls` to `file.txt` instead of the console. Similarly, input redirection like `command < inputfile` reads from a file instead of the terminal.

#### **Code Breakdown**:
```c
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
            args[i] = NULL;
        } else if (strcmp(args[i], ">>") == 0) {
            // Similar handling for appending output
        } else if (strcmp(args[i], "<") == 0) {
            // Handling for input redirection
        }
    }
}
```

- **Opening File Descriptors**:
    - We use `open()` to open a file for writing or reading. The mode (`O_WRONLY | O_CREAT | O_TRUNC`) ensures that we write to the file, create it if it doesn't exist, and truncate it to zero length if it does.
    - If `open()` fails (e.g., the file can't be created), we handle the error with `perror()` and exit the shell.

- **Redirecting Output/Input**:
    - `dup2(fd, STDOUT_FILENO)`: This function redirects the file descriptor `fd` to standard output (file descriptor `1`). After this, any output that would have gone to the console will go to the file.
    - We close the file descriptor after the redirection is set up.

Redirection is a core feature of shells, making it convenient for users to save output to files or read from files as input. This implementation intercepts the standard input/output of commands and redirects it as needed.

---

### **5. Executing Commands (`execute()`)**

#### **Purpose**:
After parsing and handling redirection, the shell needs to execute the command using the `execvp()` system call, which runs the command by replacing the current process image with the new one (the command).

#### **Code Breakdown**:
```c
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
            waitpid(pid, &status, 0);
        } else {
            printf("Process running in background with PID: %d\n", pid);
        }
    }
}
```

- **Creating a Child Process**:
    - The `fork()` system call creates a new process. The parent process (the shell) creates a child that runs the command. The child process is an exact copy of the parent but with a different process ID.
    - In the child process (`pid == 0`), we handle redirection and then use `execvp()` to execute the command. If the command fails (e.g., it's not found), `execvp()` returns `-1`, and we print an error message.

- **Parent Process**:
    - In the parent process, the shell can either wait for the child to finish (`waitpid()`) or, if the command is running in the background, immediately return control to the user while the child runs in the background.

This part of the code manages the core functionality of executing external programs. We use `fork()` and `execvp()` because they allow for process isolation—if the child fails or exits, the parent shell continues to run independently.

---

### **6. Running Commands in the Background**

#### **Purpose**:
Background processes are those that don't block the shell. For instance, when a user appends `&` to a command (`sleep 10 &`), the shell should immediately return the prompt without waiting for the command to finish.

#### **Code Breakdown**:
```c
int check_background(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        args[i - 1] = NULL;  // Remove '&' from the argument list
        return 1;
    }
    return 0;
}
```

- **Detecting `&`**:
    - The function loops through the `args` array and checks if the last argument is `&`. If it is, this indicates that the user wants the command to run in the background.

- **Removing `&`**:
    - The `&` symbol doesn't need to be passed to `execvp()`, so it's removed from the argument list before the command is executed.

Background processes improve shell usability by allowing users to continue entering commands while long-running processes (e.g., file transfers or compilation) are executed in the background. This function ensures that background processes are handled correctly by the shell.

---

### **7. Signal Handling (`SIGINT`)**

#### **Purpose**:
When users press `Ctrl+C`, a signal (`SIGINT`) is sent to the shell process. By default, this signal terminates the shell. We override this behavior to ensure the shell doesn't exit and simply displays a new prompt.

#### **Code Breakdown**:
```c
void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\nmyshell> ");
        fflush(stdout);
    }
}
```

- **Signal Handler**:
    - This function is registered as a handler for the `SIGINT` signal. When `Ctrl+C` is pressed, the shell doesn't exit; instead, the signal is caught, and the prompt is re-displayed.
    - `fflush(stdout)` ensures that the prompt is immediately shown without any buffer delay.

Without signal handling, pressing `Ctrl+C` would terminate the shell entirely, which is undesirable. By intercepting this signal, we allow the shell to ignore `Ctrl+C` and keep running, giving the user a better experience.

---

### **8. Main Loop**

#### **Purpose**:
The main loop is the core of the shell's operation. It continuously displays the prompt, reads user input, and processes commands. This loop runs until the user types `exit`.

#### **Code Breakdown**:
```c
int main() {
    char *input;
    char **args;
    int background;

    signal(SIGINT, handle_signal);

    while (1) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s@myshell:%s> ", getenv("USER"), cwd);

        input = read_input();
        if (strlen(input) == 0) continue;

        add_to_history(input);
        args = parse_input(input);

        if (strcmp(args[0], "exit") == 0) {
            break;
        } else if (strcmp(args[0], "cd") == 0) {
            cd(args);
        } else if (strcmp(args[0], "history") == 0) {
            print_history();
        } else {
            background = check_background(args);
            execute(args, background);
        }

        free(input);
        free(args);
    }
    return 0;
}
```

- **Infinite Loop**:
    - The shell keeps running inside the `while(1)` loop, continuously reading user input and processing it until the user explicitly exits with the `exit` command.
    
- **Prompt Display**:
    - The prompt displays the current working directory (using `getcwd()`) and the username (using `getenv("USER")`).
    
- **Command Processing**:
    - After reading input, the shell first adds the command to the history and parses it into arguments. Then it checks if the command is `exit`, `cd`, or `history`. If it’s an external command, the shell executes it.

This loop is the heart of the shell. It ensures the shell is always ready to take new commands and processes them appropriately. The flow of execution (reading input, parsing it, checking for built-ins, and executing commands) forms the main operation of the shell.

---
## **Conclusion**

This shell project provides a basic understanding of how to build a command-line interface from scratch. We’ve implemented features like:
- Command execution with forking
- Built-in commands (`cd`, `history`)
- Input/output redirection
- Background processes
- Signal handling (`Ctrl+C`)

There’s a lot of room for further improvements, such as adding features like piping (`|`), tab completion, or even advanced job control. This project was a fun and educational way to explore systems programming and understand how real shells work!

You can find the full source code [here on GitHub](https://github.com/Hussein-Hazimeh/Custum-Shell).

---
