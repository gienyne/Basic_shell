#define _GNU_SOURCE
#include "statuslist.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

List *statuslist = NULL;

/**
 * Fügt einen neuen Prozess zur Statusliste hinzu.
 * - `pid`: Prozess-ID
 * - `pgid`: Prozessgruppen-ID
 * - `command`: Kommando als String
 */
void statuslist_add(pid_t pid, pid_t pgid, const char *command) {
	ProcessInfo *info = malloc(sizeof(ProcessInfo));
	info->pid = pid;
	info->gpid = pgid;
	info->status = RUNNING;
	info->code = -1;
	info->command = strdup(command);  // Kopie des Befehls

	statuslist = list_append(info, statuslist);  // Zur Liste hinzufügen
}

/**
 * Aktualisiert den Status eines Prozesses basierend auf seiner PID und dem Rückgabewert `status`.
 */
void statuslist_update(pid_t pid, int status) {
	List* current = statuslist;

	while (current != NULL) {
		ProcessInfo *info = (ProcessInfo*)current->head;

		if (info->pid == pid) {
			if (info->status != RUNNING) {
				// Aktualisiere keinen bereits beendeten Prozess
				return;
			}

			if (WIFEXITED(status)) {
				info->status = EXITED;
				info->code = WEXITSTATUS(status);
			} else if (WIFSIGNALED(status)) {
				info->status = SIGNALED;
				info->code = WTERMSIG(status);
			}
			break;
		}

		current = current->tail;
	}
}

/**
 * Gibt alle Prozesse der Liste aus und bereinigt beendete Einträge.
 * Prozesse, die noch laufen, werden in einer neuen Liste behalten.
 */
void statuslist_print_and_cleanup() {
	List *current = statuslist;
	List *newlist = NULL;

	printf("%-5s %-5s %-12s %s\n", "PID", "PGID", "STATUS", "NAME");

	while (current != NULL) {
		ProcessInfo *info = (ProcessInfo*) current->head;

		// Status aktualisieren, falls Prozess noch läuft
		if (info->status == RUNNING) {
			int status;
			pid_t result = waitpid(info->pid, &status, WNOHANG);
			if (result == info->pid) {
				if (WIFEXITED(status)) {
					info->status = EXITED;
					info->code = WEXITSTATUS(status);
				} else if (WIFSIGNALED(status)) {
					info->status = SIGNALED;
					info->code = WTERMSIG(status);
				}
			}
		}

		// Zeichenkette zur Anzeige vorbereiten
		char status_str[32];
		if (info->status == RUNNING) {
			strcpy(status_str, "running");
		} else if (info->status == EXITED) {
			snprintf(status_str, sizeof(status_str), "exit(%d)", info->code);
		} else if (info->status == SIGNALED) {
			snprintf(status_str, sizeof(status_str), "signal(%d)", info->code);
		} else {
			strcpy(status_str, "unknown");
		}

		// Ausgabe und Freigabe je nach Status
		printf("%-5d %-5d %-12s %s\n", info->pid, info->gpid, status_str, info->command);

		if (info->status == RUNNING) {
			newlist = list_append(info, newlist);  // Noch laufende Prozesse behalten
		} else {
			free(info->command);
			free(info);
		}

		List *tmp = current;
		current = current->tail;
		free(tmp);
	}

	statuslist = newlist;
}

/**
 * Gibt den gesamten Speicher der Statusliste frei (inklusive aller Einträge).
 */
void statuslist_free() {
	List *current = statuslist;
	while (current != NULL) {
		ProcessInfo *entry = (ProcessInfo*)current->head;
		free(entry->command);
		free(entry);
		List *tmp = current;
		current = current->tail;
		free(tmp);
	}
	statuslist = NULL;
}