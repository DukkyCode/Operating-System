#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*Name: Duc Anh Nguyen*/
/*Command Argument Line*/
void process_exec(char *args[300][300], int process_num){
    /*Variables*/
    int status;
    //int exit_status;
    int i = 0;
    int j;
    int fd[process_num][2];
    int pids[100];

    //Exit Character
    char *exit_name = "exit";

    //Built-in Commands -- Check for Exit
    for(j = 0; j <= process_num; j++){
        if(strcmp(args[j][0], exit_name) == 0){
            exit(0);
            return;
        }
    }

    //Initialize the pipe array
    memset(fd, 0, sizeof(fd));

    //Parent process create necessaray pipes
    for(i = 0; i < process_num; i++){
        if(pipe(fd[i]) < 0){
            //fork failed
            fprintf(stderr, "Fail to create a pipe between processes\n");
            exit(1);
        }
    }

    // Child Process
    for (i = 0; i <= process_num; i++) {

	pids[i] = fork();

    	if (pids[i] < 0) {
            // fork failed
            fprintf(stderr, "Fail to create process\n");
            exit(1);
    	}
	else if (pids[i] == 0) {
	    // Close all unnecessary pipes
            for (int j = 0; j < process_num; j++) {
            	if (j != i && j != i - 1) {
                    close(fd[j][0]);
                    close(fd[j][1]);
                }
            }

            // Redirect input from previous pipe
            if (i > 0) {
		close(fd[i-1][1]);
            	dup2(fd[i-1][0], STDIN_FILENO);
            	close(fd[i-1][0]);
             }

            // Redirect output to current pipe
            if (i < process_num) {
		close(fd[i][0]);
            	dup2(fd[i][1], STDOUT_FILENO);
            	close(fd[i][1]);
            }
            // Execute process
            if (execvp(args[i][0], args[i]) == -1) {
            	printf("error: Command not found: %s\n", args[i][0]);
		exit(127);
            }
    	}
    }

    //Parent Process close all necessary pipe
    for (i = 0; i < process_num; i++){
        close(fd[i][0]);
        close(fd[i][1]);
    }

    //Parent Process Waiting for all Child processes
    for(i = 0; i <= process_num; i++){
	waitpid(pids[i], &status, 0);
    }

    //Print Error Statement
    if(WIFEXITED(status)){
        printf("jsh status: %d\n", WEXITSTATUS(status));
    }
}

//Function to remove leading space and trailing space on either side of the pipe
void trim(char *str) {
    char *start, *end;
    size_t len;

    len = strlen(str);
    if (len == 0) return;

    // Trim leading spaces
    start = str;
    while (*start && isspace(*start)) start++;

    // Trim trailing spaces
    end = str + len - 1;
    while (end > start && isspace(*end)) end--;

    // Add null terminator
    *(end + 1) = '\0';

    // Shift trimmed string to start of input string
    memmove(str, start, end - start + 2);
}


/*Main Function*/
int main(int argc, char const *argv[])
{

    /*Set up void variables*/
    (void)argc;
    (void)argv;
    int i, j, k;
    char *delim = " \n";                                        //Delimiter for strtok
    //char *pip_char = "|";

    char *tokens[300][300];
    char *pip_token[300];
    char *token;
    /*Variables for prompt*/
    char *prompt = "jsh$";
    char line[300];

    /*Main Loop*/
    while (1)
    {
	i = 0;

        /*Get Input from the user*/
        printf("%s ", prompt);
        fgets(line, sizeof(line), stdin);

	if(line[0] != '\n'){
		token = strtok(line, "|");
		//Tokenize inputs by pipes
		while(token != NULL){
			trim(token);
			pip_token[i] = token;
			i++;
			token = strtok(NULL, "|");
		}

		//Testing the token input
		//for(j = 0; j < i; j++){
			//printf("%s\n", pip_token[j]);

		//}

		//Tokenize the inputs by argumennts
		for(j = 0; j < i; j++){
			k = 0;
			token = strtok(pip_token[j], delim);
			while(token != NULL){
				tokens[j][k] = token;
				k++;
				token = strtok(NULL, delim);
			}
		}

		//Testing
		//for(j = 0; j < i; j++){
                        //k = 0;
			//while(tokens[j][k] != NULL){
				//printf("%s\n", tokens[j][k]);
				//k++;
			//}
			//printf("\n");

                //}


		process_exec(tokens, i - 1);

		//Testing the input token
                //for(j = 0; j < i; j++){
                        //printf("%s\n", tokens[j][0]);
                //}

		// Reset the tokens array
		for (j = 0; j < 100; j++) {
    			for (k = 0; k < 100; k++) {
        			tokens[j][k] = NULL;
    			}
		}


	}
    }

    return 0;
}



