#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "shell.h"
#include "helper.h"
#include "command.h"
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "statuslist.h"
#include "debug.h"
#include "execute.h"

/* do not modify this */
#ifndef NOLIBREADLINE
#include <readline/history.h>
#endif /* NOLIBREADLINE */

extern int shell_pid; // PID der Hauptshell
extern int fdtty; // Dateideskriptor des Terminals, verwendet zur Verwaltung von Prozessgruppen (tcsetpgrp)

/* do not modify this */
#ifndef NOLIBREADLINE
static int builtin_hist(char ** command){ // Zeigt die bisherigen Befehle an, wenn "hist" eingegeben wird.
    register HIST_ENTRY **the_list;
    register int i;
    printf("--- History --- \n");

    the_list = history_list ();
    if (the_list)
        for (i = 0; the_list[i]; i++)
            printf ("%d: %s\n", i + history_base, the_list[i]->line);
    else {
        printf("history could not be found!\n");
    }

    printf("--------------- \n");
    return 0;
}
#endif /*NOLIBREADLINE*/

/* Entfernt Anführungszeichen um ein Wort, z. B. "hello" wird zu hello */
void unquote(char * s){
	if (s!=NULL){
		if (s[0] == '"' && s[strlen(s)-1] == '"'){
	        char * buffer = calloc(sizeof(char), strlen(s) + 1);
			strcpy(buffer, s);
			strncpy(s, buffer+1, strlen(buffer)-2);
            s[strlen(s)-2]='\0';
			free(buffer);
		}
	}
}

/* Wendet unquote auf jedes Wort des Befehls an */
void unquote_command_tokens(char ** tokens){
    int i=0;
    while(tokens[i] != NULL) {
        unquote(tokens[i]);
        i++;
    }
}

/* Entfernt Anführungszeichen in Dateinamen von Umleitungen ("out.txt" → out.txt) */
void unquote_redirect_filenames(List *redirections){
    List *lst = redirections;
    while (lst != NULL) {
        Redirection *redirection = (Redirection *)lst->head;
        if (redirection->r_type == R_FILE) {
            unquote(redirection->u.r_file);
        }
        lst = lst->tail;
    }
}

/* Führt das Unquoting auf den gesamten Befehl aus, unabhängig von seiner Form (einfach oder Liste) */
void unquote_command(Command *cmd){
    List *lst = NULL;
    switch (cmd->command_type) {
        case C_SIMPLE:
        case C_OR:
        case C_AND:
        case C_PIPE:
        case C_SEQUENCE:
            lst = cmd->command_sequence->command_list;
            while (lst != NULL) {
                SimpleCommand *cmd_s = (SimpleCommand *)lst->head;
                unquote_command_tokens(cmd_s->command_tokens);
                unquote_redirect_filenames(cmd_s->redirections);
                lst = lst->tail;
            }
            break;
        case C_EMPTY:
        default:
            break;
    }
}

