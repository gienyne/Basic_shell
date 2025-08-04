#ifndef STATUSLIST_H

#define STATUSLIST_H

#include "list.h"
#include <sys/types.h>


// defini les etats possibles d'un processus
typedef enum {
	RUNNING,
	EXITED,
	SIGNALED
}ProcessStatus;

// Struktur jeder Einträge in die Statuslist
typedef struct{
	pid_t pid; // pid des Prozesses
	pid_t gpid; //gpid des Prozesses
	ProcessStatus status; // Zustand des Prozesses (RUNNING, EXITED, SIGNALED)
	int code; // exit code
	char *command; // ausgeführter Befehle
} ProcessInfo;

// Liste globale des statuts de processus
extern List *statuslist;

void statuslist_add(pid_t pid, pid_t pgid, const char* command);
void statuslist_update(pid_t pid, int status); // aufgerufen während des SIGCHLD
void statuslist_print_and_cleanup();           // Commande status
void statuslist_free();                        // gibt die liste frei


#endif /* end of include guard: STATUSLIST_H */
