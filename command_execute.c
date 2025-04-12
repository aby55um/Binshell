#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <capstone/capstone.h>
#include "variables.h"

//test
//#define CODE "\x55\x48\x8b\x05\xb8\x13\x00\x00"
//Todo: make the disassemble work

#define number_of_commands 3

char* commands[] = {"exit", "help", "analyze"};

char command_list_string[] = "\nCommands:\n\nexit: exit shell\nhelp: print this message\nanalyze [filename]: analyze the file with name [filename]\n\n";

char* current_command;

char* flags[8] = {"Access denied", "Execute", "Write", "Write, Execute", "Read", "Read, Execute", "Read, write", "Read, write, execute"};

int translate_command(char* current_command){
	for(int i=0;i<number_of_commands;i++){
		if(!strcmp(commands[i],current_command)){
			return i;
		}
	}
	return -1;
}

void little_endian_read(unsigned char* list,int index,int size, int format, int newline){
	int zero_byte = -1;
	for(int i=0;i<size;i++){
		if(list[index-i]!=0){
			zero_byte = i;
			break;
		}
	}
	for(int i=0;i<size;i++){
		if(i >= zero_byte && zero_byte >=0){
			if(!format){
				printf("%x",list[index-i]);
				if(list[index-i]==0){ printf("0");}
			}
			//bugged
			else {
				printf("%d",list[index-i]);
			}
		}
	}
	if(zero_byte == -1){printf("0");}
	if(newline){
		printf("\n");
	}
	else {printf("\t");}
}

int little_endian_calc(unsigned char* list, int index, int size){
	int sum = 0;
	for(int i=0;i<size;i++){
		sum += list[index - size + 1 + i] * pow(256,(double)i); 
	}	
	return sum;
}

int segment_data_64(unsigned char* list, int index, int little_endian){
	printf("Type: 0x");
	little_endian_read(list, index + 3, 4, 0, 0);
	printf("Flags: 0x");
	little_endian_read(list, index + 7, 4, 0, 0);
	printf("Segment offset in file: 0x");
	little_endian_read(list, index + 15, 8, 0, 0);
	printf("Segment size in file: 0x");
	little_endian_read(list, index + 39, 8, 0, 0);
	printf("Segment offset in memory: 0x");
	little_endian_read(list, index + 23, 8, 0, 0);
	printf("Segment size in memory: 0x");
	little_endian_read(list, index + 47, 8, 0, 1);
	return 0;
}

