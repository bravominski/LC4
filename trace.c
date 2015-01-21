/*
 * trace.c - Minsu Kim
 */

#include "ObjectFiles.h"
#include "LC4.h"
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**********************Helping Function***********************/

 void toString (MachineState *theMachineState) {
 	unsigned short int PC = theMachineState->PC;
 	unsigned short int INSN = theMachineState->memory[theMachineState->PC];
 	int Rt, Rs, Rd;
 	char operation[7];

 	if (rsMux_ctl(INSN) == 0) { // I[8:6]
		Rs = get_bits(INSN, 6, 3);
	}
	else if (rsMux_ctl(INSN) == 1) { // 0x07
		Rs = 7;
	}
	else  if (rsMux_ctl(INSN) == 2){ // I[11:9]
		Rs = get_bits(INSN, 9, 3);
	}

	if (rtMux_ctl(INSN) == 0) { // I[2:0]
		Rt = get_bits(INSN, 0, 3);
	}
	else if (rtMux_ctl(INSN) == 1){ // I[11:9]
		Rt = get_bits(INSN, 9, 3);
	}

 	if (rdMux_ctl(INSN) == 0) { // I[11:9]
 		Rd = get_bits(INSN, 9, 3);
 	}
 	else if (rdMux_ctl(INSN) == 1) { // 0x07
 		Rd = 7;
 	}
 	
 	if (get_bits(INSN, 12, 4) == 0) { // Branch
 		if (get_bits(INSN, 9, 3) == 0) {
 			strcpy(operation, "NOP");
 			printf("%s\n", operation);
 			return;
 		}
 		if (get_bits(INSN, 9, 3) == 4) {
 			strcpy(operation, "BRn");
 		}
 		if (get_bits(INSN, 9, 3) == 6) {
 			strcpy(operation, "BRnz");
 		}
 		if (get_bits(INSN, 9, 3) == 5) {
 			strcpy(operation, "BRnp");
 		}
 		if (get_bits(INSN, 9, 3) == 2) {
 			strcpy(operation, "BRz");
 		}
 		if (get_bits(INSN, 9, 3) == 3) {
 			strcpy(operation, "BRzp");
 		}
 		if (get_bits(INSN, 9, 3) == 1) {
 			strcpy(operation, "BRp");
 		}
 		if (get_bits(INSN, 9, 3) == 7) {
 			strcpy(operation, "BRnzp");
 		}
 		printf("0x%.4x : %s %d\n",PC, operation, sext(get_bits(INSN, 0, 9), 9));
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 1) { // Arith
 		if (get_bits(INSN, 5, 1) == 1) {
 			strcpy(operation, "ADD");
 			signed short int add = sext(get_bits(INSN, 0, 5), 5);
 			printf("0x%.4x : %s R%d, R%d, %d\n", PC, operation, Rd, Rs, add);
 			return;
 		}
 		if (get_bits(INSN, 3, 3) == 0 || get_bits(INSN, 5, 1) == 1) {
 			strcpy(operation, "ADD");
 		}
 		if (get_bits(INSN, 3, 3) == 1) {
 			strcpy(operation, "MUL");
 		}
 		if (get_bits(INSN, 3, 3) == 2) {
 			strcpy(operation, "SUB");
 		}
 		if (get_bits(INSN, 3, 3) == 3) {
 			strcpy(operation, "DIV");
 		}
 		printf("0x%.4x : %s R%d, R%d, R%d\n", PC, operation, Rd, Rs, Rt);
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 2) { // Comparator
 		if (get_bits(INSN, 7, 2) == 0) {
 			strcpy(operation, "CMP");
 			printf("0x%.4x : %s R%d, R%d\n",PC, operation, Rs, Rt);
 			return;
 		}
 		if (get_bits(INSN, 7, 2) == 1) {
 			strcpy(operation, "CMPU");
 			printf("0x%.4x : %s R%d, R%d\n", PC, operation, Rs, Rt);
 			return;
 		}
 		if (get_bits(INSN, 7, 2) == 2) {
 			strcpy(operation, "CMPI");
 			printf("0x%.4x : %s R%d, %d\n", PC, operation, Rs, (signed short int)get_bits(INSN, 0, 7));
 			return;
 		}
 		if (get_bits(INSN, 7, 2) == 3) {
 			strcpy(operation, "CMPIU");
 			printf("0x%.4x : %s R%d, %d\n", PC, operation, Rs, get_bits(INSN, 0, 7));
 			return;
 		}
 	}
 	if (get_bits(INSN, 12, 4) == 5) { // Logic
 		if (get_bits(INSN, 5, 1) == 1) {
 			strcpy(operation, "AND");
 			printf("0x%.4x : %s R%d, R%d, %d\n", PC, operation, Rd, Rs, (signed short int)get_bits(INSN, 0, 5));
 			return;
 		}
 		if (get_bits(INSN, 3, 3) == 0) {
 			strcpy(operation, "AND");
 		}
 		if (get_bits(INSN, 3, 3) == 1) {
 			strcpy(operation, "NOT");
 		}
 		if (get_bits(INSN, 3, 3) == 2) {
 			strcpy(operation, "OR");
 		}
 		if (get_bits(INSN, 3, 3) == 3) {
 			strcpy(operation, "XOR");
 		}
 		printf("0x%.4x : %s R%d, R%d, R%d\n", PC, operation, Rd, Rs, Rt);
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 10) { // Shift 
 		if (get_bits(INSN, 4, 2) == 3) {
 			strcpy(operation, "MOD");
 			printf("0x%.4x : %s R%d, R%d, R%d\n", PC, operation, Rd, Rs, Rt);
 			return;
 		}
 		if (get_bits(INSN, 4, 2) == 0) {
 			strcpy(operation, "SLL");
 		}
 		if (get_bits(INSN, 4, 2) == 1) {
 			strcpy(operation, "SRA");
 		}
 		if (get_bits(INSN, 4, 2) == 2) {
 			strcpy(operation, "SRL");
 		}
 		printf("0x%.4x : %s R%d, R%d, %d\n", PC, operation, Rd, Rs, get_bits(INSN, 0, 4));
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 4) { // JSR
 		if (get_bits(INSN, 11, 1) == 1) {
 			strcpy(operation, "JSR");
 			printf("0x%.4x : %s %d\n", PC, operation, (signed short int)get_bits(INSN, 0, 11));
 			return;
 		}
 		if (get_bits(INSN, 11, 1) == 0) {
 			strcpy(operation, "JSRR");
 			printf("0x%.4x : %s R%d\n", PC, operation, Rs);
 			return;
 		}
 	}
 	if (get_bits(INSN, 13, 3) == 3) { // Memory - STR, LDR
 		if (get_bits(INSN, 12, 1) == 0) {
 			strcpy(operation, "LDR");
 			printf("0x%.4x : %s R%d, R%d, %d\n", PC, operation, Rd, Rs, (signed short int)get_bits(INSN, 0, 6));
 			return;
 		}
 		if (get_bits(INSN, 12, 1) == 1) {
 			strcpy(operation, "STR");
 			printf("0x%.4x : %s R%d, R%d, %d\n", PC, operation, Rt, Rs, (signed short int)get_bits(INSN, 0, 6));
 			return;
 		}
 	}
 	if (get_bits(INSN, 12, 4) == 12) { // JMP
 		if (get_bits(INSN, 11, 1) == 0) {
 			strcpy(operation, "JMPR");
 			printf("0x%.4x : %s R%d\n", PC, operation, Rs);
 			return;
 		}
 		if (get_bits(INSN, 11, 1) == 1) {
 			strcpy(operation, "JMP");
 			unsigned short int shift = get_bits(INSN, 0, 11);
			if (get_bits(shift, 10, 1) == 1) {
				shift = ~(shift + (0x1F << 11)) + 1;
			}
 			printf("0x%.4x : %s -%d\n", PC, operation, shift);
 			return;
 		}
 	}
 	if (get_bits(INSN, 12, 4) == 8) { // RTI
 		strcpy(operation, "RTI");
 		printf("0x%.4x : %s\n", PC, operation);
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 9) { // CONST, HICONST
 		strcpy(operation, "CONST");
 		printf("0x%.4x : %s R%d, %d\n", PC, operation, Rd, (signed short int)get_bits(INSN, 0, 9));
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 13) {
 		strcpy(operation, "HICONST");
 		printf("0x%.4x : %s R%d, %d\n", PC, operation, Rs, get_bits(INSN, 0, 8));
 		return;
 	}
 	if (get_bits(INSN, 12, 4) == 15) { // TRAP
 		strcpy(operation, "TRAP");
 		printf("0x%.4x : %s %d\n", PC, operation, get_bits(INSN, 0, 8));
 		return;
 	}
 }	

