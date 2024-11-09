#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "variables.h"

#define number_of_commands 3

char* commands[] = {"exit", "help", "analyze"};

char command_list_string[] = "\nCommands:\n\nexit: exit shell\nhelp: print this message\nanalyze [filename] analyze the file with name [filename]\n\n";

char* current_command;

int translate_command(char* current_command){
	for(int i=0;i<number_of_commands;i++){
		if(!strcmp(commands[i],current_command)){
			return i;
		}
	}
	return -1;
}

void execute_command(char** token_list){
	current_command = token_list[0];
	switch(translate_command(current_command)){
	case 0:
		{
			state = 0;
		} break;
	case 1:
		{
			printf("%s",command_list_string);
		} break;
	case 2:
		{
			if(access(token_list[1],F_OK)==0){
				printf("File exists\n");
			}
			else{
				printf("File does not exist\n");
			}
		} break;
	default:
		{
			printf("Command not found\n");
		}
	}
}