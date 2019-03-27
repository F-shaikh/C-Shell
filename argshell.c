// Faisal Shaikh

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include<sys/wait.h> 
#include <fcntl.h>

extern char ** get_args();


int main()
{
    int i;
    char ** args;

    while (1)
	{
		printf ("Command ('exit' to quit): ");
		args = get_args();
		for (i = 0; args[i] != NULL; i++)
		{
	    	printf ("Argument %d: %s\n", i, args[i]);
		}
		if (args[0] == NULL)
		{
	    	printf ("No arguments on line!\n");
		}
		// exit command
		else if ( !strcmp (args[0], "exit"))
		{
	    	printf ("Exiting...\n");
	    	exit(1);
		}
		// Special Character "cd"
		else if (strcmp(args[0] , "cd") == 0)
		{
			char cwd[1000];
   			getcwd(cwd, sizeof(cwd));
			if (args[1] == NULL)
			{
				chdir(cwd);
				getcwd(cwd, sizeof(cwd));
			}
			else
			{
				chdir(args[1]);
				getcwd(cwd, sizeof(cwd));
			}
       		printf("Current directory: %s\n", cwd);
		}	
		else
		{
			// Forking operation
			pid_t processID;
			int status;
			if ((processID = fork()) < 0)
			{
				perror("Error Forking \n");
				exit(1);
			}
			// Look for special characters in commands
			else if (processID == 0)
			{
				for (i = 0; args[i] != NULL; i++)
				{
					// Special Character "<"
					if (strcmp (args[i] , "<") == 0)
					{
						if (args[i+1] != NULL)
						{
							int filedesc = open(args[i+1], O_RDONLY);
							if (dup2(filedesc,0) < 0)
							{
								perror("Error dup2");
								exit(1);
							}
							close(filedesc);
							args[i] = NULL;
							args[i+1] = NULL;
						}
					}
					// Special Character ">"
					else if (strcmp (args[i] , ">") == 0)
					{
						if (args[i+1] != NULL)
						{
							int filedesc = open(args[i+1], O_CREAT | O_RDWR | O_TRUNC);
							if (dup2(filedesc,1) < 0)
							{
								perror("Error dup2");
								exit(1);
							}
						close (filedesc);
						args[i] = NULL;
						args[i+1] = NULL;
						}
					}
					// Special Character ">>"
					else if (strcmp (args[i] , ">>") == 0)
					{
						if (args[i+1] != NULL)
						{
							int filedesc = open(args[i+1], O_CREAT | O_RDWR | O_APPEND);
							if (dup2(filedesc,1) < 0)
							{
								perror("Error dup2");
								exit(1);
							}
						close (filedesc);
						args[i] = NULL;
						args[i+1] = NULL;
						}
					}
					// Special Character "|"
					else if (strcmp (args[i] , "|") == 0)
					{
						int pipeDesc[2];
						int processID;
						char ** copyParentArgs = args;
						char ** copyChildArgs = args;
						int status;

						for (int j = 0; strcmp(args[j],"|") == 0; j++)
						{
							copyParentArgs[j] = args[j];
						}

						for (int k = i + 1; args[k]!= NULL; k++)
						{
							copyChildArgs[k] = args[k];
						}

						pipe(pipeDesc);
						processID = fork();

						if (processID == 0)
						{						
							dup2(pipeDesc[1], 1);
							close(pipeDesc[0]);
							args[i] = NULL;
							execvp(args[0], copyParentArgs);
						}
						else
						{
							while(wait(&status) != processID);
							dup2(pipeDesc[0], 0);
							close(pipeDesc[1]);
							args[i] = NULL;
							args[i-1] = NULL;
							execvp(args[i+1], copyChildArgs);
						}
					}
					// Special Character ">&"
					else if (strcmp (args[i] , ">&") == 0)
					{
						if (args[i+1] != NULL)
						{
							int filedesc = open(args[i+1], O_CREAT | O_RDWR | O_TRUNC);
							if ((dup2(filedesc,1) < 0) && (dup2(filedesc,2) < 0))
							{
								perror("Error");
								exit(1);
							}
						close (filedesc);
						args[i] = NULL;
						args[i+1] = NULL;
						}
					}
					// Special Character ">>&"
					else if (strcmp (args[i] , ">>&") == 0)
					{
						if (args[i+1] != NULL)
						{
							int filedesc = open(args[i+1], O_CREAT | O_RDWR | O_APPEND);
							if ((dup2(filedesc,1) < 0) && (dup2(filedesc,2) < 0))
							{
								perror("Error");
								exit(1);
							}
						close (filedesc);
						args[i] = NULL;
						args[i+1] = NULL;
						}
					}
					// Special Character "|&"
					else if (strcmp (args[i] , "|&") == 0)
					{
						int pipeDesc[2];
						int processID;
						char ** copyParentArgs = args;
						char ** copyChildArgs = args;

						for (int j = 0; strcmp(args[j],"|") == 0; j++)
						{
							copyParentArgs[j] = args[j];
						}

						for (int k = i + 1; args[k]!= NULL; k++)
						{
							copyChildArgs[k] = args[k];
						}

						pipe(pipeDesc);
						processID = fork();

						if (processID == 0)
						{
							dup2(pipeDesc[1], 1);
							dup2(pipeDesc[1], 2);
							close(pipeDesc[0]);
							args[i] = NULL;
							execvp(args[0], copyParentArgs);
						}
						else
						{
							while(wait(&status) != processID);
							dup2(pipeDesc[0], 0);
							close(pipeDesc[1]);
							args[i] = NULL;
							args[i-1] = NULL;
							execvp(args[i+1], copyChildArgs);
						}
					}

					// Special Character ";"
					else if (strcmp (args[i] , ";") == 0)
					{
						int numCommands = 0;
						int processID;
						char ** copyParentArgs = args;
						char ** copyChildArgs = args;

						for (int i = 0; args[i] != NULL; i++)
						{
							if (strcmp(args[i] , ";") == 0)
							{
								numCommands++;
							}
						}

						for (int j = 0; strcmp(args[j],";") == 0; j++)
						{
							copyParentArgs[j] = args[j];
						}

						for (int k = i + 1; args[k] != NULL; k++)
						{
							copyChildArgs[k] = args[k];
						}

						while (numCommands > 0)
						{
							if (processID == 0)
							{
								args[i] = NULL;
								execvp(copyParentArgs[0], copyParentArgs);
								args[i-1] = NULL;
								while(wait(&status) != processID);
								execvp(copyChildArgs[0], copyChildArgs);
							}	
							numCommands--;
						}
					}
				}
				// execvp at the end 		
				if (execvp(args[0], args) < 0)
				{
					perror("Error");
					exit(1);
				}
			}	
			else
			{
				while(wait(&status) != processID);
			}	
		}
	}		
}
