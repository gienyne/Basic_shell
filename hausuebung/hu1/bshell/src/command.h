/*
 * command.h
 *
 * Definition der Repräsentation eines Befehls.
 *
 */

#ifndef COMMAND_H
#define COMMAND_H

#include "list.h"

/*
 * Es gibt verschiedene Arten von Befehlen
 */
typedef enum {
    C_EMPTY,         /* leerer Befehl */ 
    C_SIMPLE,        /* einfacher Befehl */  // z. B. ls -l
    C_SEQUENCE,      /* Befehlssequenz mit ";" */  // mehrere Befehle durch ; getrennt
    C_PIPE,          /* Verwendung von "|" */  // z. B. ls | grep txt
    C_AND,           /* mit "&&" */
    C_OR,            /* mit "||" */
    C_IF,            /* bedingte Ausführung */
    C_WHILE          /* Schleifenkonstruktion */
} CommandType;

/*
 * Möchtest du aus einer Datei lesen? In eine schreiben? Oder anhängen?
 * z. B. mit den Symbolen <, >, >>
 */
typedef enum {
    M_READ,            /* <  = lesen */
    M_WRITE,           /* >  = schreiben (überschreibt) */
    M_APPEND           /* >> = anhängen */
} RedirectionMode;

/*
 * Wird in eine Datei (z. B. output.txt) oder in einen Deskriptor (z. B. 2>&1) umgeleitet?
 */
typedef enum {
    R_FD,   // Umleitung zu einem Dateideskriptor (z. B. 2>&1)
    R_FILE  // Umleitung zu einer Datei (z. B. > output.txt)
} RedirectionType;

/*
 * Eine Umleitung kann entweder zu einer Datei oder zu einem Deskriptor erfolgen,
 * wie stdin, stdout oder stderr.
 */
typedef struct {
    RedirectionType r_type;
    RedirectionMode r_mode;          /* steuert die Verwendung des Deskriptors (READ/WRITE/APPEND) */
    union {
        int r_fd;         /* Dateideskriptor (Quelle oder Ziel) */
        char * r_file;    /* vollständiger Dateiname (Quelle oder Ziel) */
    } u;
} Redirection;

/*
 * Ein einfacher Befehl kann verschiedene Umleitungen enthalten
 * und besteht aus Tokens, die den Befehl beschreiben
 */
typedef struct SimpleCommand {
  List *redirections;   // Liste von Redirection-Objekten
  int  command_token_counter;   // Anzahl der Tokens
  int background;    // 1 = im Hintergrund (&), 0 = im Vordergrund
  char ** command_tokens;  // Array von Zeichenketten (z. B. {"ls", "-l", NULL})
} SimpleCommand;


/*
 * Eine Befehlssequenz ist eine Liste von einfachen Befehlen
 */
typedef struct {
  List *command_list;    /* Liste von SimpleCommand */
  int  command_list_len;
} CommandSequence;

typedef struct command {
    CommandType command_type;
    CommandSequence *command_sequence;
} Command;

/* Funktion zum Erstellen eines neuen einfachen Befehls */
SimpleCommand * simple_command_new(int, char **, List *, int);

/* Erstellt einen leeren Befehl */
Command * command_new_empty();

/* Erstellt einen zusammengesetzten Befehl (z. B. Befehl1 && Befehl2) */
Command * command_new(int type, SimpleCommand * cmd, SimpleCommand * cmd2);

/* Fügt einen einfachen Befehl an einen bestehenden komplexen Befehl an */
Command * command_append(int type, SimpleCommand * s_cmd, Command * cmd);

/* Gibt den Befehl formatiert auf der Konsole aus */
void command_print(Command *cmd);

/* Gibt den belegten Speicher des Befehls frei */
void command_delete(Command *cmd);

/* Gibt den Speicher für alle Umleitungen frei */
void delete_redirections(List *rd);

/* Gibt einen einfachen Befehl mit Einrückung aus (für Debug-Zwecke) */
void simple_command_print(int indent, SimpleCommand *cmd_s);

/* Gibt eine Zeichenkette des Befehls zurück (z. B. für Verlauf oder Anzeige) */
char * command_get(Command *);

#endif /* Ende der Include-Absicherung */