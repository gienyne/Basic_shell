# 🐚 MiniShell – Hausübung 1 (Betriebssysteme)

Dieses Projekt ist im Rahmen der **Hausübung 1** des Kurses *Betriebssysteme* an der Fachhochschule entstanden.  
Ziel war es, eine vereinfachte Shell zu implementieren, die grundlegende Funktionen wie **Befehlsausführung**, **Pipes**, **Redirections**, **Hintergrundprozesse** und **Signalbehandlung** unterstützt.

> ⚠️ **Hinweis:** Das ursprüngliche C-Skelett wurde von den Dozierenden zur Verfügung gestellt.  
> Die hier dokumentierten Erweiterungen und Implementierungen wurden selbstständig vorgenommen.

---

## ✅ Unterstützte Features (Checkliste)

### 🔸 1. `cd`-Befehl
- [x] `cd` in existierendes Verzeichnis  
- [x] `cd` in nicht existierendes Verzeichnis → Fehlermeldung  
- [x] `cd` in Verzeichnis mit fehlenden Rechten  
- [x] `cd` ohne Argumente (Wechsel ins Home-Verzeichnis)

**Beispiel:**
```bash
$ cd /tmp
$ pwd
/tmp
$ cd xx
xx: No such file or directory
$ cd
$ pwd
/home/student
🔸 2. Verknüpfung von Befehlen
UND-Verknüpfung (&&)

ODER-Verknüpfung (||)

Beispiel:

$ true && echo yay
yay
$ false && echo nope
$ true || echo nope
$ false || echo yay
yay
🔸 3. Umleitungen (Redirections)
stdout-Umleitung (>, >>)

stdin-Umleitung (<)

Fehlerbehandlung bei fehlenden Dateien oder Rechten

Beispiel:

$ echo hallo > f
$ cat f
hallo
$ echo hallo >> f
$ cat f
hallo
hallo
$ cat < f >> f1
$ cat f1
hallo
hallo
$ cat < xyz
xyz: No such file or directory
$ touch outfile
$ chmod 000 outfile
$ ls >> outfile
outfile: Permission denied

🔸 4. Pipelines
Standardpipelines (z. B. cat | cat | cat)

Warten auf alle Teilnehmer der Pipeline

Keine Verklemmung bei vollen Pipes

Abbruch von >10 Prozessen mit CTRL+C korrekt behandelt

Beispiel:

$ cat | cat | cat
$ ls -l | wc
$ cat /bin/bash | od -x | head -1
$ cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat
^C
$ status

🔸 5. Statusanzeige (status)
Rückgabewerte korrekt anzeigen

Signale korrekt erkennen

Beendete Hintergrundprozesse entfernen

Beispiel:

$ ls -l xyz &
$ xterm &
$ ps &
$ status
$ kill -9 <pid>
$ status

🛠️ Struktur des Codes
Die Implementierung umfasst unter anderem folgende Dateien:

command.c / command.h – Verwaltung von einfachen und zusammengesetzten Befehlen

parser.c – Parsen der Eingabe

executor.c – Ausführung inkl. Redirection, Pipes und Hintergrundprozesse

list.c / list.h – Eigene verkettete Listenstruktur

debug.c – Optionales Debugging / Command-Ausgabe

📄 Aufgabenstellung
Die vollständige Aufgabenbeschreibung befindet sich in der Datei Hausübung1.pdf.

⚙️ Kompilieren & Starten
Um das Projekt zu kompilieren und auszuführen: siehe datei: README.txt


🧑‍💻 Autor : DIMITRY NTOFEU NYATCHA
Fachhochschule: [THM]
Kurs: Betriebssysteme – Hausübung 1
Sprache: C

🔒 Lizenz / Verwendung
Privates Uni-Projekt – kein öffentlicher Wiedergebrauch ohne Rücksprache.
Nur zu Demonstrations- und Lernzwecken gedacht.