/****************************Main Function****************************/

 int main (int argc, char **argv) {
 	int i;
 	if (argc < 3) { // check if the number of arguments are enough
 		printf("You need at least two arguments for this code to work!\n");
 		exit(1);
 	}
 	
 	MachineState *theMachineState = malloc(sizeof(MachineState)); // Initialize theMachineState
 	if (theMachineState == NULL) {
 		printf("Couldn't allocate a new stack\n");
 		exit(1);
 	}

 	FILE *theFile;
 	unsigned short int currentPC, instruction;
 	Reset(theMachineState); // Reset the machine state
 	argv = argv + 1; // skip "./trace" part
 	theFile = fopen (*argv, "wb"); // Open up the file to write on
 	if (theFile == NULL) { // If file doesn't exist, quit
 		printf("Couldn't open file\n");
 		return 1;
 	}
 	argv = argv + 1; // skip "output.txt" part
 	while(*argv != 0) // Loop through the rest of arguments(obj files)
    {
         ReadObjectFile(*argv, theMachineState); // set memory as objectfiles are
         argv = argv + 1;
    }
    
 	currentPC = theMachineState->PC;
	instruction = theMachineState->memory[currentPC];
	int pc1, pc2, inst1, inst2;
	pc1 = get_bits(currentPC, 0, 8);
	pc2 = get_bits(currentPC, 8, 8);
	inst1 = get_bits(instruction, 0, 8);
	inst2 = get_bits(instruction, 8, 8);
 	fwrite (&pc2, sizeof(char), 1, theFile);
 	fwrite (&pc1, sizeof(char), 1, theFile);
 	fwrite (&inst2, sizeof(char), 1, theFile);
 	fwrite (&inst1, sizeof(char), 1, theFile);

 	//fwrite (&currentPC, sizeof(unsigned short int), 1, theFile);
 	//fwrite (&instruction, sizeof(unsigned short int), 1, theFile);

 	while (theMachineState->PC != 0x80FF) { // Loop until PC goes to 0x80FF or to the end of memory
 		//printf("Currently at PC: %.4x, which is %.4x, NZP is %x\n", currentPC, instruction, theMachineState->PSR & 0x0007);
 		// Print out on StdOut
		toString(theMachineState); // print out formmatted content -> PC : Command (Register) (Register)
  		UpdateMachineState(theMachineState); // Update the machine state
  		currentPC = theMachineState->PC;
	 	instruction = theMachineState->memory[currentPC];
	 	
	 	// Write on file : PC and Instruction
 		pc1 = get_bits(currentPC, 0, 8);
		pc2 = get_bits(currentPC, 8, 8);
		inst1 = get_bits(instruction, 0, 8);
		inst2 = get_bits(instruction, 8, 8);
	 	fwrite (&pc2, sizeof(char), 1, theFile);
	 	fwrite (&pc1, sizeof(char), 1, theFile);
	 	fwrite (&inst2, sizeof(char), 1, theFile);
	 	fwrite (&inst1, sizeof(char), 1, theFile);
 		//fwrite (&currentPC, sizeof(unsigned short int), 1, theFile);
 		//fwrite (&instruction, sizeof(unsigned short int), 1, theFile); 
 	}
 	toString(theMachineState);
 	printf("Current registers are\n"); // Print out final register contents
 	for (i = 0; i < 8; i++) {
 		printf("R%d = %.4x\n", i, theMachineState->R[i]);
 	}

 	free(theMachineState); // Free the memory allocated to theMachineState
 	return 0;
 }