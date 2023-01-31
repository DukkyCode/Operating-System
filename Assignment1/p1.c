#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define buffer_max 32;

/*Extracting tokens from terminal input*/
char **extract_token(char *line){
    /*Declare Variables*/
    int buffer = buffer_max;
    int i = 0;                                                      //Counter
    char *delim = " \t\r\n";                                        //Delimiter for strtok
    
    char **tokens = malloc(buffer * sizeof(char*));
    char *token;
    
    if(tokens == NULL){
        fprintf(stderr, "Failed to allocate memory");
        exit(1);
    }

    token = strtok(line, delim);
    while(token != NULL){                                           
        //Grab the token and copy it to an array
        tokens[i] = token;
        i++;

        if(i >= buffer){
            buffer += buffer;
            tokens = realloc(tokens, buffer * sizeof(char*));
            //Print statement if fail to allocate memory
            if(tokens == NULL){
                fprintf(stderr, "Fail to allocate memory");
                exit(1);
            }
        }
        token = strtok(NULL, delim);
    }
    //Null terminated the string for execvp;
    tokens[i] = NULL;

    return tokens;
}

/*Command Argument Line*/
void process_exec(char **args){
    /*Variables*/
    int status;
    //int cexit_status;
    int exit_status;

    char *exit_name = "exit";
    
    if(strcmp(args[0], exit_name) == 0){
        exit(0);
        return;
    }

    pid_t rc  = fork();

    if(rc < 0){
        //fork failed
        fprintf(stderr, "Fail to create process\n");
        exit(1);
    }
    else if(rc == 0){
        //printf("Executing Process ID: %d\n", (int) getpid());
        if(execvp(args[0], args) == -1){
            //cexit_status = 127;
            //printf("jsh status: %d\n", cexit_status);
            exit(127);            
        }
    }
    else{
        waitpid(rc, &status, 0);

        if(WIFEXITED(status)){
            exit_status = WEXITSTATUS(status);
            printf("jsh status: %d\n", exit_status);
        }       
    }    
}

/*Main Function*/
int main(int argc, char const *argv[])
{
    /*Set up void variables*/
    (void)argc;
    (void)argv;

    /*Main Loop*/    
    do{
        /*Variables*/
        char *prompt = "jsh$";    
        char *line = NULL;
        char **tokens;
        size_t size = 0;
        
        /*Get Input from the user*/
        printf("%s ", prompt);
        getline(&line ,&size, stdin);
        
        // /*Call a function split the input into arguments */
        tokens = extract_token(line);        
        process_exec(tokens);

        //printf();
        free(line);
        free(tokens);

    }
    while(1);

    return 0;
}



