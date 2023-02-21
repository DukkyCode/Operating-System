#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*Name: Duc Anh Nguyen*/

/*Command Argument Line*/
void process_exec(char *args[100][100], int process_num){
    /*Variables*/
    int status;
    //int exit_status;
    int i = 0;
    int j = 0;
    int fd[process_num][2];
    char *exit_name = "exit";
    pid_t pids[100];


    //Built-in Commands -- Check for Exit
    for(j = 0; j <= process_num; j++){
        if(strcmp(args[j][0], exit_name) == 0){
            exit(0);
            return;
        }
    }
    
    //Parent process create necessaray pipes
    for(i = 0; i < process_num; i++){
        if(pipe(fd[i]) < 0){
            //fork failed
            fprintf(stderr, "Fail to create a pipe between processes\n");
            exit(1);
        }
    }

    //Child Process
    for(i = 0; i <= process_num; i++){

        pids[i] = fork();
        
        if(pids[i] < 0){
            //fork failed
            fprintf(stderr, "Fail to create process\n");
            exit(1);
        }
        else if(pids[i] == 0){
            if(i == process_num){
                //First Argument
		if(i == 0){
                    dup2(fd[i][1], STDOUT_FILENO);			
                }
		//Last Argument
                if(i != 0){
                    dup2(fd[i - 1][0], STDIN_FILENO);
                }
            }
	    //Argument in between
            else{
                dup2(fd[i][1], STDOUT_FILENO); 
                dup2(fd[i - 1][0], STDIN_FILENO);           
            }
            
            //Close the first 2 pipe of the Child
            for(j = 0; j < process_num; j ++){
                close(fd[j][0]);
		close(fd[j][1]);                
            }

            //Executing Process
            if(execvp(args[i][0], args[i]) == -1){
                exit(127);
                return;
            }
        }
    }
   
    for(i = 0; i < process_num; i ++){
        close(fd[i][0]);
        close(fd[i][1]);
    }

    //Parent Process Waiting for all Child processes
    for(i =0; i <= process_num; i++){
        waitpid(pids[i], &status, 0);    
    }
    
    //Print Error Statement
    if(WIFEXITED(status)){
        printf("jsh status: %d\n", WEXITSTATUS(status));
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
	
	if(line[0] != '\n'){
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
    }        
          
    return 0;
}



