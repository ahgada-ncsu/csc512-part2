/*============================================================================
 Name        : minishell.c
 Author      : Sardor Isakov
 Version     : v2.0
 Copyright   : All rights reserved
 Description : Simple Shell in C, Ansi-style
 ============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#define FALSE           0
#define TRUE            1
#define LINE_LEN        80
#define MAX_ARGS        64
#define MAX_ARG_LEN     64
#define MAX_PATHS       64
#define MAX_PATH_LEN    96
#define WHITESPACE      " .,\t&"
#define STD_INPUT       0
#define STD_OUTPUT      1
char commandInput = '\0';
int buf_chars = 0;
char *pathv[MAX_PATHS];
char commandLine[LINE_LEN];
struct command_t {
        char *name;
        int argc;
        char *argv[MAX_ARGS];
};
void printPrompt();
void welcomeMessage();
int readCommand(char *commandLine, char *commandInput);
int parseCommand(char *commandLine, struct command_t *command);
char * lookupPath(char **, char **);
int parsePath(char **);
int executeFileInCommand(char *, char **, char *);
int executeFileOutCommand(char *, char **, char *);
void executePipedCommand(char **, char  **, char *, char *);

void welcomeMessage() {
	printf("\nWelcome to mini-shell\n");
}

void printPrompt() {
	printf("mshell > ");
}

char *lookupPath(char **argv, char **dir) {
    char *result = NULL;
    char pName[MAX_PATH_LEN];

    if (argv[0][0] == '/') {
        return argv[0];
    }

    if (argv[0][0] == '.') {
        if (argv[0][1] == '.') {
            // Handle "../" case
            if (getcwd(pName, sizeof(pName)) == NULL) {
                perror("getcwd(): error");
                return NULL;
            }
            asprintf(&result, "%s/../%s", pName, argv[0] + 2); // Skip the leading "../"
            return result;
        } else if (argv[0][1] == '/') {
            // Handle "./" case
            if (getcwd(pName, sizeof(pName)) == NULL) {
                perror("getcwd(): error");
                return NULL;
            }
            asprintf(&result, "%s%s", pName, argv[0] + 1); // Skip the leading "."
            return result;
        }
    }

    for (int i = 0; i < MAX_PATHS && dir[i] != NULL; i++) {
        asprintf(&result, "%s/%s", dir[i], argv[0]);
        if (access(result, X_OK) == 0) { // Check if the file is executable
            return result;
        }
        free(result); // Free result if the file is not found
        result = NULL;
    }

    fprintf(stderr, "%s: command not found\n", argv[0]);
    return NULL;
}


int parsePath(char* dirs[]){
	int debug = 0;
	char* pathEnvVar;
	char* thePath;
	int i;

	for(i=0 ; i < MAX_ARGS ; i++ )
		dirs[i] = NULL;
	pathEnvVar = (char*) getenv("PATH");
	thePath = (char*) malloc(strlen(pathEnvVar) + 1);
	strcpy(thePath, pathEnvVar);

	char* pch;
	pch = strtok(thePath, ":");
	int j=0;
	while(pch != NULL) {
		pch = strtok(NULL, ":");
		dirs[j] = pch;
		j++;
	}
	if(debug){
		printf("Directories in PATH variable\n");
		for(i=0; i<MAX_PATHS; i++)
			if(dirs[i] != NULL)
				printf("    Directory[%d]: %s\n", i, dirs[i]);
	}
    return 0;
}

int parseCommand(char * commandLine, struct command_t * command) {
	int debug = 0;

	char * pch;
	pch = strtok (commandLine," ");
	int i=0;
	while (pch != NULL) {
		command->argv[i] = pch;
		pch = strtok (NULL, " ");
		i++;
	}
	command->argc = i;
	command->argv[i++] = NULL;

	if(debug) {
		printf("Stub: parseCommand(char, struct);\n");
		printf("Array size: %lu\n", sizeof(*command->argv));
		int j;
		for(j=0; j<i; j++) {
			printf("command->argv[%i] = %s\n", j, command->argv[j]);
		}
		printf("\ncommand->argc = %i\n", command->argc);

		if(command->argv[0] != NULL) {
			char **p;
			for(p = &command->argv[1]; *p != NULL; p++) {
				printf("%s\n", *p);
			}
		}
	}
	return 0;
}

int readCommand(char * buffer, char * commandInput) {
	int debug = 0;
	buf_chars = 0;


	while((*commandInput != '\n') && (buf_chars < LINE_LEN)) {
		buffer[buf_chars++] = *commandInput;
		*commandInput = getchar();
	}
	buffer[buf_chars] = '\0';
	if(debug){
		printf("Stub: readCommand(char *)\n");

		int i;
		for(i=0; i<buf_chars; i++) {
			printf("buffer[%i] = %c\n", i, buffer[i]);
		}
		printf("\nlength: %i\n", buf_chars-1);
		printf("\n1. buffer %s\n", buffer);
		printf("2. buffer[%i] = %c\n", buf_chars-2, buffer[buf_chars-2]);
		if(buffer[buf_chars-1] == '\n')
			printf("3. buffer[%i] = '\\n'\n", buf_chars-1);
		if(buffer[buf_chars] == '\0')
			printf("4. buffer[%i] = '\\0'\n", buf_chars);
	}
	return 0;
}

int executeFileInCommand(char * commandName, char * argv[], char * filename) {
	int pipefd[2];

	if(pipe(pipefd)) {
		perror("pipe");
		exit(127);
	}

	switch(fork()) {
	case -1:
		perror("fork()");
		exit(127);
	case 0:
        close(pipefd[0]);  /* the other side of the pipe */
        dup2(pipefd[1], 1);  /* automatically closes previous fd 1 */
        close(pipefd[1]);  /* cleanup */
 	   FILE * pFile;
 	   char mystring;

 	   pFile = fopen (filename , "r");
 	   if (pFile == NULL) perror ("Error opening file");
 	   else {

 		while ((mystring=fgetc(pFile)) != EOF) {
 				putchar(mystring); /* print the character */
 			}
 	     fclose (pFile);
 	   }
 	  exit(EXIT_SUCCESS);

    default:

        close(pipefd[1]);  /* the other side of the pipe */
        dup2(pipefd[0], 0);  /* automatically closes previous fd 0 */
        close(pipefd[0]);  /* cleanup */

        execve(commandName, argv, 0);
        perror(commandName);
        exit(125);

	}

	return 0;
}


