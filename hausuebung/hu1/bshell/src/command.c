#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "command.h"
#include "list.h"
#include "stringbuffer.h"

#include "debug.h"


// Erstellt ein leeres Kommando. Nützlich als "neutrale Basis", falls keine Tokens in einer Kommandozeile gefunden wurden.
Command * command_new_empty(){
	Command * cmd = malloc(sizeof(struct command));
	cmd->command_type=C_EMPTY;
	cmd->command_sequence = NULL;
	return cmd;
}


// Fügt ein neues einfaches Kommando (s_cmd) am Anfang eines zusammengesetzten Kommandos (cmd) hinzu, z. B. eine Sequenz von Befehlen wie cmd1 ; cmd2 ; cmd3.
Command * command_append(int type, SimpleCommand * s_cmd, Command * cmd){
	List * lst = malloc(sizeof(List));
	lst->head=s_cmd;
	lst->tail=cmd->command_sequence->command_list;
	cmd->command_sequence->command_list_len++;
	cmd->command_sequence->command_list=lst;
	return cmd;
}


/*
Erstellt ein zusammengesetztes Kommando:

- ein einfaches Kommando, falls cmd2 == NULL
- eine Sequenz (cmd1 && cmd2, cmd1 | cmd2, usw.), wenn beide vorhanden sind

Beispiel: ls | grep txt
cmd1 = ls
cmd2 = grep txt
type = C_PIPE → beschreibt die Art der Verbindung zwischen den beiden Befehlen

Die Funktion erstellt eine verkettete Liste: [cmd1] -> [cmd2]
*/
Command * command_new(int type, SimpleCommand * cmd1, SimpleCommand * cmd2){
	Command * new_cmd = malloc(sizeof(struct command));
	new_cmd->command_sequence = malloc(sizeof(CommandSequence));
	new_cmd->command_sequence->command_list = malloc(sizeof(List));

	((List *) new_cmd->command_sequence->command_list)->head=cmd1;

	if (cmd2 == NULL){
		new_cmd->command_type=C_SIMPLE;
		new_cmd->command_sequence->command_list_len=1;
		new_cmd->command_sequence->command_list->tail=NULL;
		return new_cmd;
		} else {
		new_cmd->command_type=type;
		new_cmd->command_sequence->command_list_len=2;

		List * lst=malloc(sizeof(List));
		lst->head=cmd2;
		lst->tail=NULL;
		new_cmd->command_sequence->command_list->tail=lst;
	}
	return new_cmd;
}


/*
Erstellt ein einfaches Kommando mit:

- seinen Argumenten (z. B. ls, -l)
- Redirectionen (> out.txt)
- Hintergrundmodus (&)
*/
SimpleCommand * simple_command_new(int len, char ** tokens, List *redirections, int background){
	SimpleCommand *cmd=malloc(sizeof(*cmd));
	cmd->redirections=redirections;
	cmd->command_token_counter=len;
	cmd->background = background;
	cmd->command_tokens=tokens;
	return cmd;
}


// Gibt den Speicher eines Kommandos (einfach oder zusammengesetzt) vollständig frei.
void command_delete(Command *cmd) {
	struct SimpleCommand *cmd_s;
	List * cmd_lst;
	switch(cmd->command_type) {
		case C_EMPTY:
		free(cmd);
		break;
		case C_SIMPLE:
		cmd_s=(SimpleCommand *) ((List *)cmd->command_sequence->command_list)->head;
		delete_redirections(cmd_s->redirections);
		for (int i=0; i< cmd_s->command_token_counter; i++) {
			free(cmd_s->command_tokens[i]);
		}
		free(cmd_s->command_tokens);
		free(cmd_s);
		free(cmd->command_sequence->command_list);
		free(cmd->command_sequence);
		free(cmd);
		break;
		case C_AND:
		case C_OR:
		case C_PIPE:
		case C_SEQUENCE:
		cmd_lst=cmd->command_sequence->command_list;
		do {
			cmd_s=cmd_lst->head;
			delete_redirections(cmd_s->redirections);
			for (int i=0; i< cmd_s->command_token_counter; i++) {
				free(cmd_s->command_tokens[i]);
			}
			free(cmd_s->command_tokens);
			free(cmd_s);

			void * previous_cmd_lst = cmd_lst;
			cmd_lst=cmd_lst->tail;
			free(previous_cmd_lst);
		} while (cmd_lst!=NULL);
		free(cmd->command_sequence);
		free(cmd);
		break;
		default:
		break;
	}

}

// Gibt das Kommando in lesbarer Form aus, hilfreich zum Debuggen.
void command_print(Command *cmd) {
	struct SimpleCommand *cmd_s;
	char * type = NULL;
	List * cmd_lst;
	printf("--- COMMANDS ---\n");
	switch(cmd->command_type) {
		case C_EMPTY:
		printf("<EMPTY_COMMAND>\n");
		break;
		case C_SIMPLE:
		cmd_s=(SimpleCommand *) ((List *)cmd->command_sequence->command_list)->head;
		simple_command_print(0,cmd_s);
		break;
		case C_AND:
		if (type==NULL) type="AND_COMMAND";
		case C_OR:
		if (type==NULL) type="OR_COMMAND";
		case C_PIPE:
		if (type==NULL) type="PIPE_COMMAND";
		case C_SEQUENCE:
		if (type==NULL) type="SEQUENCE_COMMAND";

		printf( "%*s<%s [%i]>\n", 0, "", type, cmd->command_sequence->command_list_len);
		cmd_lst=cmd->command_sequence->command_list;
		do {
			cmd_s=cmd_lst->head;
			simple_command_print(5, cmd_s);
			cmd_lst=cmd_lst->tail;
		} while (cmd_lst!=NULL);
		printf( "%*s</%s>\n", 0, "", type);
		break;
		default:
		printf( "type [%i] printing not implemented\n", cmd->command_type);
		break;
	}
	printf("<<<< COMMANDS >>>>\n");
}

