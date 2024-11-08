#include <stdio.h>
#include <string.h>
#include "variables.h"

#define number_of_commands 1

char* commands[] = {"exit"};

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
	}
}