/* Holt sich die Tokens des Befehls (z. B. ls, -l) und startet fork(): */
static int execute_fork(SimpleCommand *cmd_s, int background) {
    char **command = cmd_s->command_tokens;
    pid_t pid;

    pid = fork();

    if (pid == 0) {
        // ==== KINDPROZESS ====
        signal(SIGINT, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        // === UMLEITUNGEN ===
        List *redirige = cmd_s->redirections;
        while (redirige != NULL) {
            Redirection *redir = (Redirection *)redirige->head;
            int fd;

            if (redir->r_type == R_FILE) {
                if (redir->r_mode == M_WRITE)
                    fd = open(redir->u.r_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                else if (redir->r_mode == M_APPEND)
                    fd = open(redir->u.r_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else // M_READ
                    fd = open(redir->u.r_file, O_RDONLY);

                if (fd < 0) {
                    fprintf(stderr, "%s: %s: %s\n", command[0], redir->u.r_file, strerror(errno));
                    exit(1);
                }

                if (redir->r_mode == M_READ)
                    dup2(fd, STDIN_FILENO);
                else
                    dup2(fd, STDOUT_FILENO);

                close(fd);
            }

            redirige = redirige->tail;
        }

        execvp(command[0], command);
        fprintf(stderr, "-bshell: %s : command not found\n", command[0]);
        exit(EXIT_FAILURE);

    } else if (pid < 0) {
        // ==== FEHLER BEIM FORK ====
        perror("shell");
        return 1;

    } else {
        // ==== ELTERNPROZESS ====
        printf(">> [basicsh] executing: %s\n", command[0]);
        statuslist_add(pid, pid, command[0]);
        setpgid(pid, pid);  // In eigene Prozessgruppe setzen

        if (!background) {
            tcsetpgrp(fdtty, pid);  // Terminal an Kindprozess übergeben

            int status;
            waitpid(pid, &status, 0);
            tcsetpgrp(fdtty, shell_pid); // Terminal zurückholen

            statuslist_update(pid, status);
        }
    }

    return 0;
}

static int do_execute_simple(SimpleCommand *cmd_s, int background){
    if (cmd_s==NULL){ // Falls der Befehl leer ist (was nicht passieren sollte), einfach zurückkehren
        return 0;
    }
    if (strcmp(cmd_s->command_tokens[0],"exit")==0){ // Wenn der Benutzer "exit" eingibt, Shell sofort verlassen
        exit(0);
    }
    else if (strcmp(cmd_s->command_tokens[0], "cd") == 0) { // Prüfen, ob der eingegebene Befehl "cd" ist
        char *path = cmd_s->command_tokens[1];  // Pfadparameter lesen
    
        if (path == NULL) { // Wenn kein Argument übergeben wurde (z. B. nur "cd"), HOME verwenden
            path = getenv("HOME");
        }
    
        // Prüfen, ob Pfad gültig ist
        if (path == NULL || strlen(path) == 0) {
            fprintf(stderr, "cd: invalid path\n");
            return 1;
        }
    
        if (chdir(path) != 0) {
            perror("cd");
        }
        return 0;
    }
/* do not modify this */
#ifndef NOLIBREADLINE
     else if (strcmp(cmd_s->command_tokens[0],"hist")==0){ // Wenn "hist", dann Verlauf anzeigen (wenn durch readline unterstützt)
        return builtin_hist(cmd_s->command_tokens);
#endif /* NOLIBREADLINE */

    } 
    else if (strcmp(cmd_s->command_tokens[0], "status") == 0){
        statuslist_print_and_cleanup();  // Funktion in statuslist.c aufrufen
        return 0;
    } else { // Für alle anderen Befehle wird fork() verwendet
        return execute_fork(cmd_s, background);
    }
    fprintf(stderr, "This should never happen!\n"); // Notfallmeldung, falls kein Fall zutrifft
    exit(1);
}

/*
 * Prüft, ob der Befehl im Vordergrund oder Hintergrund ausgeführt werden soll.
 *
 * Bei Sequenzen wird das &-Zeichen am Ende geprüft.
 *
 * Rückgabewerte:
 *      0 → Vordergrund
 *      1 → Hintergrund
 */
int check_background_execution(Command * cmd){
    List * lst = NULL;
    int background =0;
    switch(cmd->command_type){
    case C_SIMPLE:
        lst = cmd->command_sequence->command_list;
        background = ((SimpleCommand*) lst->head)->background;
        break;
    case C_OR:
    case C_AND:
    case C_PIPE:
    case C_SEQUENCE:
        /*
         * Letzter Befehl in der Liste entscheidet über foreground/hintergrund
         */
        lst = cmd->command_sequence->command_list;
        while (lst !=NULL){ // Durchläuft alle Befehle in der Liste. Z. B. "ls ; echo hallo &"
            background = ((SimpleCommand*) lst->head)->background;
            lst=lst->tail;
        }
        break;
    case C_EMPTY:
    default:
        break;
    }
    return background;
}

/* Startet die vollständige Ausführung des Befehls, egal ob einfach oder komplex */
int execute(Command * cmd){
    unquote_command(cmd);

    int res=0;
    List * lst=NULL;

    int execute_in_background=check_background_execution(cmd); // Prüfen, ob am Ende ein & steht
    switch(cmd->command_type){
    case C_EMPTY:
        break;
    case C_SIMPLE:
        res=do_execute_simple((SimpleCommand*) cmd->command_sequence->command_list->head, execute_in_background);
        fflush(stderr); // Fehlerausgabe sofort erzwingen
        break;

    case C_OR:
    case C_AND:
    {
        List *lst = cmd->command_sequence->command_list;
        int last_result = 0;
    
        while (lst != NULL) {
            if (lst->head == NULL) {
                fprintf(stderr, "Error : empty list\n");
                break;
            }
    
            SimpleCommand *cmd_s = (SimpleCommand *)lst->head;

            // Entscheidet ob ausgeführt wird
            if (
                lst == cmd->command_sequence->command_list ||  // Erste Anweisung wird sowieso ausgeführt
                (cmd->command_type == C_AND && last_result == 0) ||  // && → nur wenn vorheriger Erfolg
                (cmd->command_type == C_OR && last_result != 0) // || → nur wenn vorheriger Fehler
            ) {
                last_result = execute_fork(cmd_s, 0); // Ergebnis merken
            }
    
            lst = lst->tail;
        }
        break;
    }
    case C_SEQUENCE:
        /** Iteration durch die Befehlsliste **/
        lst = cmd->command_sequence->command_list;
        while (lst !=NULL){
            SimpleCommand * cmd = (SimpleCommand*)lst->head; // Befehl holen
            execute_fork(cmd, 0); // Vordergrundausführung
            lst=lst->tail; // Nächster Befehl
        }
        break;

    case C_PIPE: {
        List *lst = cmd->command_sequence->command_list;
        int fd_pipe[2];
        int last_fd = -1;
        pid_t pgid = 0;
        pid_t pids[256]; // feste Größe (max. 256 Befehle in einer Pipe)
        int i = 0;

        while (lst != NULL) {
            SimpleCommand *cmd_s = (SimpleCommand *)lst->head;

            if (lst->tail != NULL) {
                if (pipe(fd_pipe) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }

            pid_t pid = fork();

            if (pid == 0) {
                if (pgid == 0) pgid = getpid();
                setpgid(0, pgid);

                signal(SIGINT, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);

                if (last_fd != -1) {
                    dup2(last_fd, STDIN_FILENO);
                    close(last_fd);
                }

                if (lst->tail != NULL) {
                    close(fd_pipe[0]);
                    dup2(fd_pipe[1], STDOUT_FILENO);
                    close(fd_pipe[1]);
                }

                // Redirection behandeln
                List *redirige = cmd_s->redirections;
                while (redirige != NULL) {
                    Redirection *redir = (Redirection *)redirige->head;
                    int fd;

                    if (redir->r_type == R_FILE) {
                        if (redir->r_mode == M_WRITE)
                            fd = open(redir->u.r_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        else if (redir->r_mode == M_APPEND)
                            fd = open(redir->u.r_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        else
                            fd = open(redir->u.r_file, O_RDONLY);

                        if (fd < 0) {
                            fprintf(stderr, "%s: %s: %s\n", cmd_s->command_tokens[0], redir->u.r_file, strerror(errno));
                            exit(1);
                        }

                        if (redir->r_mode == M_READ)
                            dup2(fd, STDIN_FILENO);
                        else
                            dup2(fd, STDOUT_FILENO);

                        close(fd);
                    }

                    redirige = redirige->tail;
                }

                execvp(cmd_s->command_tokens[0], cmd_s->command_tokens);
                perror("exec");
                exit(1);
            } else if (pid > 0) {
                if (pgid == 0) pgid = pid;
                setpgid(pid, pgid);

                pids[i++] = pid;
                statuslist_add(pid, pgid, cmd_s->command_tokens[0]);

                if (last_fd != -1)
                    close(last_fd);

                if (lst->tail != NULL) {
                    close(fd_pipe[1]);
                    last_fd = fd_pipe[0];
                }
            } else {
                perror("fork");
                exit(1);
            }

            lst = lst->tail;
        }

        // Auf alle Prozesse in der Pipe warten
        tcsetpgrp(fdtty, pgid);
        int status;

        for (int j = 0; j < i; j++) {
            pid_t ret = waitpid(pids[j], &status, 0);
            if (ret > 0) {
                statuslist_update(ret, status);
            }
        }
        tcsetpgrp(fdtty, shell_pid);
        break;
    }

    default:
        printf("[%s] unhandled command type [%i]\n", __func__, cmd->command_type);
        break;
    }
    return res;
}