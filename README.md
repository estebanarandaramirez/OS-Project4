# OS-Project4
Esteban Aranda (earandaramirez) and Benjamin Newmark (brnewmark)
----------------------------------------------------------------
Part 1:
Work in progress simulating virtual memory
To compile type "make" when in the appropriate directory. To run type "./memorysim". Each instruction requires 4 arguments separated by a comma and no spaces. For example: "0,map,34,0". You can exit the program by pressing Ctrl+C. Finally, to clean the project type "make clean".

Map is working properly right now and store.c contains code in progress but not yet finished for the store instruction. We could not yet implement the load instruction. Known bugs are that if you do not input the commands in a very specific manner the program segfaults.

Output for testing map can be seen in tests_output.txt and to run these tests type "./memorysim < test1_part1.txt" or "./memorysim < test2_part1.txt"
