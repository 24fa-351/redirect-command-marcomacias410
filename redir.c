#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

/*Usage: redir <inp> <cmd> <out> */

void add_character_to_string(char *str, char c) {
    int len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
}



// splits string by spaces; adds sa nULL into the array after the last word
void split (char * cmd, char * words[], char delimiter){
 

    int word_count = 0;
    char * next_char = cmd;
    char current_word[1000];
    strcpy(current_word, "");


    while (*next_char != '\0'){
       
       if (*next_char == delimiter){
        words[word_count++]  = strdup(current_word);
        strcpy(current_word, "");
       } else {
        add_character_to_string(current_word, *next_char);
       }
       ++next_char;

    }

    words[word_count++] = strdup(current_word);
    words[word_count] = NULL;
}


void break_into_words(char * cmd, char *words[]){

  
    words[0] = strdup(cmd);
    words[1] = NULL;
}

bool find_absolute_path( char * cmd, char *absolute_path){
  
    
      char * directories [1000];

      split(getenv("PATH"), directories, ':');
      // look in array until I find the paththing+cmd

      for (int ix = 0; directories[ix] != NULL; ix++){

        char path[1000];
        strcpy(path, directories[ix]);
        add_character_to_string(path, '/');
        strcat(path, cmd);

        if (access(path, X_OK) == 0){
            strcpy(absolute_path, path);
            return true;
        }


      }


    return false;

}




int main(int argc, char *argv[]) {

    char absolute_path[1000];
    char *words[1000];

    int fd_input;
    int fd_output;



    if (argc < 4) {
        fprintf(stderr, "Usage %s <inp> <cmd> <out> \n", argv[0]);
        return 1;
    }
    split(argv[2], words, ' ');

    if(words[0] == NULL){
        printf("No command found\n");
        return 1;
    }

    if (!find_absolute_path(words[0], absolute_path)) {
        printf("Command not found: %s\n", words[0]);
        return 1;
    }



    printf("argc %d", argc);

    if (strcmp(argv[1], "-") == 0) {
        fd_input = STDIN_FILENO;
    } else {
        fd_input = open(argv[1], O_RDONLY);
        if (fd_input == -1) {
            perror("Failed to open input file");
            return 1;
        }
        dup2(fd_input, STDIN_FILENO);
        close(fd_input);
    }


    // Check if last argument is a text file 
    if (strcmp(argv[argc - 1], "-") == 0) {
        fd_output = STDOUT_FILENO;
    } else {
        fd_output = open(argv[argc - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_output == -1) {
            perror("Failed to open output file");
        }
    }

/*
    char **newargv = (char **)malloc(sizeof(char *) * (argc));

    for (int ix = 2; ix < argc - 1; ix++) {
        newargv[ix - 2] = (char *)argv[ix];
    }
    newargv[argc - 2] = NULL; */

    pid_t pid = fork();

    if (pid == 0) {
        if (fd_output != STDOUT_FILENO) {
            dup2(fd_output, STDOUT_FILENO);
            close(fd_output);
        }

       // execvp(newargv[0], newargv);
       execve(absolute_path, words, NULL);

        printf("Exec failed\n");
        _exit(1);
    }

    close(fd_output);

    int status;
    pid_t wpid = waitpid(pid, &status, 0);
    printf(
        "%s pid is %d. forked %d. "
        "Parent exiting\n",
        argv[0], getpid(), pid);

    return 0;
}