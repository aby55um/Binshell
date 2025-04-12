all: binshell

binshell:
	gcc -o binshell program.c command_parser.c command_execute.c input_loop.c variables.c -lm -lcapstone -Wall

clean:
	rm -f binshell
