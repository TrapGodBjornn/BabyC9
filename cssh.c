#define _POSIX_C_SOURCE 200809L // required for strdup() on cslab
#define _DEFAULT_SOURCE // required for strsep() on cslab
#define _BSD_SOURCE // required for strsep() on cslab

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define MAX_ARGS 32

char **get_next_command(size_t *num_args)
{
    printf("cssh$ ");
    char *line = NULL;
    size_t len = 0;
    getline(&line, &len, stdin);
    if (ferror(stdin))
    {
        perror("getline");
        exit(1);
    }
    if (feof(stdin))
    {
	free(line);
        return NULL;
    }

   
    char **words = (char **)malloc(MAX_ARGS*sizeof(char *));
    int i=0;

    char *parse = line;
    while (parse != NULL)
    {
        char *word = strsep(&parse, " \t\r\f\n");
        if (strlen(word) > 0)
        {
            words[i++] = strdup(word);
        }
    }
    *num_args = i;
    for (; i<MAX_ARGS; ++i)
    {
        words[i] = NULL;
    }

    free(line);

    return words;
}

void free_command(char **words)
{
    for (int i=0; words[i] != NULL; ++i)
    {
        free(words[i]);
    }
    free(words);
}

void execute_command(char **args, size_t num_args)
{
	if(num_args == 0) return;

	if (strcmp(args[0], "exit") == 0)
	{
		exit(0);
	}

	pid_t pid = fork();
	if (pid== -1)
	{
		perror("fork");
		return;
	}
	if (pid== 0)
	{
		for (int i= 0; i < num_args; i++)
		{
			if (strcmp(args[i], "<")== 0)
			{
				int fd = open(args[i+1], O_RDONLY);
				if (fd== -1)
				{
					perror("open");
					exit(1);
				}
				dup2(fd, STDIN_FILENO);
				close(fd);
				args[i] = NULL;
				break;
			}
			else if (strcmp (args[i], ">") == 0 || strcmp(args[i], ">>")== 0)
{
				int flags= O_WRONLY | O_CREAT | (strcmp(args[i], ">>")== 0 ? O_APPEND : O_TRUNC);
				int fd = open(args[i+1], flags, 0644);
				if (fd ==-1)
				{
					perror("open");
					exit(1);
				}
				dup2(fd, STDOUT_FILENO);
				close(fd);
				args[i]= NULL;
				break;
			}
		}
		execvp(args[0], args);
		perror("execvp");
		exit(1);
	}
	else
	{
		waitpid(pid, NULL, 0);
	}
}


int main()
{
    size_t num_args;

    char **command_line_words = get_next_command(&num_args);
    while (command_line_words != NULL)
    {
        execute_command(command_line_words, num_args);
        free_command(command_line_words);
        command_line_words = get_next_command(&num_args);
    }
    if (command_line_words !=NULL)
    {
    free_command(command_line_words);
    }
    return 0;
}
