![C](https://img.shields.io/badge/C-Language-A8B9CC?logo=c&logoColor=black) 
![Linux](https://img.shields.io/badge/Linux-POSIX-FCC624?logo=linux&logoColor=black) 
![Shell](https://img.shields.io/badge/Shell-Unix_Programming-4EAA25) 
![Pipes](https://img.shields.io/badge/Pipes-%7C%20Processes-blue) 
![Signals](https://img.shields.io/badge/Signals-Process_Handling-red)
![Makefile](https://img.shields.io/badge/Build-Makefile-brightgreen) 
![Status](https://img.shields.io/badge/Status-Functional-success) 

# MiniShell – Implementation of a Simplified Shell

This project involves the development of a **MiniShell** in C that supports essential features of modern command-line environments.  
The shell implements command execution, pipes, redirections, background processes, and signal handling.

---

## Supported Features

### 1. `cd` Command

- Switching to existing directories  
- Error handling for non-existing directories or missing permissions  
- Default behavior (`cd` without arguments -> switch to the home directory)

**Example:**
```bash
$ cd /tmp
$ pwd
/tmp
$ cd xx
xx: No such file or directory
$ cd
$ pwd
/home/user
````

---

### 2. Command Chaining

Support for logical AND (`&&`) and logical OR (`||`) operators.

**Example:**

```bash
$ true && echo yay
yay
$ false || echo yay
yay
```

---

### 3. Redirections

* stdout redirection (`>`, `>>`)
* stdin redirection (`<`)
* Error handling for missing files or insufficient permissions

**Example:**

```bash
$ echo hello > f
$ cat f
hello
$ echo hello >> f
$ cat f
hello
hello
$ cat < xyz
xyz: No such file or directory
```

---

### 4. Pipelines

* Standard pipelines (e.g. `cat | cat | cat`)
* Proper waiting for all processes within a pipeline
* No deadlocks with full pipes
* Safe process termination using `CTRL+C`

**Example:**

```bash
$ ls -l | wc
$ cat /bin/bash | od -x | head -1
$ cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat
^C
$ status
```

---

### 5. Status Command (`status`)

* Display of return values
* Signal detection
* Management and cleanup of terminated background processes

**Example:**

```bash
$ ls -l xyz &
$ status
$ kill -9 <pid>
$ status
```

---

## Architecture & Code Structure

The implementation follows a modular architecture and includes:

* `command.c / command.h` – Command management (simple & compound)
* `parser.c` – Command-line parser
* `executor.c` – Execution including redirections, pipes, and background processes
* `list.c / list.h` – Custom list structure for process management
* `debug.c` – Optional debug output

---

## Build & Execution

Compile and run using the provided Makefile or the instructions in `README.txt`.

```bash
make
./minishell
```
Autor : DIMITRY NTOFEU NYATCHA
Fachhochschule: [THM]
Sprache: C

Lizenz / Verwendung
kein öffentlicher Wiedergebrauch ohne Rücksprache.