void print_redirection(int indent, Redirection * r){
	char *type="";
	char *mode="";
	switch (r->r_type){
		case R_FD:
		type="descriptor";
		break;
		case R_FILE:
		type="file";
		break;
		default:
		type="unknown";
	}

	switch (r->r_mode){
		case M_READ:
		mode = "READ";
		break;
		case M_WRITE:
		mode = "WRITE";
		break;
		case M_APPEND:
		mode= "APPEND";
		break;
		default:
		mode="unknown";
		break;
	}

	if (r->r_type == R_FILE) {
		printf("%*s {mode: \"%s\", type: \"%s\", filename: \"%s\"}", indent, "", mode, type, r->u.r_file);
		} else if (r->r_type == R_FD){
		printf("%*s {mode: \"%s\", type: \"%s\", filedescriptor: %i}", indent, "", mode, type, r->u.r_fd);
		} else {
		fprintf(stderr, "Redirection error. Unknown type\n");
		exit(1);
	}

}

// Gibt die gesamte Redirectionsliste eines SimpleCommand frei.
void delete_redirections(List * rd){
	if (rd == NULL) return;
	List *current=rd;
	List *previous=NULL;
	while (current != NULL) {
		previous=current;
		current=previous->tail;
		Redirection *r_cur=(Redirection *) previous->head;
		if (r_cur->r_type==R_FILE)
		free(r_cur->u.r_file);
		free(r_cur);
		free(previous);
	}
}

// Gibt ein SimpleCommand in lesbarer Form aus. `indent` wird benutzt, um Einrückungen zu steuern (für verschachtelte Darstellung).
void simple_command_print(int indent, SimpleCommand *cmd_s){
	printf("%*s%s", indent, "", "<SIMPLE_COMMAND> {command: \"");
		for (int i=0; i< cmd_s->command_token_counter;i++){
			if (i==cmd_s->command_token_counter-1)
			printf("%s", cmd_s->command_tokens[i]);
			else
			printf("%s ", cmd_s->command_tokens[i]);
		}
	printf("\", background: \"%s\"}\n", (cmd_s->background==1? "yes" : "no"));

	if (cmd_s->redirections != NULL) {
		printf("%*s <REDIRECTIONS>\n", indent, "");
		List *r_cur=cmd_s->redirections;
		do {
			printf("%*s <REDIRECTION>", indent+2, "");
			print_redirection(0, (Redirection *) r_cur->head);
			printf("%*s </REDIRECTION> \n", indent+2, "");
			r_cur = r_cur->tail;
		} while (r_cur != NULL);
		printf("%*s </REDIRECTIONS>\n", indent, "");
	}

}

/*
Diese Funktion durchläuft die gesamte Struktur eines Kommandos (einfach oder zusammengesetzt)
und erzeugt daraus eine Textzeile – bereit zur Anzeige oder Ausführung.

Beispiel:
Du hast intern gespeichert: `echo hello > out.txt &`
→ Rückgabe: "echo hello > out.txt & "
*/
char * command_get(Command *cmd){
	char *token = "";
	SimpleCommand *cmd_s;
	List * cmd_lst;
	if(cmd->command_type == C_EMPTY) { return NULL; }
	cmd_lst=cmd->command_sequence->command_list;

	StringBuffer cmd_str = string_buffer_new(8192);

	do {
		cmd_s=cmd_lst->head;

		for (int i = 0; i < cmd_s->command_token_counter; i++) {
			string_buffer_append_formatted(&cmd_str, "%s ", cmd_s->command_tokens[i]);
		}

		// Verarbeitung der Redirections (<, >, >> ...)
		List *redirect_lst = cmd_s->redirections;
		while (redirect_lst != NULL) {
			Redirection *redirection = (Redirection *)redirect_lst->head;

			if (redirection->r_type == R_FILE) {
				char *r_token = "";
				switch (redirection->r_mode) {
					case M_READ: r_token = "<"; break;
					case M_WRITE: r_token = ">"; break;
					case M_APPEND: r_token = ">>"; break;
					default: break;
				}
				string_buffer_append_formatted(&cmd_str, "%s %s ", r_token, redirection->u.r_file);
			}
			redirect_lst = redirect_lst->tail;
		}

		if (cmd_s->background && cmd_lst->tail == NULL) {
			string_buffer_append_formatted(&cmd_str, "& ");
		}

		cmd_lst=cmd_lst->tail;
		if (cmd_lst != NULL){
			switch(cmd->command_type) {
				case C_IF:
				case C_WHILE:
				case C_EMPTY:
				break;
				case C_SIMPLE:
				token=" ";
				break;
				case C_AND:
				token="&&";
				break;
				case C_OR:
				token="||";
				break;
				case C_PIPE:
				token="|";
				break;
				case C_SEQUENCE:
				token=";";
				break;
			}
			string_buffer_append_formatted(&cmd_str, "%s ", token);
		}
	} while (cmd_lst!=NULL);
	return cmd_str.cstring;
}