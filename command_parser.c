#include <stdio.h>

void parse(char *command){

	//calculating command length
	int command_length = 0;
	for(int i=0;command[i]!=0;i++){
		command_length++;
	}
	command_length--;

	//remove trailing and whitespaces
	char clean_command[command_length];
	int index = 0;
	for(int i=0;i<command_length;i++){
		if(i==0 && command[i]==' '){

		}
		else if(i==0 && command[i]!=' '){
			clean_command[index]=command[i];
			index++;
		}
		else if(command[i]!=' ' || command[i]==' ' && command[i-1]!=' '){
			clean_command[index]=command[i];
			index++;
		}
	}
}

void execute(char *command){
	parse(command);
}