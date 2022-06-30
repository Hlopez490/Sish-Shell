#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

#define READ  0
#define WRITE 1

static char* argv[512];
static char  history[100][80];
static int n = 0;
char s[100];

static int command(int input,int first,int last) {
    pid_t pid;
    int pipefd[2];
    pipe(pipefd);
    pid = fork();
    if (pid == 0) {
        if (first == 1 && last == 0 && input == 0) { 
	   
	   dup2(pipefd[WRITE], STDOUT_FILENO);
        
	}else if(first == 0 && last == 0 && input != 0) {
         
	   dup2(input, STDIN_FILENO);
	   dup2(pipefd[WRITE], STDOUT_FILENO);
        
	}else{ 
	    dup2(input, STDIN_FILENO);
    }
        if (execvp(argv[0], argv) == -1)
            _exit(EXIT_FAILURE);
    }

    if (input != 0)
        close(input);   
    close(pipefd[WRITE]);
    if (last == 1)
        close(pipefd[READ]);
    return pipefd[READ];
}

static void addHistory(const char *commandPointer, int *historyCount) {
  if (*historyCount == 100) {
    for (int i = 0; i < 99 ; i++)
          strcpy(history[i], history[i+1]);
  }else{
    strcpy(history[*historyCount], commandPointer);
    (*historyCount)++;
  }
}

static char* ignoreSpace(char* spaceCount){
    while (isspace(*spaceCount)) ++spaceCount;
    return spaceCount;
}

static void tokenize(char* commandPointer){

    commandPointer = ignoreSpace(commandPointer);
    char* next = strchr(commandPointer, ' ');

    int i = 0;
    while(next != NULL) {

        next[0] = '\0';
        argv[i] = commandPointer;
        ++i;

        commandPointer = ignoreSpace(next + 1);
        next = strchr(commandPointer, ' ');
    }

    if (commandPointer[0] != '\0') {
        argv[i] = commandPointer;
        next = strchr(commandPointer, '\n');
        next[0] = '\0';
        ++i;
    }
    argv[i] = NULL;
}

int main(){

    int historyCount = 0;
    char *buffer;
    size_t bufsize = 1024;
    buffer = (char *)malloc(bufsize * sizeof(char));
    while (1) {
    
        printf("sish> ");
        getline(&buffer, &bufsize, stdin);
        
        char* hist = strstr(buffer, "history");
        if ( hist != NULL ){
          
          char tmp[80] = "";
          
	        strcpy(tmp, buffer);
          char *hasParam = strchr(tmp,' ');
          
          char *temp = ignoreSpace(tmp);
          tmp[strlen(tmp)-1] = '\0';
          
          if (strcmp(temp,"history") == 0){
            int i;
            for ( i = historyCount - 1 ; i >= 0 ; i--)
                printf("%d %s",i, history[i]);
            addHistory(buffer, &historyCount);
            continue;
          }
          else if ( strcmp(temp,"history -c") == 0)
          {
            historyCount = 0;
            addHistory(buffer, &historyCount);
            continue;
          }
          
          if ( hasParam != NULL)
          {
            char param[16] = "";
            strcpy(param, hasParam+1);
            
            addHistory(buffer, &historyCount);
            
            int i, flag;
           
            for ( i  = 0 ; i < strlen(param); i++)
              if ( isdigit(param[i]) == 0 )
              {
                flag = 1;
                break;
              }
              
            if ( flag != 1 ){
              int index = atoi(param);
              
              if ( index > (historyCount-1) || index < 0 )
              continue;
              
              strcpy(buffer, history[index]);
              
            }
          }
        }
        
        if (strcmp(buffer,"\n") != 0 )
            addHistory(buffer, &historyCount);
        
        int input = 0;
        int first = 1;
        char* commandPointer = buffer;
        char* next = strchr(commandPointer, '|'); 

        while (next != NULL) {

            *next = '\0';
            commandPointer = ignoreSpace(commandPointer);
            char* nxtInput = strchr(commandPointer, ' ');
        
            int i = 0;
        
            while(nxtInput != NULL) {
        
                nxtInput[0] = '\0';
                argv[i] = commandPointer;
                ++i;
        
                commandPointer = ignoreSpace(nxtInput + 1);
                nxtInput = strchr(commandPointer, ' ');
            }
        
            if (commandPointer[0] != '\0') {
        
                argv[i] = commandPointer;
                nxtInput = strchr(commandPointer, '\n');
                nxtInput[0] = '\0';
                ++i;
        
            }
        
            argv[i] = NULL;
            if (argv[0] != NULL) {
                if (strcmp(argv[0], "exit") == 0)
                    exit(0);
        
                if (strcmp(argv[0], "cd") == 0)
                    return 0;
        
                n += 1;
                input = command(input, first, 0);
            }

            commandPointer = next + 1;
            next = strchr(commandPointer, '|'); 
            first = 0;
        }
        
        tokenize(commandPointer);

        if (argv[0] != NULL) {
            
            if (strcmp(argv[0], "exit") == 0) exit(0);
            
            if (strcmp(argv[0], "cd") == 0) {
                chdir(argv[0]);
                printf("%s\n", getcwd(s, 100));
            }
    
            n += 1;
            input = command(input, first, 1);
        }
        for (int i = 0; i < n; ++i)
            wait(NULL);
        n = 0;
        }
    return 0;
}



