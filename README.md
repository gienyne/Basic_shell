# MiniShell – Implementierung einer vereinfachten Shell

Dieses Projekt beinhaltet die Entwicklung einer **MiniShell** in C, die grundlegende Funktionen moderner Kommandozeilenumgebungen unterstützt.  
Die Shell implementiert Befehlsausführung, Pipes, Umleitungen, Hintergrundprozesse sowie Signalbehandlung.

---

## Unterstützte Features

### 1. `cd`-Befehl

- Wechsel in bestehende Verzeichnisse  
- Fehlerbehandlung bei nicht vorhandenen Verzeichnissen oder fehlenden Rechten  
- Standardverhalten (`cd` ohne Argument → Wechsel ins Home-Verzeichnis)

**Beispiel:**
```bash
$ cd /tmp
$ pwd
/tmp
$ cd xx
xx: No such file or directory
$ cd
$ pwd
/home/user

2. Verknüpfung von Befehlen
Unterstützung von UND-Verknüpfung (&&) und ODER-Verknüpfung (||).

Beispiel:

$ true && echo yay
yay
$ false || echo yay
yay

3. Umleitungen (Redirections)

stdout-Umleitung (>, >>)

stdin-Umleitung (<)

Fehlerbehandlung bei nicht vorhandenen Dateien oder fehlenden Berechtigungen

Beispiel:

$ echo hallo > f
$ cat f
hallo
$ echo hallo >> f
$ cat f
hallo
hallo
$ cat < xyz
xyz: No such file or directory

4. Pipelines

Standard-Pipelines (z. B. cat | cat | cat)

Korrektes Warten auf alle Prozesse in der Pipeline

Keine Verklemmungen bei vollen Pipes

Sichere Beendigung von Prozessen bei CTRL+C

Beispiel:

$ ls -l | wc
$ cat /bin/bash | od -x | head -1
$ cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat
^C
$ status

5. Statusanzeige (status)

Anzeige von Rückgabewerten

Erkennung von Signalen

Verwaltung und Entfernen beendeter Hintergrundprozesse

Beispiel:

$ ls -l xyz &
$ status
$ kill -9 <pid>
$ status

Architektur & Code-Struktur

Die Implementierung ist modular aufgebaut und umfasst u. a.:

command.c / command.h – Verwaltung von Befehlen (einfach & zusammengesetzt)

parser.c – Kommandozeilen-Parser

executor.c – Ausführung inkl. Redirections, Pipes und Hintergrundprozesse

list.c / list.h – Eigene Listenstruktur zur Prozessverwaltung

debug.c – Optionale Debug-Ausgaben

Build & Ausführung

Kompilieren und Starten über die bereitgestellte Makefile bzw. die Anleitung in README.txt.

make
./minishell

Autor : DIMITRY NTOFEU NYATCHA
Fachhochschule: [THM]
Kurs: Betriebssysteme
Sprache: C

Lizenz / Verwendung
kein öffentlicher Wiedergebrauch ohne Rücksprache.


