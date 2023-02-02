#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Command Argument Line*/
void process_exec(char *args[100][100], int process_num){
    /*Variables*/
    int status;
    int exit_status;
    int i = 0;
    int j = 0;
    char *exit_name = "exit";
    pid_t pids[100];
    pid_t wpid;
    

    //Built-in Commands -- Check for Exit
    for(j = 0; j <= process_num; j++){
        if(strcmp(args[j][0], exit_name) == 0){
            exit(0);
            return;
        }
    }

    //Child Process
    for(i =0; i <= process_num; i++){
        pids[i] = fork();
        if(pids[i] < 0){
            //fork failed
            fprintf(stderr, "Fail to create process\n");
            exit(1);
        }
        else if(pids[i] == 0){
            if(execvp(args[i][0], args[i]) == -1){
                exit(127);
            }
        }
    }

    //Parent Process Waiting for all Child processes
    while((wpid = waitpid(-1, &status, 0)) != -1){
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
    int i, j;
    char *delim = " \t\r\n";                                        //Delimiter for strtok
    //char *pip_char = "|";

    char *tokens[100][100];
    char *token;
    /*Variables for prompt*/
    char *prompt = "jsh$";    
    char line[100];   

    /*Main Loop*/    
    while(1){
        /*Variables for tokens*/
        i = 0;
        j = 0;

        /*Get Input from the user*/
        printf("%s ", prompt);
        fgets(line, sizeof(line), stdin);
               
        /*Extracting the tokens*/
        token = strtok(line, delim);
            
        while(token != NULL){
            tokens[i][j] = token;

            if(strcmp(tokens[i][j], "|") == 0){
                tokens[i][j] = NULL;
                i = i + 1;
                j = 0;
                token = strtok(NULL, delim);
            }
            else{
                j++;
                token = strtok(NULL, delim);
            }                     
        }
        tokens[i][j] = NULL;
        process_exec(tokens, i);
        
    }

    return 0;
}



