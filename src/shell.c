#define _POSIX_C_SOURCE 200809L

#include "../include/msgs.h"

#include <ctype.h>
#include <errno.h>
#include <linux/limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAX_LEN 10
#define FORMAT_MSG(cmd, msg) cmd ": " msg "\n"
#define FORMAT_HISTORY(num, cmd) num "\t" cmd "\n"

// format the messages
const char *shellCDunable = FORMAT_MSG("cd", CHDIR_ERROR_MSG);
const char *shellCDtooMany = FORMAT_MSG("cd", TMA_MSG);
const char *shellPWDunable = FORMAT_MSG("pwd", GETCWD_ERROR_MSG);
const char *shellPWDtooMany = FORMAT_MSG("pwd", TMA_MSG);
const char *shellREAD = FORMAT_MSG("shell", READ_ERROR_MSG);
const char *shellFORK = FORMAT_MSG("shell", FORK_ERROR_MSG);
const char *shellEXEC = FORMAT_MSG("shell", EXEC_ERROR_MSG);
const char *shellWAIT = FORMAT_MSG("shell", WAIT_ERROR_MSG);
const char *shellEXIT = FORMAT_MSG("exit", TMA_MSG);
const char *helpTooMany = FORMAT_MSG("help", TMA_MSG);
const char *helpCD = FORMAT_MSG("cd", CD_HELP_MSG);
const char *helpEXIT = FORMAT_MSG("exit", EXIT_HELP_MSG);
const char *helpHELP = FORMAT_MSG("help", HELP_HELP_MSG);
const char *helpPWD = FORMAT_MSG("pwd", PWD_HELP_MSG);
const char *helpHISTORY = FORMAT_MSG("history", HISTORY_HELP_MSG);
const char *historyInvalid = FORMAT_MSG("history", HISTORY_INVALID_MSG);
const char *historyNO = FORMAT_MSG("history", HISTORY_NO_LAST_MSG);

