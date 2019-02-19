CFLAGS = -Wall -lm -I.
DEPS = 
CC = gcc

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

phase1_project3: phase1_project3.c
	gcc -pthread -o phase1_project3 phase1_project3.c $(CFLAGS)

clean:
	rm phase1_project3
