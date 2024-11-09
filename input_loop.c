//todo: implement reallocation

#include <stdio.h>
#include <stdlib.h>
#include "command_parser.h"
#include "variables.h"

#define CMD_SIZE 100

void input_loop(){
	while(state > 0){

		int buf_size = CMD_SIZE * sizeof(char);
		char *command = malloc(buf_size);

		printf("> ");
		fgets(command,buf_size,stdin);

		execute(command);
	}
}	