int counter_history[MAX_LEN];
char *input_history[MAX_LEN];
int history_count = 0;
char *tokenArray[50];
char prevCD[PATH_MAX] = "";
int cCount = 0;
void add_to_command_history(char *input);
void remove_oldest_command();
void print_command_history();
char *get_user_input();
void chdirNoArg();
void cwd();
void pwd();
void executeCommand(char *input);
void cleanZombies();
char *getLastCommand();
char *getNCommand(int n);
char *input;
void cleanInpHist();
void handle_sigint(int signum) {

  write(STDOUT_FILENO, "\n", 2);
  write(STDOUT_FILENO, helpHELP, strlen(helpHELP));
  write(STDOUT_FILENO, helpEXIT, strlen(helpEXIT));
  write(STDOUT_FILENO, helpCD, strlen(helpCD));
  write(STDOUT_FILENO, helpPWD, strlen(helpPWD));
  write(STDOUT_FILENO, helpHISTORY, strlen(helpHISTORY));
}
int main() {

  struct sigaction handler;

  handler.sa_handler = handle_sigint;
  handler.sa_flags = 0;
  sigemptyset(&handler.sa_mask);

  if (sigaction(SIGINT, &handler, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  while (1) {

    if (errno == EINTR) {

      errno = 0;
      continue;
    }
    cwd();
    input = get_user_input();
    if (input == NULL) {
      // printf("input fail 37\n");
      //
      // free(input);
      continue;
    }

    if (strcmp(input, "!!") == 0) {
      if (cCount == 0) {
        // printf("input fail 37\n");
        //
        write(STDERR_FILENO, historyNO, strlen(historyNO));

        free(input);
        continue;
      }

      free(input);
      input = getLastCommand();
      write(STDOUT_FILENO, input, strlen(input));
      write(STDOUT_FILENO, "\n", strlen("\n"));
    }

    if (input[0] == '!' && isdigit(input[1])) {

      if (cCount == 0) {
        // printf("input fail 37\n");
        //
        write(STDERR_FILENO, historyNO, strlen(historyNO));

        free(input);

        continue;
      }

      // write(STDOUT_FILENO, "HERE", strlen("HERE"));

      // printf("%d\n\n", cCount);
      // printf("%d\n\n", input[1]);

      if (atoi(input + 1) < cCount && atoi(input + 1) > cCount - 10) {
        input = input_history[atoi(input + 1) % 10];

        // write(STDOUT_FILENO, "HERE1", strlen("HERE1"));
        write(STDOUT_FILENO, input, strlen(input));
        write(STDOUT_FILENO, "\n", strlen("\n"));
      } else {
        write(STDERR_FILENO, historyInvalid, strlen(historyInvalid));
        // write(STDOUT_FILENO, "HERE2", strlen("HERE2"));
        free(input);
        continue;
      }
    } else if (input[0] == '!' && !isdigit(input[1])) {
      write(STDERR_FILENO, historyInvalid, strlen(historyInvalid));

      free(input);
      continue;
    }

    add_to_command_history(input);
    executeCommand(input);
    if (strcmp(input, "history") == 0) {
      print_command_history();
    }
  }

  // need to free input history
  // for (int i = 0; i < MAX_LEN; i++) {
  //  free(input_history[i]);
  //}

  return 0;
}

void cwd() {
  char current_working_directory[PATH_MAX];
  if (getcwd(current_working_directory, sizeof(current_working_directory)) ==
      NULL) {
    write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
  } else {
    // printf("%s$ ", current_working_directory);
    write(STDOUT_FILENO, current_working_directory,
          strlen(current_working_directory));
    write(STDOUT_FILENO, "$ ", strlen("$ "));
  }
}

void pwd() {
  char current_working_directory[PATH_MAX];
  if (getcwd(current_working_directory, sizeof(current_working_directory)) ==
      NULL) {
    write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
  } else {
    // printf("%s$ ", current_working_directory);
    write(STDOUT_FILENO, current_working_directory,
          strlen(current_working_directory));
    write(STDOUT_FILENO, "\n", strlen("\n"));
  }
}

char *getLastCommand() {
  int lastIndex = history_count - 1;
  if (lastIndex < 0) {
    write(STDERR_FILENO, historyNO, strlen(historyNO));
    return NULL;
  }

  return input_history[lastIndex];
}

char *getNCommand(int n) {
  if (n <= cCount && n >= cCount - 10) {

  } else {
  }
  return input_history[n % 10];
}

char *get_user_input() {
  char *buffer = malloc(1024 * sizeof(char));
  if (buffer == NULL) {
    write(STDERR_FILENO, shellREAD, strlen(shellREAD));
    return NULL;
  }

  // read user input
  int len = read(STDIN_FILENO, buffer, (1024 * sizeof(char)));
  if (errno == EINTR) {

    errno = 0;
    free(buffer);
    return NULL;
  }

  // write(STDERR_FILENO, "get user input 1", strlen("get user input 1"));

  // read error
  if (len == -1) {
    free(buffer);
    write(STDERR_FILENO, shellREAD, strlen(shellREAD));
    return NULL;
  }

  // write(STDERR_FILENO, "get user input 2", strlen("get user input 2"));
  if (len == 0) {
    buffer[0] = '\0';
    return buffer;
  }

  // write(STDERR_FILENO, "get user input 3", strlen("get user input 3"));
  //  account for empty string
  buffer[len] = '\0';
  // remove trailing null term
  if (len > 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = '\0';
  }

  return buffer;
}

void chdirNoArg() {
  uid_t id = getuid();
  struct passwd *pw = getpwuid(id);
  if (pw == NULL) {
    write(STDERR_FILENO, "cd: error user", 14);
  }

  if (chdir(pw->pw_dir) == -1) {
    write(STDERR_FILENO, shellCDunable, strlen(shellCDunable));
  }
}

void executeCommand(char *input) {

  if (input == NULL) {
    return;
  }
  char *inputCopy = strdup(input);
  if (inputCopy == NULL) {
    free(inputCopy);
    write(STDERR_FILENO, shellREAD, strlen(shellREAD));
    return;
  }
  // tokenize the input
  char *token = NULL;
  char *saveptr;
  char *argv[100];
  int argCounter = 0;
  int isBackground = 0;

  // init arg array
  //
  if (strlen(inputCopy) < 1) {

    free(inputCopy);
    return;
  }

  if (inputCopy[strlen(inputCopy) - 1] == '&') {
    inputCopy[strlen(inputCopy) - 1] = ' ';
    isBackground = 1;
  }
  token = strtok_r(inputCopy, " ", &saveptr);
  while (token != NULL) {
    // check for &

    if (strcmp(token, "&") == 0) {

      isBackground = 1;
    } else {
      argv[argCounter] = token;
      argCounter++;
    }

    token = strtok_r(NULL, " ", &saveptr);
  }
  argv[argCounter] = NULL;

  if (argCounter == 0) {
    free(inputCopy);
    return; // no command dont fork
  }

  if (strcmp(argv[0], "!!") == 0) {

    if (argCounter == 1) {

      if (cCount == 0) {
        // print error
      }
      char *inp = getLastCommand();
      if (inp == NULL) {
        write(STDERR_FILENO, shellREAD, strlen(shellREAD));

        return;
      } else {
        executeCommand(getLastCommand());
      }
      free(inputCopy);
      return;
    } else // too many args
    {
      write(STDERR_FILENO, historyInvalid, strlen(historyInvalid));
    }

    free(inputCopy);
    return;
  }

  // history
  if (strcmp(argv[0], "history") == 0) {
    if (argCounter == 1) {
      free(inputCopy);
      return;
    } else // too many args
    {
      write(STDERR_FILENO, historyInvalid, strlen(historyInvalid));
    }

    free(inputCopy);
    return;
  }

  // internal
  // exit
  //
  // printf("exit to be presssed\n");
  if (strcmp(argv[0], "exit") == 0) {
    // printf("exit pressed\n");
    if (argCounter == 1) {
      // printf("exit pressed once");
      //   if(kill(getppid(), SIGINT) == -1)
      //{
      //
      //
      //   cleanInpHist();
      free(inputCopy);
      exit(0);
      //}
    } else {
      write(STDERR_FILENO, shellEXIT, strlen(shellEXIT));
    }

    free(inputCopy);
    return;
  }

  // pwd
  //
  // printf("exit to be presssed\n");
  if (strcmp(argv[0], "pwd") == 0) {
    // printf("exit pressed\n");
    if (argCounter == 1) {
      pwd();

      free(inputCopy);
      return;
    } else {
      write(STDERR_FILENO, shellPWDtooMany, strlen(shellPWDtooMany));
    }

    free(inputCopy);
    return;
  }

  // help
  if (strcmp(argv[0], "help") == 0) {
    if (argCounter == 1) {
      // list all

      write(STDOUT_FILENO, helpHELP, strlen(helpHELP));
      write(STDOUT_FILENO, helpEXIT, strlen(helpEXIT));
      write(STDOUT_FILENO, helpCD, strlen(helpCD));
      write(STDOUT_FILENO, helpPWD, strlen(helpPWD));
      write(STDOUT_FILENO, helpHISTORY, strlen(helpHISTORY));

    } else if (argCounter == 2) {
      if (strcmp(argv[1], "help") == 0) {
        write(STDOUT_FILENO, helpHELP, strlen(helpHELP));
      } else if (strcmp(argv[1], "exit") == 0) {
        write(STDOUT_FILENO, helpEXIT, strlen(helpEXIT));

      } else if (strcmp(argv[1], "cd") == 0) {
        write(STDOUT_FILENO, helpCD, strlen(helpCD));

      } else if (strcmp(argv[1], "pwd") == 0) {
        write(STDOUT_FILENO, helpPWD, strlen(helpPWD));

      } else if (strcmp(argv[1], "history") == 0) {
        write(STDOUT_FILENO, helpHISTORY, strlen(helpHISTORY));

      } else {

        write(STDOUT_FILENO, argv[1], strlen(argv[1]));
        write(STDOUT_FILENO, ": ", 2);
        write(STDOUT_FILENO, EXTERN_HELP_MSG, strlen(EXTERN_HELP_MSG));
      }

    } else {
      write(STDERR_FILENO, helpTooMany, strlen(helpTooMany));
    }

    free(inputCopy);
    return;
  }

  // cd
  if (strcmp(argv[0], "cd") == 0) {
    printf("cd pressed\n");
    if (argCounter == 1) {
      char cDir[PATH_MAX];
      if (getcwd(cDir, sizeof(cDir)) == NULL) {
        write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
      } else {
        strcpy(prevCD, cDir);
        chdirNoArg();
      }

      // cd hhome
      free(inputCopy);
      return;
    } else if (argCounter == 2) { // cd to home
      if (strcmp(argv[1], "~") == 0) {

        char cDir[PATH_MAX];
        if (getcwd(cDir, sizeof(cDir)) == NULL) {
          write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
        } else {
          strcpy(prevCD, cDir);
          chdirNoArg();
        }

        // cd hhome
        free(inputCopy);
        return;

      } else if (argv[1][0] == '~') {

        char cDir[PATH_MAX];
        if (getcwd(cDir, sizeof(cDir)) == NULL) {
          write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
        } else {
          strcpy(prevCD, cDir);
        }
        chdirNoArg();
        if (chdir(argv[1] + 2) == -1) {
          write(STDERR_FILENO, shellCDunable, strlen(shellCDunable));
        }

        free(inputCopy);
        return;
        // normal cd
      } else if (strcmp(argv[1], "-") == 0) {

        printf("CD - : \n");
        printf("prevCD: %s\n", prevCD);

        if (chdir(prevCD) == -1) {
          write(STDERR_FILENO, shellCDunable, strlen(shellCDunable));
        }

        char cDir[PATH_MAX];
        if (getcwd(cDir, sizeof(cDir)) == NULL) {
          write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
        } else {
          strcpy(prevCD, cDir);
        }

        free(inputCopy);
        return;
        //
        //
      }

      char cDir[PATH_MAX];
      if (getcwd(cDir, sizeof(cDir)) == NULL) {
        write(STDERR_FILENO, shellPWDunable, strlen(shellPWDunable));
      } else {
        strcpy(prevCD, cDir);
      }

      if (chdir(argv[1]) == -1) {

        write(STDERR_FILENO, shellCDunable, strlen(shellCDunable));
        // last cd
        //
      }
      free(inputCopy);
      return;
      // else if()
      //  write(STDERR_FILENO, shellCDtooMany, strlen(shellCDtooMany));
      //  cd too many
    } else {

      write(STDERR_FILENO, shellCDtooMany, strlen(shellCDtooMany));
    }

    free(inputCopy);
    return;
  }

  // signal handler
  //

  // external
  //
  pid_t pid = fork();

  // parent process waits for child
  // need to implement background and forground
  if (pid > 0) {
    int wstatus = 0;
    if (isBackground == 0) {

      if (waitpid(pid, &wstatus, 0) == -1) {
        write(STDERR_FILENO, shellWAIT, strlen(shellWAIT));
        free(inputCopy);
        return;
      }

    } else {
      // dont wait
      //
      while ((waitpid(-1, &wstatus, WNOHANG))) {
      }
    }

    // printf("child 149\n");
    if (WIFEXITED(wstatus)) {
      // printf("Child done. \n");

    } else {
      printf("%d\n", WEXITSTATUS(wstatus));
    }
    // child process
  } else if (pid == 0) {
    // printf("child 156\n");
    if (execvp(argv[0], argv) == -1) {

      write(STDERR_FILENO, shellEXEC, strlen(shellEXEC));

      free(inputCopy);
      exit(EXIT_FAILURE);
    }
  } else { // fork fail
    write(STDERR_FILENO, shellFORK, strlen(shellFORK));
  }

  free(inputCopy);
  return;
}

void add_to_command_history(char *input) {
  if (strcmp(input, "!!") == 0) {

    return;
  } else {

    if (input == NULL) {

    } else {
      if (history_count >= MAX_LEN && input_history[0] != NULL) {
        remove_oldest_command();
      }
      counter_history[history_count] = cCount;

      input_history[history_count] = input;
      history_count++;
      cCount++;
    }
  }
}

void remove_oldest_command() {
  if (history_count > 0) {
    // free the mem of the oldest record
    if (input_history[0] == NULL) {

    } else {

      free(input_history[0]);
      input_history[0] = NULL;
    }
    for (int i = 1; i < history_count; i++) {
      input_history[i - 1] = input_history[i];
      counter_history[i - 1] = counter_history[i];
    }
    history_count--;
  }
}

void cleanInpHist() {
  for (int i = 0; i < history_count; i++) {
    free(input_history[i]);
    input_history[i] = NULL;
  }
  history_count = 0;
}

void print_command_history() {
  for (int i = history_count - 1; i >= 0; i--) {
    // printf("%s\n", input_history[i]);
    char buffer[24];

    int len = snprintf(buffer, sizeof(buffer), "%d", counter_history[i]);
    if (len < 0) {
      perror("printCommandHistory");
      exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, buffer, len);
    write(STDOUT_FILENO, "\t", strlen("\t"));
    if (input_history[i] == NULL) {

    } else {
      write(STDOUT_FILENO, input_history[i], strlen(input_history[i]));
    }

    write(STDOUT_FILENO, "\n", strlen("\n"));
    // char *msg = FORMAT_HISTORY(buffer, input_history[i]);

    // const char *w = " ";
    // const char *msg = FORMAT_HISTORY(w, w);
    // write(STDOUT_FILENO, msg, strlen(msg));
  }
}
