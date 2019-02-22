#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERROR -2 //return if error in a function
#define MEMORY_SIZE 64 //64 bytes for physical and virtual memory
#define PAGE_SIZE 16 //16 bytes, size of a page
#define NUM_PAGES 4 //number of pages
#define PTE_SIZE 4 //4 bytes, size of a page table entry
#define READ 0 //if page is readable
#define WRITE 1 //if page is readable and writeable
#define VPN 0 //virtual page number byte offset in a PTE
#define PFN 1 //page frame number byte offset in a PTE
#define PERMISSIONS 2 //permission bits offset in PTE
#define PRESENT 3 //present bits offset in PTE

//Function definitions
int map(unsigned char pid, unsigned char vaddress,unsigned char value);
int store(unsigned char pid, unsigned char vaddress,unsigned char value);
int load(unsigned char pid, unsigned char vaddress);
int findPte(int pid, int vaddress);
int findFree();
int locate(int pid, int vaddress);
void incrementrr();
int freeMemory(int pid, int num);
void evictPT(int pid);
void evictP(int pid, int paddress);
int pageToDisk(unsigned char *memory, int linenum, int start);

typedef struct hardwarePointer{
	int address; //address to start of page table for a process
	int inMemory; //2 if in disk(file), 1 if in physical memory, 0 otherwise
} hp;

//physical memory, 64 bytes long
unsigned char memory[MEMORY_SIZE];
//contains the free pages, 0 if the page is free 1 if not
int freepages[NUM_PAGES];
//contains information on what process holds a page
int pages[NUM_PAGES];
//contains information if page in memory is page table (1) or not (0)
int isPagetable[NUM_PAGES];
//round robin counter to swap out pages
int roundrobin;
//line counter in file representing disk
int line;
//hardware pointer to the begginning of the page table for a process
hp hardware[4];

