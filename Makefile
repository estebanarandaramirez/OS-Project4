all: memorysim

memorysim: part1_project4.c
	gcc part1_project4.c -g -Wall -o memorysim
clean:
	rm memorysim
