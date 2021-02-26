#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <signal.h>
#include <time.h>
using namespace std;

#define RED   "\x1B[31m"
#define YELLOW  "\x1b[33m"
#define RESET "\x1B[0m"

#define MAXLINE 100
#define MOD "exit with CTR C"

bool background = false;
time_t t_start;
time_t t_end;

// Im Falle eines Interrupts/Signals --> Tue folgendes
void handler(int signal)
{
	t_end = time(NULL);
	time_t currentTime = t_end - t_start;
	struct tm* ptm = localtime(&currentTime);
	printf("\nTime Elapsed %02d h :%02d m :%02d s\n", ptm->tm_hour = 0, ptm->tm_min, ptm->tm_sec);

	printf("\nProzess starb mit Signal: %d\n", signal);
	exit(0);
}

// Gibt "Titel" der Minishell aus
void printTitle()
{
	printf(YELLOW "\n************************ MINISHELL ************************\n" RESET);
	printf("exit with CTR-C\n");
}

// Gibt aktuelles Verzeichnis aus
void printDir()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf( YELLOW "\n%s" RESET, cwd);
}

int read_command(char* command, char* parameters[])
{
	fprintf(stdout, RED "> " RESET); // Prompt wird ausgegeben
	int noParam = 0;
	string eingabe; // Benutzereingabe

	while (1)
	{
		if (cin.peek() == '\n') // Wenn '\n' im Eingabestrom erscheint:
		{
			cin.ignore(); // dann ignoriere dieses '\n'
			break; // Kommandoeingabe ist fertig
		}

		cin >> eingabe; // cin holt alles bis zum naechsten ' '
		parameters[noParam] = new char[20]; // Allokiere Speicher fuer das char*-Array
//			   Ziel-Array			Quell-Array
		strcpy(parameters[noParam], eingabe.c_str()); // Eingabe Zeichen fuer Zeichen kopieren
		noParam++; // naechste Stelle im char*-Array
	}

	strcpy(command, parameters[0]); // NUR Kommando was eingegeben wurde in command kopieren

	if (strcmp(parameters[noParam - 1], "&") == 0) // Wenn das vorletzte Zeichen ein "&" war
	{
		parameters[noParam - 1] = NULL; // entferne dieses Zeichen
		background = true; // Es wird ein Hintergrundprozess begonnen
	}
	else
	{
		parameters[noParam] = NULL; // ansonsten ist es ein "normales" Kommando
		background = false; // Es wird kein Hintergrundprozess gestartet -> Vater soll nur warten
	}
	return noParam; // Springe ins Hauptprogramm zurueck
}

int main(int argc, char* argv[])
{
	printTitle();

	t_start = time(NULL);

	int childPid; // Kindprozessidentifikationsnummer
	int status;

	char command[MAXLINE]; // Kommando
	char* parameters[MAXLINE]; // Parameter/Argumente d. Kommandos

	int noParams;

	signal(SIGINT, handler); // Immer wenn das Programm ein Signal SIGINT bekommt, soll der handler aufgerufen werden
	signal(SIGCHLD, SIG_IGN);

	while (1)
	{
		// Verzeichnis & Prompt ausgeben:
		printDir();
		noParams = read_command(command, parameters); // Nun erfolgt Benutzereingabe

		if (!strcmp(parameters[0], "cd")) // wird das Verzeichnis gewechselt?
		{
			// Verzeichnis wechseln
			chdir(parameters[1]);
			continue;
		}

		if (noParams == 0)
		{
			fprintf(stderr, "no command ?!\n");
			exit(1);
		}

		childPid = fork(); // Kind/Subshell erzeugen
		cout << "Nun wurde fork() gemacht\n";
		if (childPid == -1) // Fehler, wenn fork() - 1 zurueckgibt
		{
			fprintf(stderr, "can't fork!\n");
			exit(2);
		}
		else if (childPid == 0)  // Wir befinden uns im Kind: Fuehre das Kommando aus
		{
			execvp(command, parameters); // executes command
			exit(3);
		}
		else
		{
			if (background == false)
			{
				waitpid(childPid, &status, WUNTRACED | WCONTINUED); // Vater wartet bis Kind fertig ist
			}
			else if (background == true)
			{
				// NICHT warten --> Kind arbeitet im Hintergrund
				cout << "[" << childPid << "]";
			}
		}
	}
	exit(0);
}
