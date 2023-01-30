#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define buffer_max 32;


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

int process_exec(char **args){
    
    int rc  = fork();

    if(rc < 0){
        //fork failed
        fprintf(stderr, "Fail to create process");
        exit(1);
    }
    else if(rc == 0){
        execvp(args[0], args);
    }
    else{
        //int rc_wait = wait(NULL);
        printf("Executing Process \n");
    }

    return 1;
}


/*Main Function*/
int main(int argc, char const *argv[])
{
    /*Set up void variables*/
    (void)argc;
    (void)argv;
    
    /* Main code */
    /* Variables */
    char *prompt = "jsh$";    
    char *line = NULL;
    size_t size = 0;
    
    char **tokens;  
    int status;

    do{
        /*Get Input from the user*/
        printf("%s ", prompt);
        getline(&line ,&size, stdin);
        
        /*Call a function split the input into arguments */
        tokens = extract_token(line);
        status = process_exec(tokens);


        free(line);
        free(tokens);

    }
    while(status != 0);

    return 0;
}



