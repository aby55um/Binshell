#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "variables.h"

#define number_of_commands 3

char* commands[] = {"exit", "help", "analyze"};

char command_list_string[] = "\nCommands:\n\nexit: exit shell\nhelp: print this message\nanalyze [filename]: analyze the file with name [filename]\n\n";

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
				FILE *file = fopen(token_list[1],"r");
				unsigned char line[4];  
				fgets(line, sizeof(line)+1, file);
				//Check if ELF file
				if(line[0]==0x7f && line[1]=='E' && line[2]=='L' && line[3]=='F'){
					printf("\nFile format: ELF\n");
					//Check if 32 or 64 bit file
					int b64 = 0;
					int b32 = 0;
					int address_size;
					char header_bytes[2];
					fgets(header_bytes,3,file);
					int remaining_header_size;
					if(header_bytes[0]==1){
						printf("32 bit\n");
						remaining_header_size = 46;
						b32 = 1;
						address_size = 4;
					}
					if(header_bytes[0]==2){
						printf("64 bit\n");
						remaining_header_size = 58;
						b64 = 1;
						address_size = 8;
					}
					if(header_bytes[1]==1){
						printf("Little endian\n");
					}
					if(header_bytes[1]==2){
						printf("Big endian\n");
					}
					unsigned char *remaining_header = malloc(remaining_header_size * sizeof(char));
					fgets(remaining_header, remaining_header_size + 1,file);

					// Program entry
					printf("Program entry: 0x");
					int index;
					if(b32){
						index = 21;
					}
					else if(b64){
						index = 25;
					}
					int zero = 0;
					for(int i=0;i<8;i++){
						if(remaining_header[25-i]!=0){
							zero = 1;
						}
						if(zero!=0){
							printf("%x",remaining_header[25-i]);
						}
					}
					printf("\n");

					printf("\n\nTodo: Test for big endian files!\n");
					printf("Debug:\n");
					for(int i=0;i<remaining_header_size;i++){ 
						printf(" %d: %x|",i+6,remaining_header[i]);
					}
					printf("\n");
				}

			}
			else{
				printf("File does not exist\n");
			}
		} break;
	default:
		{
			printf("\nCommand not found\n");
			printf("%s",command_list_string);
		}
	}
}