#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERROR -1 //return if error in a function
#define MEMORY_SIZE 64 //64 bytes for physical and virtual memory
#define PAGE_SIZE 16 //16 bytes, size of a page
#define NUM_PAGES 4 //number of pages
#define PTE_SIZE 4 //4 bytes, size of a page table entry
#define VPN 0 //virtual page number byte offset in a PTE
#define PFN 1 //page frame number byte offset in a PTE
#define READ 0 //if page is readable
#define WRITE 1 //if page is writeable

//Function definitions
int map(unsigned char pid, unsigned char vaddress,unsigned char value);
int store(unsigned char pid, unsigned char vaddress,unsigned char value);
int load(unsigned char pid, unsigned char vaddress);
int findPte(int pid, int vaddress);
int findFree();
void robin();
void freespace(int n, int pid);

typedef struct hardwarePointer{
	int address; //address to memory
	int inMemory; //1 if in physical memory, 0 if not
} hp;

//memory, 64 bytes long
unsigned char memory[MEMORY_SIZE];
//virtual memory. 64 bytes long, one for each process
unsigned char vmem0[MEMORY_SIZE];
unsigned char vmem1[MEMORY_SIZE];
unsigned char vmem2[MEMORY_SIZE];
unsigned char vmem3[MEMORY_SIZE];

//contains the free pages, 0 if the page is free 1 if not
int freepages[NUM_PAGES];

//contains information on what process holds a page
int pages[NUM_PAGES];

//hardware pointer to the begginning of the page table for a process
hp hardware[4];

int linecount;
int roundrobin;

int main(){
	printf("Usage: process_id,instruction_type,virtual_address,value\n");
	for(int i = 0; i < NUM_PAGES; i++){
		hardware[i].inMemory = 0;
		freepages[i] = 0;
		pages[i] = -1;
	}
	unsigned char pid, instruction = -1, vaddress, value = -1;
	char input[20];
	while(1){
		char * token;
		printf("Instruction?: ");
		if(fgets(input, 20, stdin)==NULL){
			printf("\n");
			return 0;
		}
		while(input[0] == '\0' || input[0] == '\n' || input[1] =='\0' || input[1] =='\n'){
			printf("Instruction?:");
			if(fgets(input, 20, stdin)==NULL){
			  printf("\n");
			  return 0;
		  }
		}
		token = strtok(input,",");
		pid = atoi(token);
		token = strtok(NULL,",");
		if(token[0] =='m' && token[1] == 'a' && token[2] == 'p'){
			instruction = 0;
		} else if(token[0] =='s' && token[1] =='t' && token[2] == 'o' && token[3] == 'r' && token[4] == 'e'){
			instruction = 1;
		} else if(token[0] =='l' && token[1] =='o' && token[2] == 'a' && token[3] == 'd'){
			instruction = 2;
		} else {
			printf("Error: Invalid instruction. Valid instructions: map, load, store.\n");
		}
		token = strtok(NULL,",");
		vaddress = atoi(token);
		token = strtok(NULL,",");
		int firstNum = token[0] - 48;
		char lastChar = token[3];
		value = atoi(token);
		token = NULL;
		if(pid < 0 || pid > 3){
			printf("Error: Process ID not valid. Range [0,3].\n");
		}
		if(vaddress < 0 || vaddress > 63){
			printf("Error: Virtual Address is not valid. Range [0,63].\n");
		}
		if((value >= 0 && value < 100) && lastChar != '\n'){
			printf("Error: Value not valid. Range [0,255].\n");
		} else if((value >= 100 && value < 200) && firstNum > 1){
			printf("Error: Value not valid. Range [0,255].\n");
		} else if((value >= 200 && value < 256) && firstNum > 2){
			printf("Error: Value not valid. Range [0,255].\n");
		}
		switch(instruction){
			case 0:
				//map(pid,vaddress,value);
				break;
			case 1:
				//store(pid,vaddress,value);
				break;
			case 2:
				//load(pid,vaddress);
				break;
		}
	}
	return 0;
}

//creates a mapping in the page table between a virtual and physical address
int map(unsigned char pid,unsigned char vaddress,unsigned char value){
	if(findPte(pid,vaddress) == ERROR){

	}
	return 1;
}

//returns pte address for the given virtual address for the given pid
int findPte(int pid, int vaddress){
	int pteAddress;
	int vpage = 0;
	while(vaddress >= 16){
		vaddress -= 16;
		vpage++;
	}
	if(hardware[pid].inMemory == 1){
		pteAddress = (hardware[pid].address + (PTE_SIZE * vpage));
		return pteAddress; //return pte address in phsyical memory
	}
	return ERROR;//return an error if it could not be found
}

//returns an address for the start of a free page in memory
int findFree(){
	for(int i = 0; i < NUM_PAGES; i++){
		if(freepages[i] == 0){ //page is free
			freepages[i] = 1; //remove page from free list
			return i * PAGE_SIZE; //return physical address
		}
	}
	return ERROR; //error because there are no free pages
}
