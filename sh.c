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

  while ( go )
  {
  /* Put PATH into a linked list */
    pathlist = get_path();

    prompt[0] = ' '; prompt[1] = '\0';


    /* print your prompt */
    printf("%s>>", getcwd(NULL, PATH_MAX+1));

    /* get command line and process */

    fgets(commandline, MAX_CANON, stdin);
    command = calloc(MAX_CANON, sizeof(char));
    command = strtok(commandline, " ");//Gets the first word they type, which will be the command
//    printf("The command is %s\n", command);
    argsct = 0;
    args[argsct] = command;
    arg = strtok(NULL, " ");
    while(arg != NULL){
        argsct++;
        args[argsct] = arg;
//        printf("arg %d is %s\n", argsct, arg);
        arg = strtok(NULL, " ");
    }

    args[argsct][strlen(args[argsct]) - 1] = '\0';

    if(access(command, F_OK) == 0){ // Checks to see if command is a direct path, and if so then execute
	execve(command, args, envp);
    }
    /* check for each built in command and implement */
    //EXIT COMMAND
    if(strcmp(command,"exit") == 0){
	free(prompt);
	free(commandline);
	free(args);
	exit(2);
    }
    //WHICH COMMAND
    else if(strcmp(command,"which") == 0){
	if(argsct != 1){
	    printf("usage: %s [command]", command);
	    continue;
	}
	printf("%s\n", which(args[1], pathlist));
    }
    //WHERE COMMAND
    else if(strcmp(command,"where") == 0){
        if(argsct != 1){
            printf("usage: %s [command]", command);
            continue;
        }
        where(args[1], pathlist);
    }
    //PROMPT COMMAND
    else if(strcmp(command,"prompt") == 0){ 
	if(argsct == 1){
	    strcpy(prompt, args[1]);
	}
	if(prompt[0] == ' ' && prompt[1] == '\0'){
            printf("Input prompt prefix: ");
            fgets(prompt, PROMPTMAX, stdin);
        }
        printf("%s\n", prompt);
    }
    //LIST COMMAND
    else if(strcmp(command,"list") == 0){
        if(argsct != 0){
	    printf("usage: %s [command]");
            continue;
        }
        list(homedir);
    }
    //CD COMMAND
    else if (strcmp(command, "cd") == 0){
	char *temp;
	temp = calloc(PATH_MAX+1, sizeof(char));
	if(args[1] == NULL){    
	    memcpy(temp, pwd, strlen(pwd));
	    memcpy(pwd, homedir, strlen(homedir));
	    memcpy(owd, temp, strlen(temp));
	    chdir(pwd);
	}
	else if(args[1] == "-"){
	    printf("%s\n", pwd);
	    memcpy(temp, pwd, strlen(pwd));
            memcpy(pwd, owd, strlen(owd));
            memcpy(owd, temp, strlen(temp));
            chdir(pwd);
	}
	else{
 	    owd = getcwd(NULL, PATH_MAX + 1);
	    owd = strcat(owd, "/");
	    owd = strcat(owd, argv[1]);
	    chdir(owd);	    
	}
	//printf("Changing directory...\n");
    }

    else if (strcmp(command, "pid") == 0){
        pidd();
    }
     /*  else  program to exec */
       /* find it */
     /* do fork(), execve() and waitpid() */
    else if(which(command, pathlist) != NULL){
	fork();	
	execve(which(command, pathlist), args, envp);
	waitpid();
    }
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
        strcpy(commandtemp, command);
	strcpy(commandpath, "");
	strcpy(commandpath, pathlist->element);
        commandpath = strcat(commandpath, "/");
        commandpath = strcat(commandpath, commandtemp); //Adds the / to the PATH element and$
	pathlist = pathlist->next;	
        if(access(commandpath, F_OK) == 0){ //checks if the commandpath exists (0 is goo$
	    return commandpath;
        }
    }
    printf("There is no such command in the PATH. Exiting...\n");
    return NULL;

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
    char *commandpath = calloc(100, sizeof(char));
    char *commandtemp = calloc(strlen(command), sizeof(char));
    while(pathlist->next != NULL){
        strcpy(commandtemp, command);
	strcpy(commandpath, "");
        strcpy(commandpath , pathlist->element);
        commandpath = strcat(commandpath, "/");
        commandpath = strcat(commandpath, commandtemp); //Adds the / to the PATH element and$
        pathlist = pathlist->next;
        if(access(commandpath, F_OK) == 0){ //checks if the commandpath exists (0 is goo$
            printf("%s\n", commandpath);
        }
    }
    return NULL;

} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
    DIR *mydir;
    struct dirent *myfile;

    mydir = opendir(dir);
    while((myfile = readdir(mydir)) != NULL)
    {
        printf(" %s\n", myfile->d_name);
    }
    closedir(mydir);
} /* list() */
    
int pidd(){
    int p_id,p_pid;

    p_id=getpid();  /*process id*/
    p_pid=getpid(); /*parent process id*/

    printf("Process ID: %d\n",p_id);
    printf("Parent Process ID: %d\n",p_pid);
    return 0;
}
