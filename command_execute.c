#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
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
				//rewind(file);  
				//unsigned char line[5];
				fgets(line, sizeof(line)+1, file);
				printf("first: %ld\n",ftell(file));
				//printf("line: %s",line);
				//fread(line,sizeof(line),1,file);
				//printf("line: %s", line);
				//Check if ELF file
				//printf("%c",line[0]);
				if(line[0]==0x7f && line[1]=='E' && line[2]=='L' && line[3]=='F'){
					printf("\nFile format: ELF\n");
					//Check if 32 or 64 bit file
					int b64 = 0;
					int b32 = 0;
					int address_size;
					char header_bytes[2];
					fgets(header_bytes,3,file);
					printf("second: %ld\n",ftell(file));
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
					printf("third: %ld\n",ftell(file));
					//fread(remaining_header, remaining_header_size, 1, file);

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
					printf("Program header table offset: 0x");
					if(b64){
						index = 33;
					} else if(b32){
						index = 25;
					}
					unsigned long long int program_header_offset = 0;
					for(int i=0;i<address_size;i++){
						program_header_offset += (unsigned long long int)remaining_header[index-address_size+1+i]*(unsigned long long int)pow(256,(double)(i));
					}
					little_endian_read(remaining_header,index,address_size,0);
					
					// Section header table offset
					printf("Section header table offset: 0x");
					if(b64){
						index = 41;
					} else if(b32){
						index = 29;
					}
					unsigned long long int section_header_table_offset = 0;
					for(int i=0;i<address_size;i++){
						section_header_table_offset += remaining_header[index-address_size+1+i]*pow(256,(long)i);
					}
					//printf("\n%lld\n",section_header_table_offset);
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

					// Section index to the section header string table
					printf("Section index to the section header string table: 0x");
					if(b64){
						index = 57;
					} else if(b32){
						index = 45;
					}
					little_endian_read(remaining_header,index,2,0);
					printf("\n");

					// Number of program header entries
					unsigned char ch_ptr_program_header_entry_num[2];
					if(b64){
						index = 56;
					} else if(b32){
						index = 44;
					}
					unsigned int program_header_entry_num = (unsigned int)remaining_header[index-6] + (unsigned int)remaining_header[index-7] * 256;

					// Number of section header entries
					unsigned int section_header_entry_num = 0;
					if(b64){
						index = 55;
					} else if(b32){
						index = 42;
					}
					section_header_entry_num = remaining_header[index-1] + remaining_header[index]*256;
					//printf("%d\n",section_header_entry_num);
					//little_endian_read(remaining_header,index,2,1);

					// Program header entry size
					unsigned char ch_ptr_program_header_entry_size[2];
					if(b64){
						index = 54;
					} else if(b32){
						index = 42;
					}
					unsigned int program_header_entry_size = (unsigned int)remaining_header[index-6] + (unsigned int)remaining_header[index-7] * 256;

					// Section header entry size
					unsigned long long int section_header_entry_size = 0;
					if(b64){
						index = 53;
					} else if(b32){
						index = 42;
					}
					section_header_entry_size = remaining_header[index-1] + remaining_header[index];
					//printf("Section header entry size: %lld\n",section_header_entry_size);
					//little_endian_read(remaining_header,index,2,1);

					// Todo: make it work for files that have program entry offset other than 64
					if(program_header_offset!=64){
						break;
					}

					unsigned char *program_header = malloc(program_header_entry_num * program_header_entry_size * sizeof(unsigned char));
					fgets(program_header, program_header_entry_num * program_header_entry_size ,file);

					unsigned long long int segment_types[program_header_entry_num];
					unsigned long long int segment_offset[program_header_entry_num];
					for(int i=0;i<program_header_entry_num;i++){
						segment_types[i]=0;
						segment_offset[i]=0;
					}

					for(int i=0;i<program_header_entry_num;i++){
						for(int j=0;j<4;j++){
							segment_types[i]+=program_header[program_header_entry_size*i+j]*(int)pow(256,(double)j);
							//printf("%x   ",program_header[program_header_entry_size*i+j]);
						}
						//printf("\n");
						printf("First four bytes of the %d header entry: 0x%llx\n",i,segment_types[i]);
					}

					// Close and reopen the file
					//fclose(file);		

					//file = fopen(token_list[1],"r");
					/*fseek(file,0,SEEK_END);
					long fsize = ftell(file);
					fseek(file, 0, SEEK_SET);
					printf("%ld",fsize);
					//rewind(file);

					unsigned char *file_content = malloc(fsize+1);
					fread(file_content,fsize,1,file);*/
					fclose(file);
					//fread(file_content,1,1,file);
					//fclose(file);

					// Section types
					
					/*for(int i=0;i<section_header_entry_num;i++){
						for(int j=0;j<4;j++){

						}
					}*/

					/*printf("Program header bytes:\n");
					for(int i=0;i<program_header_entry_num * program_header_entry_size;i++){
						printf(" %d: %x | ",i+64,program_header[i]);
					}			

					/*printf("\n\nTodo: Test for big endian files!\n");
					printf("Debug:\n");
					for(int i=0;i<remaining_header_size;i++){ 
						printf(" %d: %x|",i+6,remaining_header[i]);
					}*/
				}
				if(line[0]==0x23 && line[1]==0x21){
					printf("script");
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