// getBinary() - takes one argument, decForm
// int decForm - the decimal form of the desired number
// returns: the argument number as a binary number
int getBinary(int decForm){
    int bNum[6];
	int i = 5; 
    while (decForm > 0) {
        bNum[i] = decForm % 2; 
        decForm = decForm / 2; 
        i--; 
    } 
	return bNum;
}

// getDecimal() - takes one argument, binaryForm
// int binaryForm - the binary form of the desired number
// returns: the argument number as a decimal number
int getDecimal(int binaryForm){
	int pow = 1;
	int decForm = 0;
	for (int i = (sizeof(binaryForm)-1); i >= 0; i--){
		decForm += binaryForm[i]*pow;
		pow *= 2;
	}
	return pow;
}

// store() - takes two arguments, virtualAddress and value
// int virtualAddress - the virtual address in decimal form
// int value - the value to be stored
void store (int virtualAddress, int value){
	int page = translate(virtualAddress, 'p');
	int offset = translate(virtualAddress, 'o');
	
	//use page to find the physical page
	int phyFrame = 2; //PLACEHOLDER NUM
	int phyAdBinary[6] = (getBinary(phyFrame) * 10^4) + offset;
	physicalAddress = getDecimal(phyAdBinary);
	memory[physicalAddress] = value;
	
}

// translate() - takes two arguments, virtualAddress and option
// int virtualAddress - the virtual address in decimal form
// char option - to pick between page table and offset, accepts 'p' and 'o'
// returns: the portion of the physical address requested, in decimal form 
int translate (int virtualAddress, char option){
    int binaryNum = getBinary(virtualAddress); 

	if (option == 'p')
		return (binaryNum[1]*1 + binaryNum[0]*2);
	else if (option = 'o')
		return offset = binaryNum[5]*1 + binaryNum[4]*2 + binaryNum[3]*4 + binaryNum[2]*8;
	
	//neither option worked for whatever reason
	printf("Option failed! Tried to use option '%c'\n", option);
	return ERROR;
}