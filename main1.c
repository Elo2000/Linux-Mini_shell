#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(void){

  char cwd[250], prompt[300];
  char * args[100];
  int nargs;

  //counters for final statistics
  int num_commands = 0;
  int num_words = 0;
  int stop = 0;

  //create the prompt
  getcwd(cwd, sizeof(cwd));
  snprintf(prompt, sizeof(prompt), "%s>", cwd);

  while(!stop){

    //show prompt and get user line
    char * line = NULL;
    size_t line_size = 0;

    printf("%s", prompt);
    ssize_t line_len = getline(&line, &line_size, stdin);
    if(line_len < 0){
      printf("\n");
      break;
    }else if(line_len == 0){
      continue;
    }

    //split line into arguments
    nargs = 0;
    args[nargs] = strtok(line, " \t\r\n");
    while(args[nargs]){
      args[++nargs] = strtok(NULL, " \t\r\n");
    }
    args[++nargs] = NULL;

    if(nargs == 1){ //if empty line was entered
      free(line);
      continue;
    }

    ++num_commands;
    num_words += nargs - 1;

    if(strcmp(args[0], "done") == 0){ //done command
      stop = 1;
    }else if(strcmp(args[0], "cd") == 0){ //cd command
      printf("command not supported (Yet)\n");
    }else{  //external command

      const pid_t pid = fork();
      if(pid == 0){
        execvp(args[0], args);
        perror("execvp");
        exit(1);
      }else{
        int status;
        if(waitpid(pid, &status, 0) == -1){
          perror("waitpid");
          break;
        }
      }
    }
    free(line);
  }

  printf("Num of commands: %d\n", num_commands);
  printf("Total number of words in all commands: %d !\n", num_words);

  return 0;
}