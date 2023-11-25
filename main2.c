#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

static int history_size = 0;
static int history_len  = 0;
static char ** history = NULL;

static void history_add(char * line){
    //add to history
    if(history_len >= history_size){
        history_size += 10;
        history = realloc(history, sizeof(char*)*history_size);
    }
    history[history_len++] = line;
}

static int load_history(const char * filename){

    //open the file in read mode
    FILE * fp = fopen(filename, "r");
    if(fp == NULL){
        return 0;
    }

    //read each line from file
    char * line = NULL;
    size_t line_size = 0;
    ssize_t line_len = 0;

    while((line_len =getline(&line, &line_size, fp)) > 0){
        //remove any newlines
        size_t len = strlen(line);
        if(line[len-1] == '\r') line[--len] = '\0';
        if(line[len-1] == '\n') line[--len] = '\0';

        //add the word to list
        history_add(line);
        line = NULL;
        line_size = 0;
    }
    free(line);
    fclose(fp);

    return 0;
}

//Save the words from list to file
static int save_history(const char * filename){
    int i;

    //open the file for writing
    FILE * fp = fopen(filename, "w");
    if(fp == NULL){
        return 0;
    }

    //read each line from file
    for(i = 0; i < history_len; ++i) {
        fprintf(fp, "%s\n", history[i]);
    }
    fclose(fp);

    return 0;
}

//check if the string is a number
static int is_number(const char * num){
    while(*num != '\0'){
        if((*num < '0') || (*num > '9')){
            return 0;
        }
        num++;
    }
    return 1;
}

int main(void){

    char cwd[250], prompt[300];
    char * args[100];
    int i, nargs;

    //counters for final statistics
    int num_commands = 0;
    int num_words = 0;
    int stop = 0;

    //create the prompt
    getcwd(cwd, sizeof(cwd));
    snprintf(prompt, sizeof(prompt), "%s>", cwd);

    load_history("history.txt");

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
        if(line[line_len-1] == '\n') line[--line_len] = '\0';

        if(line[0] == '!'){
            const int id   = atoi(&line[1]);
            const char * num = &line[1];
            if(!is_number(num)){
                printf("Error: '%s' is not a number\n", num);
                continue;
            }

            free(line);
            if(id < history_len){
                line = strdup(history[id]);
            }else{
                printf("Error: Invalid history number %d\n", id);
                continue;
            }

        }else{
            history_add(line);
        }


        //split line into arguments
        nargs = 0;
        args[nargs] = strtok(line, " \t\r\n");
        while(args[nargs]){
            args[++nargs] = strtok(NULL, " \t\r\n");
        }
        args[++nargs] = NULL;

        if(nargs == 1){ //if empty line was entered
            continue;
        }

        ++num_commands;
        num_words += nargs - 1;

        if(strcmp(args[0], "done") == 0){ //done command
            stop = 1;
        }else if(strcmp(args[0], "cd") == 0){ //cd command
            printf("command not supported (Yet)\n");

        }else if(strcmp(args[0], "history") == 0){

            for(i = 0; i < history_len; ++i) {
                printf("%5d\t%s\n", i, history[i]);
            }

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
    }

    save_history("history.txt");

    //clear the history
    for(i = 0; i < history_len; ++i) {
        free(history[i]);
    }
    free(history);

    printf("Num of commands: %d\n", num_commands);
    printf("Total number of words in all commands: %d !\n", num_words);

    return 0;
}