// translate() - takes two arguments, virtualAddress and option
// int virtualAddress - the virtual address in decimal form
// char option - to pick between page table and offset, accepts 'p' and 'o'
// returns: the portion of the physical address requested, in decimal form 
int translate (int virtualAddress, char option){
    int binaryNum[6]; 

    int i = 5; 
    while (n > 0) {
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i--; 
    } 
	if (option == 'p')
		return (binaryNum[1]*1 + binaryNum[0]*2);
	else if (option = 'o')
		return offset = binaryNum[5]*1 + binaryNum[4]*2 + binaryNum[3]*4 + binaryNum[2]*8;
	
	//neither option worked for whatever reason
	printf("Option failed! Tried to use option '%c'\n", option);
	return ERROR;
}