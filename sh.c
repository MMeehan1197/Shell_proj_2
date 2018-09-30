#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, pid, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

  while ( go )
  {
    /* print your prompt */
    printf("%s>>", homedir);
    /* get command line and process */

    fgets(commandline, MAX_CANON, stdin);
    command = calloc(MAX_CANON, sizeof(char));
    command = strtok(commandline, " ");//Gets the first word they type, which will be the command
    printf("The command is %s\n", command);
    argsct = 0;
    args[argsct] = command;
    arg = strtok(NULL, " ");
    while(arg != NULL){
        argsct++;
        args[argsct] = arg;
        printf("arg %d is %s\n", argsct, arg);
        arg = strtok(NULL, " ");
    }

    args[argsct][strlen(args[argsct]) - 1] = '\0';

    if(access(command, F_OK) == 0){ // Checks to see if command is a direct path, and if so then execute
	execve(command, args, envp);
    }
    /* check for each built in command and implement */
    if(strcmp(command,"exit") == 0){
	exit(2);
    }
    if(strcmp(command,"which") == 0){
	if(argsct != 1){
	    printf("usage: %s [command]", command);
	    break;
	}
	which(args[1], pathlist);
    }

     /*  else  program to exec */
       /* find it */
       /* do fork(), execve() and waitpid() */
      else
        fprintf(stderr, "%s: Command not found.\n", args[0]);
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
    char *commandpath = calloc(100, sizeof(char));
    char *commandtemp = calloc(strlen(command), sizeof(char));
    while(pathlist->next != NULL){
        commandtemp = command;
	commandpath = pathlist->element;
        commandpath = strcat(commandpath, "/");
        commandpath = strcat(commandpath, commandtemp); //Adds the / to the PATH element and$
	pathlist = pathlist->next;	
        printf("path->element is: %s\n", pathlist->element);
	printf("The command value is: %s\n", command);
        if(access(commandpath, F_OK) == 0){ //checks if the commandpath exists (0 is goo$
	    printf("FINAL VALUE: %s\n", commandpath);
	    pathlist = get_path();
	    return commandpath;
        }
    }
    printf("There is no such command in the PATH. Exiting...\n");
    return NULL;

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
    char *commandpath;
    commandpath = calloc(100, sizeof(char));
    char *result = "";
    while(pathlist->next != NULL){
        commandpath = strcat(pathlist->element, "/");
        commandpath = strcat(commandpath, command); //Adds the / to the PATH element and$
        if(access(commandpath, F_OK) == 0){ //checks if the commandpath exists (0 is goo$
	    printf("%s\n",commandpath);
	    strcat(result, commandpath);
        }
    }
    return result;
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */
