#include <stdio.h>
#include <stdlib.h>

#define CMD_SIZE 100

void input_loop(){
	while(1){

		int buf_size = CMD_SIZE * sizeof(char);
		char *command = malloc(buf_size);

		printf(">>> ");
		fgets(command,buf_size,stdin);
	}
}	