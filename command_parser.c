#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_execute.h"

char** parse(char *command){

	int command_length = strlen(command)-1;

	//remove trailing and whitespaces
	char *clean_command = malloc(command_length * sizeof(char));
	int index = 0;
	for(int i=0;i<command_length;i++){
		if(i==0 && command[i]==' '){

		}
		else if(i==0 && command[i]!=' '){
			clean_command[index]=command[i];
			index++;
		}
		else if(command[i] != ' ' || command[i]==' ' && command[i-1]!=' '){
			clean_command[index]=command[i];
			index++;
		}
	}
	if(index > 0 && clean_command[index-1]==' '){
		clean_command[index-1]='\0';
	}

	//tokenization
	char **token_list = malloc(sizeof(char *));
	int token_list_size = 1;

	char *token;
	token = strtok(command," ");
	
	index = 0;
	while(token != NULL){
		*(token_list + index) = malloc((strlen(token)) * sizeof(char));
		*(token_list + index) = token;
		index++;
		if(token_list_size>1){
			token_list = realloc(token_list, (token_list_size + 1) * sizeof(char *));
		}
		token_list_size++;
		token = strtok(NULL, " ");
	}	
	token_list[token_list_size-2][strlen(token_list[token_list_size-2])-1] = '\0';

	return token_list;
}

void execute(char *command){
	char** token_list = parse(command);
	execute_command(token_list);
}