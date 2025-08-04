#define _GNU_SOURCE // Aktiviert GNU-spezifische Erweiterungen (z. B. get_current_dir_name, strdup, etc.)
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
#include "execute.h"
#include "debug.h"

#ifndef NOLIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

/*
 * Die Struktur 'cmd' wird nur vom Parser verwaltet.
 * Du musst dich darum nicht kümmern – sie funktioniert!
 */
extern Command * cmd;
extern List *statuslist;

#ifndef NOLIBREADLINE
extern char *current_readline_prompt; // Prompt wie z. B. bshell [/home/user]>, wo Benutzer Befehle eingeben können
#endif

int fdtty;
int shell_pid;

/* Deklaration für die Parserfunktionen und -strukturen */
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char * str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yylex();
extern char *yytext;

/**
 * Ignoriert bestimmte Signale im Shell-Prozess, damit der Benutzer die Shell nicht versehentlich beendet.
 */
void disable_signal(int signo, int flags) {
    struct sigaction siga;
    siga.sa_handler = SIG_IGN; // Signal ignorieren (z. B. Ctrl+C)
    siga.sa_flags = flags;
    sigemptyset(&siga.sa_mask); // Leere Maske: keine weiteren Signale blockieren
    if (sigaction(signo, &siga, NULL) < 0) {
        perror("Fehler bei sigaction");
        exit(1);
    }
}

/**
 * Deaktiviert alle störenden Signale (z. B. Ctrl+C, Hintergrundprozesse), damit die Shell stabil läuft.
 */
void disable_signals() {
    disable_signal(SIGINT, SA_RESTART);   // Ctrl+C
    disable_signal(SIGTTOU, 0);           // Terminal-Zugriffsprobleme bei Hintergrundprozessen
    disable_signal(SIGTSTP, 0);           // Ctrl+Z (Stoppen durch Benutzer)
}

/**
 * Signalhandler für SIGCHLD – wird aufgerufen, wenn ein Kindprozess beendet wurde.
 * Verhindert Zombie-Prozesse durch regelmäßiges Aufräumen via waitpid().
 */
void sigchld_handler(int signo) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            statuslist_update(pid, status);  // Aktualisiert die Liste der Kinderprozesse
        }
    }
}

/**
 * Hauptfunktion der Shell
 */
int main(int argc, char *argv[], char **envp) {
    char *line = NULL;

    disable_signals(); // Signale wie Ctrl+C deaktivieren

    // SIGCHLD-Handler einrichten: Aufräumen beendeter Kindprozesse
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask); // Keine Signale während des Handlers blockieren
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    // Öffnet das Terminal direkt (für Gruppensteuerung)
    fdtty = open("/dev/tty", O_RDONLY | O_CLOEXEC);
    shell_pid = getpid();           // PID der Shell speichern
    setpgid(0, shell_pid);          // Neue Prozessgruppe für die Shell setzen
    tcsetpgrp(fdtty, shell_pid);    // Kontrolle über das Terminal übernehmen

    int print_commands = 0;
    if (argc > 1) {
        print_commands = strcmp(argv[1], "--print-commands") == 0 ? 1 : 0; // Aktiviert Debug-Ausgabe der eingegebenen Befehle
    }

#ifndef NOLIBREADLINE
    using_history(); // Initialisiert die Verlaufsspeicherung
    current_readline_prompt = malloc(1024); // Reserviert Speicher für das Prompt
#endif

    while (1) {
        int parser_res;
        char cwd[256]; // Aktuelles Arbeitsverzeichnis

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            cwd[0] = '\0';
        }

#ifndef NOLIBREADLINE
        sprintf(current_readline_prompt, "bshell [%s]> ", cwd); // Prompt zusammensetzen
#else
        printf("bshell [%s]> ", cwd);
#endif
        fflush(stdout); // Stellt sicher, dass das Prompt sofort angezeigt wird

        parser_res = yyparse(); // Startet den Parser (Analyse der Benutzereingabe)

        if (parser_res == 0) { // Erfolgreich geparst
            if ((line = command_get(cmd)) != NULL) {
#ifndef NOLIBREADLINE
                add_history(line); // Zur Verlaufsliste hinzufügen
#endif
                free(line); // Speicher wieder freigeben
            }

            if (print_commands == 1) {
                command_print(cmd); // Optional: gibt intern analysierten Befehl aus
            }

            execute(cmd);         // Führt den Befehl aus (Fork + Exec)
            command_delete(cmd); // Bereinigt den Speicher
        } else if (parser_res == 1) {
            fprintf(stderr, "[%s %s %i] Parser-Fehler: yyerror ausgelöst (parser_res = 1)!\n",
                    __FILE__, __func__, __LINE__);
        } else if (parser_res == 2) {
            fprintf(stderr, "[%s %s %i] Parser-Fehler: Ungültige Eingabe (parser_res = 2)!\n",
                    __FILE__, __func__, __LINE__);
        } else {
            fprintf(stderr, "[%s %s %i] Schwerwiegender Parser-Fehler (parser_res = %i)\n",
                    __FILE__, __func__, __LINE__, parser_res);
        }
    }
}





















































