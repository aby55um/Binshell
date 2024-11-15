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

void little_endian_read(unsigned char* list,int index,int size, int format){
	int zero_byte = 0;
	for(int i=0;i<size;i++){
		if(list[index-i]!=0){
			zero_byte = 1;
		}
		if(zero_byte!=0){
			if(!format){
				printf("%x",list[index-i]);
			}
			else {
				printf("%d",list[index-i]);
			}
		}
	}
	printf("\n");
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

					int index;

					// Program entry offset
					printf("\nProgram entry: 0x");
					if(b64){
						index = 25;
					} else if(b32){
						index = 21;
					}
					little_endian_read(remaining_header,index,address_size,0);
					
					// Program header offset
					printf("Program header table: 0x");
					if(b64){
						index = 33;
					} else if(b32){
						index = 25;
					}
					little_endian_read(remaining_header,index,address_size,0);
					
					// Section header table offset
					printf("Section header table: 0x");
					if(b64){
						index = 41;
					} else if(b32){
						index = 29;
					}
					little_endian_read(remaining_header,index,address_size,0);

					// Number of program header entries
					printf("Number of program header entries: ");
					if(b64){
						index = 51;
					} else if(b32){
						index = 39;
					}
					little_endian_read(remaining_header,index,2,1);

					// Number of section header entries
					printf("Number of section header entries: ");
					if(b64){
						index = 55;
					} else if(b32){
						index = 41;
					}
					little_endian_read(remaining_header,index,2,1);

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