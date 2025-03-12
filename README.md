# Custom-Command-Line-Shell

This project is a basic shell implemented in C that allows users to run commands in both foreground and background, manage command history, and utilize basic shell commands.

## Features

### Command Execution
- **Foreground Execution**: Commands run in the foreground, with the shell waiting until the process completes.
- **Background Execution**: Commands appended with `&` run in the background, allowing the shell to continue without waiting for completion.

### Internal Commands
- **exit**: Exits the shell, provided no arguments are given.
- **pwd**: Prints the current working directory.
- **cd**: Changes the current directory with support for:
  - Home directory (`~`)
  - Previous directory (`-`)
- **help**: Displays help for internal commands or lists all commands if no specific command is given.

### Command History
- Stores the last 10 commands entered, accessible via:
  - **`history`**: Lists the most recent commands.
  - **`!!`**: Repeats the last command.
  - **`!<n>`**: Runs the command at position `<n>` in the history.
- Commands run with `!` are also added to the history.

### Signal Handling
- **SIGINT (CTRL-C)**: Captures `CTRL-C` signals, displaying help information instead of exiting the shell.

### Error Handling
Displays specific error messages when:
- **Invalid commands** are entered.
- **Too many arguments** are given for internal commands.
- **Fork/exec failures** occur, displaying appropriate error messages.

### Memory Management
The shell includes memory cleanup for command tokens and history.

### Compile
```
cmake -S . -B build
```
```
cmake --build build
```

### Run
```
./shell
```
```
./shell_test
```


### Example commands
$ pwd               # Prints current directory
$ cd ~              # Changes to home directory
$ ls &              # Runs `ls` in the background
$ history           # Displays the last 10 commands
$ !!                # Re-runs the last command
$ !5                # Runs command with history index 5