int main(){
	printf("Usage: process_id,instruction_type,virtual_address,value\n");
	FILE* disk = fopen("disk.txt","w");
	fclose(disk);
	unsigned char pid, instruction = -1, vaddress, value = -1;
	roundrobin = 0;
	char input[20];
	for(int i = 0; i < NUM_PAGES; i++){
		hardware[i].inMemory = 0;
		freepages[i] = 0;
		isPagetable[i] = 0;
		pages[i] = -1;
	}
	while(1){
		char * token;
		printf("Instruction?: ");
		if(fgets(input, 20, stdin) == NULL){
			printf("\n");
			return 0;
		}
		while(input[0] == '\0' || input[0] == '\n' || input[1] =='\0' || input[1] =='\n'){
			printf("Instruction?:");
			if(fgets(input, 20, stdin) == NULL){
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
		char lastChar1 = token[1];
		char lastChar2 = token[2];
		value = atoi(token);
		token = NULL;
		if(pid < 0 || pid > 3){
			printf("Error: Process ID not valid. Range [0,3].\n");
		}
		if(vaddress < 0 || vaddress > 63){
			printf("Error: Virtual Address is not valid. Range [0,63].\n");
		}
		if((value >= 0 && value < 10) && (lastChar1 != '\n')){
			printf("Error: Value not valid. Range [0,255].\n");
		} else if((value >= 10 && value < 100) && (lastChar2 != '\n')){
			printf("Error: Value not valid. Range [0,255].\n");
		} else if((value >= 100 && value < 200) && firstNum > 1){
			printf("Error: Value not valid. Range [0,255].\n");
		} else if((value >= 200 && value < 256) && firstNum > 2){
			printf("Error: Value not valid. Range [0,255].\n");
		}
		switch(instruction){
			case 0:
				map(pid,vaddress,value);
				break;
			case 1:
				store(pid,vaddress,value);
				break;
			case 2:
				load(pid,vaddress);
				break;
		}
	}
	return 0;
}

//creates a mapping in the page table between a virtual and physical address
int map(unsigned char pid,unsigned char vaddress,unsigned char value){
	if(value < 0 || value > 1){
		printf("Error: Map only accepts 0 and 1 as values. 0 = readable, 1 = readable and writeable.\n");
		return ERROR;
	}
	int pte = findPte(pid,vaddress);
	if(pte == ERROR){
		int free1 = findFree();
		if(free1 == ERROR){
			freeMemory(pid, 1);
			free1 = findFree();
		}
		int p1 = free1/PAGE_SIZE;
		hardware[pid].address = free1;
		hardware[pid].inMemory = 1;
		pages[p1] = pid;
		isPagetable[p1] = 1;
		for(int i = 0; i < NUM_PAGES; i++){
			memory[free1 + (PTE_SIZE * i)] = i;
			memory[free1 + (PTE_SIZE * i) + PRESENT] = 0;
		}
		printf("Put page table for PID %d into physical frame %d\n", pid, p1);
	} else if(pte == -1){
		freeMemory(pid, 1);
	}

	pte = findPte(pid,vaddress);
	if(memory[pte + PRESENT] == 1){
		if(memory[pte + PERMISSIONS] == value){
			printf("Error: Page already has value %d\n",value);
			return ERROR;
		}	else {
			printf("Updating permissions from %d to %d\n", memory[pte + PERMISSIONS], value);
			memory[pte + PERMISSIONS] = value;
		}
		return 0;
	}

	int free2 = findFree();
	if(free2 == ERROR){
		freeMemory(pid, 1);
		free2 = findFree();
	}
	int p2 = free2/PAGE_SIZE;
	pages[p2] = pid;
	isPagetable[p2] = 0;
	printf("Mapped virtual address %d (page %d) for PID %d into physical frame %d\n", vaddress, vaddress/PAGE_SIZE, pid, p2);
	memory[pte + PFN] = free2;
	memory[pte + PERMISSIONS] = value;
	memory[pte + PRESENT] = 1;
	return 0;
}

//writes the supplied value into the phsyical memory location associated with the virtual address
int store(unsigned char pid, unsigned char vaddress, unsigned char value){
	int pte = findPte(pid,vaddress);
	if(pte == ERROR){
		printf("Error: Segmentation fault, memory hasn't been allocated for virtual address %d \n", vaddress);
		return ERROR;
	} else if(pte >= 0){
		int paddress = locate(pid,vaddress);
		if(paddress == ERROR){
			printf("Error: Segmentation fault, memory hasn't been allocated for virtual address %d \n", vaddress);
			return ERROR;
		}
		if(memory[pte + PERMISSIONS] == READ){
			printf("Error: Writes are not allowed to this page \n");
			return ERROR;
		} else if(memory[pte + PERMISSIONS] == WRITE){
			memory[paddress] = value;
			printf("Stored value %d at virtual address %d (physical address %d)\n", value, vaddress, paddress);
		}
	}
	return 0;
}

//returns the byte stored at the memory location specified by the virtual address
int load(unsigned char pid, unsigned char vaddress){
	int pte = findPte(pid,vaddress);
	if(pte == ERROR){
		printf("Error: Segmentation fault, memory hasn't been allocated for virtual address %d \n", vaddress);
		return ERROR;
	} else if(pte >= 0){
		int paddress = locate(pid,vaddress);
		if(paddress == ERROR){
			printf("Error: Segmentation fault, memory hasn't been allocated for virtual address %d \n", vaddress);
			return ERROR;
		}
		int value = memory[paddress];
		printf("The value %d is virtual address %d (physical address %d)\n", value, vaddress, paddress);
	}
	return 0;
}

//returns pte address for the given virtual address for the given pid
int findPte(int pid, int vaddress){
	if(hardware[pid].inMemory == 0){
		return ERROR; //return an error if it could not be found
	} else if(hardware[pid].inMemory == 1){
		int pteAddress;
		int vpage = 0;
		while(vaddress >= 16){
			vaddress -= 16;
			vpage++;
		}
		pteAddress = (hardware[pid].address + (PTE_SIZE * vpage));
		return pteAddress; //return pte address in phsyical memory
	} else if(hardware[pid].inMemory == 2){
		return -1;
	}
	return ERROR;
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

//locates the physical address for the given virtual address for the specified process
int locate(int pid, int vaddress){
	if(hardware[pid].inMemory == 1){ //process page table is in memory
		int vaddress_copy = vaddress;
		int vpage = 0;
		while(vaddress >= 16){
			vaddress -= 16;
			vpage++;
		}
		if(memory[hardware[pid].address + (PTE_SIZE * vpage) + PRESENT] == 0){
			return ERROR; //phsyical memory has not been allocated for virtual address
		}
		int paddress = memory[hardware[pid].address + (PTE_SIZE * vpage) + PFN];
		int offset = vaddress_copy - (PAGE_SIZE * vpage);
		paddress += offset;
		return paddress; //return physical address
	}
	return ERROR; //physical address not located
}

void incrementrr(){
	roundrobin++;
	roundrobin = roundrobin % NUM_PAGES;
}

int freeMemory(int pid, int num){
	int counter = 0;
	for(int i = 0; i < NUM_PAGES; i++){
		if(counter == num){
			continue;
		}
		if(isPagetable[roundrobin] == 1){
			if(pages[roundrobin] != pid) {
				evictPT(pages[roundrobin]);
				counter++;
				incrementrr();
				continue;
			}
		} else if(isPagetable[roundrobin] == 0){
			evictP(pages[roundrobin], roundrobin * PAGE_SIZE);
			counter++;
			incrementrr();
			continue;
		}
		incrementrr();
	}
	return 0;
}

void evictPT(int pid){
	int pt = hardware[pid].address;
	hardware[pid].address = pageToDisk(memory, line++, pt);
	hardware[pid].inMemory = 2;
	freepages[roundrobin] = 0;
	isPagetable[roundrobin] = 0;
	pages[roundrobin] = -1;
	for(int i = 0; i < PAGE_SIZE; i++){
		memory[pt + i] = 0;
	}
	printf("Swapped frame %d (page table for PID %d) into physical frame %d \n", roundrobin, pid, line - 1);
}

void evictP(int pid, int paddress){
	int pte = hardware[pid].address;
	int vpage = 0;
	for(int i = 0; i < PTE_SIZE; i++){
		if(memory[pte * i + 1] == paddress){
			vpage = memory[pte * i];
		}
	}
	int daddress = pageToDisk(memory, line++, paddress);
	memory[pte + (PTE_SIZE * vpage) + PFN] = daddress;
	memory[pte + (PTE_SIZE * vpage) + PRESENT] = 2;
	freepages[roundrobin] = 0;
	pages[roundrobin] = -1;
	for(int i = 0; i < PAGE_SIZE; i++){
		memory[pte + i] = 0;
	}
	printf("Swapped frame %d (PID %d) into physical frame %d \n", roundrobin, pid, line - 1);
}

int pageToDisk(unsigned char *memory, int linenum, int start){
	FILE* disk = fopen("disk.txt","a");
	fprintf(disk,"%d.",linenum);
	for(int i = 0; i < NUM_PAGES; i++){
		fprintf(disk,"%u ",memory[start + i]);
	}
	fprintf(disk,"%c",'\n');
	fclose(disk);
	return linenum;
}
