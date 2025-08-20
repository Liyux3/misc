/*
 * Student name: Yuxian LI
 * Student No.: 3036100349
 * Development platform: Docker on Mac OS
 * Remark: All features completed
 *   - Process creation and execution (4.5 pts)
 *   - Pipe handling up to 4 pipes (4 pts)
 *   - SIGINT signal handling (2 pts)
 *   - Watch command with /proc monitoring (4 pts)
 *   - Exit command (0.5 pts)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 30


// Tokenize input by spaces, return number of tokens
int tokenize(char *input, char **tokens) {
    int count = 0;
    char *token = strtok(input, " \t");

    while (token != NULL && count < MAX_TOKENS) {
        tokens[count++] = token;
        token = strtok(NULL, " \t");
    }
    tokens[count] = NULL;  // exec functions need NULL-terminated array

    return count;
}

// Find executable in PATH, return 1 if found, 0 otherwise
// If found, full_path contains the complete path
int find_in_path(char *cmd, char *full_path) {
    // If command contains '/', it's already a path
    if (strchr(cmd, '/') != NULL) {
        strcpy(full_path, cmd);
        return (access(full_path, X_OK) == 0);
    }

    // Search in PATH
    char *path_env = getenv("PATH");
    if (path_env == NULL) return 0;

    char path_copy[4096];
    strcpy(path_copy, path_env);

    char *dir = strtok(path_copy, ":");
    while (dir != NULL) {
        snprintf(full_path, 1024, "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0) {
            return 1;
        }
        dir = strtok(NULL, ":");
    }

    return 0;
}

// Execute a single command, return child pid
void execute_command(char **tokens) {
    char full_path[1024];

    // Find the executable
    if (!find_in_path(tokens[0], full_path)) {
        // Check errno for specific error
        if (access(tokens[0], F_OK) == 0) {
            // File exists but not executable
            printf("3230yash: '%s': Permission denied\n", tokens[0]);
        } else {
            printf("3230yash: '%s': No such file or directory\n", tokens[0]);
        }
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process
        execv(full_path, tokens);
        // If exec returns, it failed
        perror("execv failed");
        exit(1);
    } else {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);

        // Check how child terminated
        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGINT) {
                printf("%s: Interrupt\n", tokens[0]);
            } else if (sig == SIGKILL) {
                printf("%s: Killed\n", tokens[0]);
            } else if (sig == SIGSEGV) {
                printf("%s: Segmentation fault\n", tokens[0]);
            } else {
                printf("%s: Terminated by signal %d\n", tokens[0], sig);
            }
        }
    }
}

// SIGINT handler for parent shell
void sigint_handler(int sig) {
}

// Check if pipe usage is valid
// Returns 0 if valid, -1 if invalid
int validate_pipes(char **tokens, int count) {
    // Check first token
    if (strcmp(tokens[0], "|") == 0) {
        printf("3230yash: Incorrect pipe sequence\n");
        return -1;
    }

    // Check last token
    if (strcmp(tokens[count-1], "|") == 0) {
        printf("3230yash: Incorrect pipe sequence\n");
        return -1;
    }

    // Check for consecutive pipes
    for (int i = 0; i < count - 1; i++) {
        if (strcmp(tokens[i], "|") == 0 && strcmp(tokens[i+1], "|") == 0) {
            printf("3230yash: should not have two consecutive | without in-between command\n");
            return -1;
        }
    }

    return 0;
}

// Split tokens into separate commands by |
// Returns number of commands
int split_by_pipe(char **tokens, int count, char ***commands) {
    int cmd_count = 0;
    int start = 0;

    commands[cmd_count] = &tokens[start];

    for (int i = 0; i < count; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            tokens[i] = NULL;  // Terminate previous command
            cmd_count++;
            commands[cmd_count] = &tokens[i+1];
        }
    }

    return cmd_count + 1;  // Total commands = number of pipes + 1
}


// Execute piped commands
void execute_pipeline(char ***commands, int cmd_count) {
    int pipes[cmd_count-1][2];  // Need (n-1) pipes for n commands
    pid_t pids[cmd_count];
    char full_path[1024];

    // Create all pipes
    for (int i = 0; i < cmd_count - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe failed");
            return;
        }
    }

    // Fork and exec each command
    for (int i = 0; i < cmd_count; i++) {
        // Check if command exists first
        if (!find_in_path(commands[i][0], full_path)) {
            if (access(commands[i][0], F_OK) == 0) {
                printf("3230yash: '%s': Permission denied\n", commands[i][0]);
            } else {
                printf("3230yash: '%s': No such file or directory\n", commands[i][0]);
            }
            // Clean up and return
            // Kill already forked children
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGKILL);
                waitpid(pids[j], NULL, 0);  // Reap zombie
            }

            // Close all pipes
            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            return;
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork failed");
            return;
        }

        if (pids[i] == 0) {
            // Child process

            // If not first command, redirect stdin from previous pipe
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // If not last command, redirect stdout to next pipe
            if (i < cmd_count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe fds in child
            for (int j = 0; j < cmd_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execv(full_path, commands[i]);
            perror("execv failed");
            exit(1);
        }
    }

    // Parent: close all pipe fds
    for (int i = 0; i < cmd_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < cmd_count; i++) {
        int status;
        waitpid(pids[i], &status, 0);

        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGINT) {
                printf("%s: Interrupt\n", commands[i][0]);
            } else if (sig == SIGKILL) {
                printf("%s: Killed\n", commands[i][0]);
            } else if (sig == SIGSEGV) {
                printf("%s: Segmentation fault\n", commands[i][0]);
            }
        }
    }
}



typedef struct {
    char state;
    int processor;
    unsigned long utime;
    unsigned long stime;
    unsigned long vsize;
    unsigned long minflt;
    unsigned long majflt;
} proc_stat_t;

// Read /proc/{pid}/stat and parse relevant fields
int read_proc_stat(pid_t pid, proc_stat_t *pstat) {
    char path[256];
    char buffer[2048];

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    if (fgets(buffer, sizeof(buffer), f) == NULL) {
        fclose(f);
        return -1;
    }
    fclose(f);

    // Parse the stat file
    // Format: pid (comm) state ppid pgrp session tty_nr tpgid flags minflt cminflt majflt cmajflt utime stime ...
    char *p = strrchr(buffer, ')');  // Find last ) to skip comm field which may contain spaces
    if (!p) return -1;
    p += 2;  // Skip ") "

    unsigned long cminflt, cmajflt, cutime, cstime;
    long priority, nice, num_threads, itrealvalue;
    unsigned long long starttime;
    long rss;
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned int flags;

    sscanf(p, "%c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %llu %lu %ld",
           &pstat->state,
           &ppid, &pgrp, &session, &tty_nr, &tpgid,
           &flags,
           &pstat->minflt, &cminflt,
           &pstat->majflt, &cmajflt,
           &pstat->utime, &pstat->stime,
           &cutime, &cstime,
           &priority, &nice, &num_threads, &itrealvalue,
           &starttime,
           &pstat->vsize, &rss);

    // Reopen and parse fully
    f = fopen(path, "r");
    if (!f) return -1;
    fgets(buffer, sizeof(buffer), f);
    fclose(f);

    p = strrchr(buffer, ')');
    p += 2;

    int field_count = 2;  // Already past pid and comm
    char *token = strtok(p, " ");

    while (token && field_count < 39) {
        if (field_count == 38) {
            pstat->processor = atoi(token);
        }
        token = strtok(NULL, " ");
        field_count++;
    }
    if (field_count == 38 && token) {
        pstat->processor = atoi(token);
    }

    return 0;
}

void execute_watch(char **tokens) {
    char full_path[1024];

    // tokens[0] is "watch", tokens[1] is the actual command
    if (!find_in_path(tokens[1], full_path)) {
        if (access(tokens[1], F_OK) == 0) {
            printf("3230yash: '%s': Permission denied\n", tokens[1]);
        } else {
            printf("3230yash: '%s': No such file or directory\n", tokens[1]);
        }
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child: execute the command
        // Shift tokens to remove "watch"
        execv(full_path, &tokens[1]);
        perror("execv failed");
        exit(1);
    } else {
        // Parent: monitor the child
        proc_stat_t stats[1000];  // Store up to 1000 snapshots
        int stat_count = 0;
        int status;
        long clock_ticks = sysconf(_SC_CLK_TCK);  // Get clock ticks per second

        // Small delay to let child start
        usleep(10000);  // 10ms

        // Monitor loop
        while (1) {
            if (read_proc_stat(pid, &stats[stat_count]) == 0) {
                stat_count++;
            }

            // Check if child terminated (non-blocking)
            pid_t result = waitpid(pid, &status, WNOHANG);

            if (result == pid) {
                // Child terminated
                break;
            } else if (result < 0) {
                perror("waitpid failed");
                break;
            }

            // Wait 500ms
            usleep(500000);
        }

        // Print header once
        printf("\n%-5s %-5s %-5s %-5s %-9s %-6s %s\n",
               "STATE", "CPUID", "UTIME", "STIME", "VSIZE", "MINFLT", "MAJFLT");

        // Print all collected stats
        for (int i = 0; i < stat_count; i++) {
            char state_str[16];
            switch(stats[i].state) {
                case 'R': strcpy(state_str, "R"); break;
                case 'S': strcpy(state_str, "S"); break;
                case 'D': strcpy(state_str, "D"); break;
                case 'Z': strcpy(state_str, "Z"); break;
                case 'T': strcpy(state_str, "T"); break;
                case 't': strcpy(state_str, "t"); break;
                case 'W': strcpy(state_str, "W"); break;
                case 'X': strcpy(state_str, "X"); break;
                default: sprintf(state_str, "%c", stats[i].state); break;
            }

            printf("%-5s %-5d %-5.2f %-5.2f %-9lu %-6lu %lu\n",
                   state_str,
                   stats[i].processor,
                   (double)stats[i].utime / clock_ticks,
                   (double)stats[i].stime / clock_ticks,
                   stats[i].vsize,
                   stats[i].minflt,
                   stats[i].majflt);
        }

        // Check termination status
        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGINT) {
                printf("%s: Interrupt\n", tokens[1]);
            } else if (sig == SIGKILL) {
                printf("%s: Killed\n", tokens[1]);
            }
        }
    }
}


int main() {
    char input[MAX_INPUT];
    char *tokens[MAX_TOKENS + 1];
    int token_count;

    // Install SIGINT handler for the shell
    signal(SIGINT, sigint_handler);

    while (1) {
        printf("## 3230yash >> ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            if (feof(stdin)) {
                break;  // Real EOF (Ctrl-D)
            }
            // Interrupted by signal, continue
            clearerr(stdin);
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            continue;
        }

        // Tokenize
        token_count = tokenize(input, tokens);

        if (token_count == 0) {
            continue;
        }

        // 1. Check for exit command
        if (strcmp(tokens[0], "exit") == 0) {
            if (token_count == 1) {
                printf("3230yash: Terminated\n");
                break;
            } else {
                printf("3230yash: \"exit\" with other arguments!!!\n");
                continue;
            }
        }

        // 2. Check for watch
        if (strcmp(tokens[0], "watch") == 0) {
            if (token_count < 2) {
                printf("3230yash: watch requires a command\n");
                continue;
            }

            // Check if watch command contains pipes
            int has_pipe = 0;
            for (int i = 1; i < token_count; i++) {
                if (strcmp(tokens[i], "|") == 0) {
                    has_pipe = 1;
                    break;
                }
            }

            if (has_pipe) {
                printf("3230yash: Cannot watch a pipe sequence\n");
                continue;
            }

            execute_watch(tokens);
            continue;  // Important: skip the rest
        }

        // Count pipes
        int pipe_count = 0;
        for (int i = 0; i < token_count; i++) {
            if (strcmp(tokens[i], "|") == 0) {
                pipe_count++;
            }
        }

        if (pipe_count > 0) {
            // Validate pipes
            if (validate_pipes(tokens, token_count) < 0) {
                continue;
            }

            // Split and execute pipeline
            char **commands[pipe_count + 1];
            int cmd_count = split_by_pipe(tokens, token_count, commands);
            execute_pipeline(commands, cmd_count);
        } else {
            // Single command
            execute_command(tokens);
        }
    }

    return 0;
}