int executeFileOutCommand(char * commandName, char * argv[], char * filename) {
	int def = dup(1);

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
    if(file < 0)    return 1;

    if(dup2(file,1) < 0)    return 1;
    int pid;
    pid = fork();
    if( pid == 0) {
    	close(file);
    	close(def);
    	execve(commandName, argv,0);
    	return 0;
    }
    dup2(def, 1);
    close(file);
    close(def);
    wait(NULL);
 	close(file);
	return 0;
}


void executePipedCommand(char *argvA[], char  *argvB[], char * nameA, char * nameB) {
	int pipefd[2];

	if(pipe(pipefd)) {
		perror("pipe");
		exit(127);
	}

	switch(fork()) {
	case -1:
		perror("fork()");
		exit(127);
	case 0:
        close(pipefd[0]);  /* the other side of the pipe */
        dup2(pipefd[1], 1);  /* automatically closes previous fd 1 */
        close(pipefd[1]);  /* cleanup */
        /* exec ls */
        execve(nameA, argvA, 0);
        perror(nameA);
        exit(126);
    default:
       
        close(pipefd[1]);  /* the other side of the pipe */
        dup2(pipefd[0], 0);  /* automatically closes previous fd 0 */
        close(pipefd[0]);  /* cleanup */
        /* exec tr */
        execve(nameB, argvB, 0);
        perror(nameB);
        exit(125);

	}
}

struct command_t command;

void clearScreen() {
	printf("\033[2J\033[1;1H");
}

void self() {
	printf("self...\n");
}


void changeDir() {
	if (command.argv[1] == NULL) {
		chdir(getenv("HOME"));
	} else {
		if (chdir(command.argv[1]) == -1) {
			printf(" %s: no such directory\n", command.argv[1]);
		}
	}
}