int section_data_64(unsigned char* list, int index, int little_endian){
	printf("Name index: %d\t",little_endian_calc(list, index + 3, 4));
	printf("Name: ");
	int x=0;
	char y = 
		list[little_endian_calc(
			list,
			little_endian_calc(list, 47, 8) 
			+ little_endian_calc(list, 63, 2) * (16 * list[59] + list[58]) 
			+ 31,
			8 
		) + little_endian_calc(list, index + 3, 4) + x]; 
	while(y!=0){
		printf("%c",y);
		x++;
		y=list[little_endian_calc(
			list,
			little_endian_calc(list, 47, 8) 
			+ little_endian_calc(list, 63, 2) * (16 * list[59] + list[58]) 
			+ 31,
			8 
		) + little_endian_calc(list, index + 3, 4) + x]; 
	}
	printf("\t");
	printf("Type: 0x");
	little_endian_read(list, index + 7, 4, 0, 0);
	printf("Memory address: 0x");
	little_endian_read(list, index + 23, 8, 0, 0);
	printf("File offset: 0x");
	little_endian_read(list, index + 31, 8, 0, 0);
	printf("Size in file: %d\n",little_endian_calc(list, index + 39, 8));
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

				FILE *fl = fopen(token_list[1],"r");
				fseek(fl, 0, SEEK_END);
				long file_size = ftell(fl);
				fseek(fl, 0, SEEK_SET);
				unsigned char *buffer = (unsigned char *)malloc(file_size + 1);
				fread(buffer, 1, file_size, fl);
				fclose(fl);

				int b64=0;

				printf("File size: %ld\n",file_size);

				if(buffer[0]==0x7f && buffer[1]=='E' && buffer[2]=='L' && buffer[3]=='F'){
					printf("\nFile format: ELF\n");
					if(buffer[4]==1){
						printf("32 bit\n");
					}
					if(buffer[4]==2){
						printf("64 bit\n");
						b64=1;
					}
					if(buffer[5]==1){
						printf("Little endian\n");
					}
					if(buffer[5]==2){
						printf("Big endian\n");
						break;
					}
					printf("Program entry: 0x");
					little_endian_read(buffer, 27 + 4 * b64, 4 + 4 * b64, 0, 1);
					printf("Program header table offset: 0x");
					little_endian_read(buffer, 31 + 8 * b64, 4 + 4 * b64, 0, 1);
					int program_header_table_offset = 0;
					for(int i=0;i < 4 + 4 * b64;i++){
						program_header_table_offset += buffer[28 + 4 * b64 + i] * pow(256,(double)i);
						//printf("%d\n",buffer[28 + 4 * b64 +i]);
					}
					//printf("%d",program_header_table_offset);
					int section_header_table_offset = little_endian_calc(buffer, 35+12*b64, 4+4*b64);
					printf("Section header table offset: 0x%x\n",section_header_table_offset);
					//little_endian_read(buffer, 35 + 12 * b64, 4 + 4 * b64, 0, 1);
					printf("Number of program header entries: ");
					//little_endian_read(buffer, 45 + 12 * b64, 2, 0, 1);
					int prog_header_entry_number = buffer[44 + 12 * b64] + 16 * buffer[45 + 12 * b64];
					printf("%d\n",prog_header_entry_number);
					printf("Program header table entry size: ");
					int prog_header_table_entry_size = 16 * buffer[43 + 12 * b64] + buffer[42 + 12 * b64];
					printf("%d\n", prog_header_table_entry_size);
					int section_header_entry_number = little_endian_calc(buffer, 49+12*b64,2);
					printf("Number of section header entries: %d\n",section_header_entry_number);
					//little_endian_read(buffer, 49 + 12 * b64, 2, 0, 1);
					printf("Section header table entry size: ");
					int section_header_table_entry_size = 16 * buffer[47 + 12 * b64] + buffer[46 + 12 * b64];
					printf("%d\n", section_header_table_entry_size);
					//printf("Section index to the section header string table: 0x");
					int string_table_index = little_endian_calc(buffer, 51 + 12 * b64, 2);
					//little_endian_read(buffer, 51 + 12 * b64, 2, 0, 1);
					printf("Section index to the section header string table: %d\n",string_table_index);

					printf("\n\nSegment data:\n");
					for(int i=0;i<prog_header_entry_number;i++){
						//little_endian_read(buffer, program_header_table_offset + i * prog_header_table_entry_size + 2, 3, 0);
						printf("%d   ",i+1);
						segment_data_64(buffer, program_header_table_offset + i * prog_header_table_entry_size, 1);
					}
					printf("\n\n");

					/*printf("String table: ");
					for(int i=0;i<1000;i++){
						printf("%c",buffer[section_header_table_offset + 0 * section_header_table_entry_size + i]);
					}*/

					//printf("string")

					/*printf("\n\nSection data:\n");
					for(int i=0;i<section_header_entry_number;i++){
						little_endian_read(buffer, section_header_table_offset + i * section_header_table_entry_size + 7,4,0,0);
					}
					printf("\n");*/

					printf("\n\nSection data:\n");
					for(int i=0;i<section_header_entry_number;i++){
						printf("%d    ",i+1);
						section_data_64(buffer, section_header_table_offset + i * section_header_table_entry_size, 1);
					}

					/*
					int string_count = 0;
					//printf("%d: ",string_count);
					for(int i=19064;i<19064 + 1075;i++){
						printf("%c",buffer[i]);
						if(buffer[i]==0 && buffer[i+1]!=0){
							string_count++;
							printf("\n%d: ",string_count);
						}
					}
					printf("\n");*/

					/*for(int i=0;i<file_size;i++){
						printf("%d:  %d,  ",i,buffer[i]);
					}*/

					printf("\n\nStrings:\n");
					for(int i=0;i<section_header_entry_number;i++){
						if(little_endian_calc(buffer, section_header_table_offset + i * section_header_table_entry_size + 7, 4) == 3){
							for(int j=0;j<little_endian_calc(buffer, section_header_table_offset + i * section_header_table_entry_size + 39,8);j++){
								printf("%c",buffer[little_endian_calc(buffer, section_header_table_offset + i * section_header_table_entry_size + 31,8) + j]);
								if(buffer[little_endian_calc(buffer, section_header_table_offset + i * section_header_table_entry_size + 31,8) + j] == 0){
									printf("\n");
								}
							}
						}
					}
					printf("\n");

					if(b64!=1){printf("Not a 64-bit file");}
					//Only works for 64-bit little endian
					int elf_block_count = prog_header_entry_number + section_header_entry_number + 3;
					int elf_blocks[elf_block_count], elf_block_sizes[elf_block_count], elf_blocks_mem[elf_block_count], elf_blocks_mem_size[elf_block_count];
					//elf_blocks = malloc(elf_block_count*sizeof(int));
					//elf_block_sizes = malloc(elf_block_count*sizeof(int));

					elf_blocks[0] = 0;
					elf_block_sizes[0] = 64;
					// not sure what this is
					elf_blocks_mem[0] = 0;
					elf_blocks_mem_size[0] = 0;

					elf_blocks[1] = program_header_table_offset;
					elf_block_sizes[1] = prog_header_entry_number * prog_header_table_entry_size;
					// not sure what this is
					elf_blocks_mem[1] = 0;
					elf_blocks_mem_size[1] = 0;

					elf_blocks[2] = section_header_table_offset;
					elf_block_sizes[2] = section_header_entry_number * section_header_table_entry_size;
					elf_blocks_mem[2] = 0;
					elf_blocks_mem_size[2] = 0;

					for(int i=3;i<3+prog_header_entry_number;i++){
						elf_blocks[i] = little_endian_calc(buffer, program_header_table_offset + (i-3) * prog_header_table_entry_size + 15, 8);
						elf_block_sizes[i] = little_endian_calc(buffer, program_header_table_offset + (i-3) * prog_header_table_entry_size + 39, 8);
						elf_blocks_mem[i] = little_endian_calc(buffer, program_header_table_offset + (i-3) * prog_header_table_entry_size + 23, 8);
						elf_blocks_mem_size[i] = little_endian_calc(buffer, program_header_table_offset + (i-3) * prog_header_table_entry_size + 47, 8);
					}

					for(int i=3+prog_header_entry_number;i<3+prog_header_entry_number+section_header_entry_number;i++){
						elf_blocks[i] = 
						little_endian_calc(buffer, section_header_table_offset + (i-3-prog_header_entry_number) * section_header_table_entry_size + 31, 8);
						elf_block_sizes[i] = 
						little_endian_calc(buffer, section_header_table_offset + (i-3-prog_header_entry_number) * section_header_table_entry_size + 39, 8);
						elf_blocks_mem[i] =
						little_endian_calc(buffer, section_header_table_offset + (i-3-prog_header_entry_number) * section_header_table_entry_size + 23, 8);
						elf_blocks_mem_size[i] =
						little_endian_calc(buffer, section_header_table_offset + (i-3-prog_header_entry_number) * section_header_table_entry_size + 39, 8);
					}


					printf("----Block-----File offset------------Memory address------------------------------\n");
					printf("ELF header: 0x%x - 0x%x\n",elf_blocks[0],elf_blocks[0] + elf_block_sizes[0]-1);
					printf("Program header: 0x%x - 0x%x\n",elf_blocks[1],elf_blocks[1] + elf_block_sizes[1]-1);
					printf("Section header: 0x%x - 0x%x\n",elf_blocks[2],elf_blocks[2] + elf_block_sizes[2]-1);

					for(int i=3;i<prog_header_entry_number + 3;i++){
						printf("Segment: 0x%x - 0x%x          0x%x - 0x%x\n", 
							elf_blocks[i],
							elf_blocks[i] + elf_block_sizes[i]-1 >= 0 ? elf_blocks[i],elf_blocks[i] + elf_block_sizes[i]-1 : 0,
							elf_blocks_mem[i],
							elf_blocks_mem[i] + elf_blocks_mem_size[i] - 1 >= 0 ? elf_blocks_mem[i] + elf_blocks_mem_size[i] - 1 : 0
							);	
					}

					for(int i=3+prog_header_entry_number;i<3+prog_header_entry_number+section_header_entry_number;i++){
						printf("Section: 0x%x - 0x%x        0x%x - 0x%x\n",
							elf_blocks[i],
							elf_blocks[i]+elf_block_sizes[i]-1>=0 ? elf_blocks[i],elf_blocks[i]+elf_block_sizes[i]-1 : 0,
							elf_blocks_mem[i],
							elf_blocks_mem[i]+elf_blocks_mem_size[i]-1>=elf_blocks_mem[i] ? elf_blocks_mem[i]+elf_blocks_mem_size[i]-1 : elf_blocks_mem[i]
							);
					}

					printf("\n\nELF Block details: \n");
					printf("Segments\n");
					for(int i=0;i<prog_header_entry_number;i++){
						printf("Segment header %d",i);
						if(little_endian_calc(buffer,program_header_table_offset + i * prog_header_table_entry_size + 7,4) % 2 == 1){
							printf("    [executable segment]\n\n");
							csh handle;
							cs_insn *insn;
							size_t cnt;

							char *CODE = 
							malloc((little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 39, 8)) * 4 + 1);
							//printf("%d\n",little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 39, 8));
							for(int j=0;j<little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 39, 8);j++){
								/*CODE[4 * j] = '\\';
								CODE[4 * j + 1] = 'x';
								CODE[4 * j + 2] = (char)(buffer[little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 15, 8)+j]/16);
								CODE[4 * j + 3] = (char)(buffer[little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 15, 8)+j]%16);*/
								sprintf(CODE, "\\x%02x", buffer[little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 15, 8)+j]);
								CODE += 4;
							}
							/*for(int u=0;u<sizeof(CODE);u++){
								printf("%c",CODE[u]);
							}*/
							printf("%s\n",CODE);

							if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK){
								printf("Executable failed to open by Capstone");
							}
							cnt = cs_disasm(handle, CODE, sizeof(CODE)-1, 0X0000, 0, &insn);
							if (cnt > 0){
								size_t m;
								for(m = 0; m < cnt; m++){
									printf("0x%"PRIx64":\t%s\t\t%s\n", insn[m].address, insn[m].mnemonic,insn[m].op_str);
								}
								cs_free(insn, cnt);
							} else printf("ERROR: Failed to disassemble given code!\n");

							cs_close(&handle);
						}
						printf("\n");
						for(int j=0;j<prog_header_table_entry_size;j++){
							printf("%x ",buffer[program_header_table_offset + i * prog_header_table_entry_size + j]);
						}
						printf("\n");
						printf("Segment %d\n",i);
						for(int j=0;j<little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 39, 8);j++){
							switch(little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 3, 4)){
							//case 4:
							case 3:
								printf("%c", buffer[little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 15, 8)+j]);
								break;
							default:
								printf("%x ", buffer[little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 15, 8)+j]);
							}
							//printf("%x ", buffer[little_endian_calc(buffer, program_header_table_offset + i * prog_header_table_entry_size + 15, 8)+j]);
						}
						printf("\n\n");
					}

					printf("\nSections\n");
					for(int i=0;i<section_header_entry_number;i++){
						printf("Section header %d\n", i);
						for(int j=0;j<section_header_table_entry_size;j++){
							printf("%x ",buffer[section_header_table_offset + i * section_header_table_entry_size + j]);
						}
						printf("\n");
						printf("Section %d\n",i);
						for(int j=0;j<little_endian_calc(buffer, section_header_table_offset + i * section_header_table_entry_size + 39, 8);j++){
							printf("%x ",buffer[little_endian_calc(buffer, section_header_table_offset + i * section_header_table_entry_size + 31, 8) + j]);
						}
						printf("\n\n");
					}

					/*for(int i=0xb20;i<0x1000;i++){
						printf("%x",buffer[i]);
					}*/

					/*for(int i=20139;i<20139 + 282;i++){
						printf("%c",buffer[i]);
					}*/

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