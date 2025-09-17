# OS_Assignment2
# Custom Shell with Command History and Pipelining

This project is a **custom shell implementation in C** that supports executing commands, handling pipelines, and maintaining a history of executed commands. It also includes signal handling to gracefully exit and display full command history when interrupted with `CTRL+C`.

---

## Features

- **Command Execution**  
  Executes system commands entered by the user.

- **Pipeline Support**  
  Handles commands with pipes (`|`), e.g.:
  ```bash
  cat file.c | grep main | wc -l
strip_newline → Removes trailing newline characters from input strings.

giveHistory → Prints only the list of commands executed.

givefullHistory → Prints detailed history including times, duration, and PID.

signal_handler → Handles SIGINT by showing history before exiting.

Main Loop:

Reads user input.

Parses commands and handles pipes.

Forks processes and executes each stage of the pipeline.

Records execution details (start/end time, duration, PID).

Stores successful commands into history.

Contributions

Isobel
Implemented command parsing and pipeline handling logic.
Worked on structuring argument arrays for piped commands.

Fixed issues(constructed an array of string array) and ensured commands were executed correctly with execvp.

Navrun
Developed command history storage and retrieval (history and full history).
Implemented signal handling (SIGINT) for graceful exits.
Added time-tracking for command execution (start time, end time, duration).