int checkInternalCommand() {

	if(strcmp("cd", command.argv[0]) == 0) {
		changeDir();
		return 1;
	}
	if(strcmp("clear", command.argv[0]) == 0) {
		clearScreen();
		return 1;
	}
	if(strcmp("self", command.argv[0]) == 0) {
		clearScreen();
		return 1;
	}

	return 0;
}


int excuteCommand() {

	int child_pid;
	int status;
	pid_t thisChPID;


	child_pid = fork();
	if(child_pid < 0 ) {
		fprintf(stderr, "Fork fails: \n");
		return 1;
	}
	else if(child_pid==0) {
		/* CHILD */
		execve(command.name, command.argv,0);
		printf("Child process failed\n");
		return 1;
	}
	else if(child_pid > 0) {
		/* PARENT */

		do {
			thisChPID = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
            if (thisChPID == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) {
                return 0;
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		return 0;
	}
    return 0;

}

int processPipedCommand(int i) {
	char *argvA[5];
	char *argvB[5];
	char *nameA, *nameB;

	int ii;
	for(ii=0;ii<i;ii++) {
		argvA[ii] = command.argv[ii];
	}
	argvA[ii]=NULL;
	nameA = lookupPath(argvA, pathv);

	int j,jj=0;
	for(j=i+1; j<command.argc; j++) {
		argvB[jj] = command.argv[j];
		jj++;
	}
	argvB[jj]=NULL;
	nameB = lookupPath(argvB, pathv);

	int pid, status;
	fflush(stdout);

    switch ((pid = fork())) {
    case -1:
        perror("fork");
        break;
    case 0:
        /* child */
    	executePipedCommand(argvA, argvB, nameA, nameB);
        break;  /* not reached */
    default:
        pid = wait(&status);
        return 0;
    }
    return 1;
}

int processFileOutCommand(int i) {

	char *argv[5];
	char *commandName;
	int j;
	for(j=0; j<i; j++) {
		argv[j] = command.argv[j];
	}
	argv[j] = NULL;
	commandName = lookupPath(argv, pathv);

	return executeFileOutCommand(commandName, argv, command.argv[i+1]);
}


int processFileInCommand(int i) {
	char *argv[5];
	char *commandName;

	int j;
	for(j=0; j<i; j++) {
		argv[j] = command.argv[j];
	}
	argv[j] = NULL;
	commandName = lookupPath(argv, pathv);

	int pid, status;
	fflush(stdout);

    switch ((pid = fork())) {
    case -1:
        perror("fork");
        break;
    case 0:
        /* child */
    	executeFileInCommand(commandName, argv, command.argv[i+1]);
        break;  /* not reached */
    default:
        pid = wait(&status);
        return 0;
    }

	return 0;
}

int processCommand() {

	int i;
	int infile=0, outfile=0, pipeLine=0;
	char *outFileName;
	char *inFileName;
	for(i=0;i<command.argc; i++) {
		if(strcmp(command.argv[i], ">") == 0) {
			return processFileOutCommand(i);
		}
		else if(strcmp(command.argv[i], "<") == 0) {
			return processFileInCommand(i);

		}
		else if(strcmp(command.argv[i], "|") == 0) {
		    return processPipedCommand(i);
		}
	}
	return excuteCommand();
}

int main(int argc, char* argv[]) {
	int i;
	int debug = 0;

	parsePath(pathv);
	welcomeMessage();

	while(TRUE) {
		printPrompt();

		commandInput = getchar(); //gets 1st char
		if(commandInput == '\n') { // if not input print prompt
			continue;
		}
		else {
			readCommand(commandLine, &commandInput); // read command

			if((strcmp(commandLine, "exit") == 0) || (strcmp(commandLine, "quit") == 0))
				break;

			parseCommand(commandLine, &command); //parses command into argv[], argc

			if(checkInternalCommand() == 0) {
				command.name = lookupPath(command.argv, pathv);

				if(command.name == NULL) {
					printf("Stub: error\n");
					continue;
				}

				processCommand();
			}
		}
	}

	printf("\n");
	exit(EXIT_SUCCESS);
}