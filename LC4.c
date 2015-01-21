/*
 * LC4.c - Minsu Kim
 */
#include "LC4.h"
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int i;

//********Helping functions**************

signed short int sext(unsigned short int IMM, int bits) {
 	signed short int result;
 	if (get_bits(IMM, bits - 1, 1) == 1) {
			IMM = ~(IMM + (0xFFFF << bits)) + 1;
			return -IMM;
 	}
 	else {
 		return IMM;
 	}
 }

signed short int ALU_Arith (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char Arith_CTL,
			   unsigned char ArithMux_CTL) {
	
	signed short int result;

	if (Arith_CTL == 0) { // Add
		if (ArithMux_CTL == 0) { // input = RT
			result = RSOut + RTOut;
		}
		else if (ArithMux_CTL == 1) { // input = SEXT(IMM5)
			result = RSOut + sext(get_bits(INSN, 0, 5), 5);
		}
		else { // ArithMux_CTL == 2 -> input = SEXT(IMM6)
			result = RSOut + sext(get_bits(INSN, 0, 6), 6);
		}
	}
	else if (Arith_CTL == 1) { // Multiply
		result = RSOut * RTOut;
	}
	else if (Arith_CTL == 2) { // Subtract
		result = RSOut - RTOut;
	}
	else if (Arith_CTL == 3) { // Divide
		if (RTOut == 0) {
			return 0;
		}
		result = RSOut / RTOut;
	}
	else { // Arith_CTL == 4 // Mod
		if (RTOut == 0) {
			return 0; 
		}
		result = RSOut % RTOut;
	}
	return result;
}

unsigned short int ALU_Logic (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char LOGIC_CTL,
			   unsigned char LogicMux_CTL) {

	unsigned short int result;

	if (LOGIC_CTL == 0) { // AND
 		if (LogicMux_CTL == 0) { // input = RT
 			result = RSOut & RTOut;
 		}
 		else { // input = SEXT(IMM5)
 			result = RSOut & sext(get_bits(INSN, 0, 5), 5);
 		}
	}
	else if (LOGIC_CTL == 1) { // NOT
		result = ~RSOut + 1;
	}
	else if (LOGIC_CTL == 2) { // OR
		result = RSOut | RTOut;
	}
	else { // LOGIC_CTL == 3 -> XOR
		result = RSOut ^ RTOut;
	}
	return result;
}

unsigned short int ALU_Shift (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char SHIFT_CTL) {

	unsigned short int result;

	if (SHIFT_CTL == 0) { // SLL
		result = RSOut << sext(get_bits(INSN, 0, 4), 4);
	}
	else if (SHIFT_CTL == 1) { // SRA
		if (sext(RSOut, 16) < 0) {
			result = (RSOut >> get_bits(INSN, 0, 4)) | ~(~0U >> get_bits(INSN, 0, 4));
		}
		else {
			result = RSOut >> get_bits(INSN, 0, 4);	
		}
	}
	else { // SHIFT_CTL == 2 -> SRL
		result = RSOut >> sext(get_bits(INSN, 0, 4), 4);
	}
	return result;
}

signed short int ALU_Const (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char CONST_CTL) {

	unsigned short int result;

	if (CONST_CTL == 0) { // OUTPUT = SEXT(IMM9)
		result = sext(get_bits(INSN, 0, 9), 9);
	}
	else { // CONST_CTL == 1 -> HICONST, OUTPUT = (RS & 0xFF) | (UIMM8 << 8)
		result = (RSOut & 0xFF) | (get_bits(INSN, 0, 8) << 8); 
	}
	return result;
}

unsigned short int ALU_CMP (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char CMP_CTL) {

	unsigned short int result;

	if (CMP_CTL == 0) { // signed-CC(Rs - Rt) [-1, 0, +1]
		if ((signed short int)RSOut - (signed short int)RTOut > 0) {
			result = 1;
		}
		else if ((signed short int)RSOut - (signed short int)RTOut == 0) {
			result = 0;
		}
		else {
			result = -1;
		}
	}
	else if (CMP_CTL == 1) { // unsigned-CC(Rs - Rt) [-1, 0, +1]
		if (RSOut - RTOut > 0) {
			result = 1;
		}
		else if (RSOut - RTOut == 0) {
			result = 0;
		}
		else {
			result = -1;
		}
	}
	else if (CMP_CTL == 2) { // signed-CC(RS - SEXT(UIMM7)) [-1, 0, +1]
		if ((signed short int)RSOut - sext(get_bits(INSN, 0, 7), 7) > 0) {
			result = 1;
		}
		else if ((signed short int)RSOut - sext(get_bits(INSN, 0, 7), 7) == 0) {
			result = 0;
		}
		else {
			result = -1;
		}
	}
	else { // CMP_CTL == 3 -> unsigned-CC(RS - UIMM7) [-1, 0, +1]
		if (RSOut - get_bits(INSN, 0, 7) > 0) {
			result = 1;
		}
		else if (RSOut - get_bits(INSN, 0, 7) == 0) {
			result = 0;
		}
		else {
			result = -1;
		}
	}
	return result;
}

// get_bits:  get n bits from position p 
unsigned short int get_bits(unsigned short int x, int p, int n) {
       
       return (x >> p & ~(~0 << n));

}

// returns rsMux control
unsigned short int rsMux_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 12, 4) == 1  || get_bits(INSN, 12, 4) == 5 || get_bits(INSN, 12, 4) == 7 || get_bits(INSN, 13, 3) == 3 || get_bits(INSN, 11, 5) == 8 || get_bits(INSN, 12, 4) == 10 || get_bits(INSN, 11, 5) == 24) {
		return 0;
	}
	else if (get_bits(INSN, 12, 4) == 8) {
		return 1;
	}
	else if (get_bits(INSN, 12, 4) == 2 || get_bits(INSN, 12, 4) == 13) {
		return 2;
	}
	else {
		return 3;
	}
}

// returns rtMux control
unsigned short int rtMux_ctl(unsigned short int INSN) {
	if ((get_bits(INSN, 12, 4) == 1 &&  get_bits(INSN, 5, 1) == 0) || (get_bits(INSN, 12, 4) == 2 &&  get_bits(INSN, 8, 1) == 0) || (get_bits(INSN, 12, 4) == 5 &&  (get_bits(INSN, 3, 3) == 0 || get_bits(INSN, 3, 3) == 2 || get_bits(INSN, 3, 3) == 3)) || (get_bits(INSN, 12, 4) == 10 &&  get_bits(INSN, 4, 2) == 3))  {
		return 0;
	}
	else if (get_bits(INSN, 12, 4) == 7) {
		return 1;
	}
	else {
		return 2;
	}
}

// returns rdMux control
unsigned short int rdMux_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 12, 4) == 1 || get_bits(INSN, 12, 4) == 5 || get_bits(INSN, 12, 4) == 6 || get_bits(INSN, 12, 4) == 9 || get_bits(INSN, 12, 4) == 10 || get_bits(INSN, 12, 4) == 13) {
		return 0;
	}
	else if (get_bits(INSN, 12, 4) == 4 || get_bits(INSN, 12, 4) == 15) {
		return 1;
	}
	else {
		return 2;
	}
}

// returns pcMux control
unsigned short int pcMux_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 12, 4) == 0) {
		return 0;
	}
	else if (get_bits(INSN, 11, 5) == 25) {
		return 2;
	}
	else if (get_bits(INSN, 11, 5) == 24 || get_bits(INSN, 12, 4) == 8) {
		return 3;
	}
	else if (get_bits(INSN, 12, 4) == 15) {
		return 4;
	}
	else if (get_bits(INSN, 11, 5) == 9) {
		return 5;
	}
	else {
		return 1;
	}
}

// returns regInputMux control
unsigned short int regInputMux_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 12, 4) == 1 || get_bits(INSN, 12, 4) == 5 || get_bits(INSN, 12, 4) == 9 || get_bits(INSN, 12, 4) == 10 || get_bits(INSN, 12, 4) == 13) {
		return 0; // Write input = ALUOut
	}
	else if (get_bits(INSN, 12, 4) == 6 || get_bits(INSN, 12, 4) == 4) { // Write input = Output of Data Memory
		return 1;
	}
	else if (get_bits(INSN, 12, 4) == 15) { // Write input = PC + 1
		return 2;
	}
	else { //Invalid
		return 3;
	}
}

// returns Arith control
unsigned char arith_ctrl(unsigned short int INSN) {
	if (get_bits(INSN, 13, 3) == 3 || get_bits(INSN, 3, 3) == 0 || (get_bits(INSN, 12, 4) == 1 && get_bits(INSN, 5, 1) == 1) || get_bits(INSN, 12, 4) == 8 || get_bits(INSN, 12, 4) == 9) { // ADD
		return 0;
	}
	else if (get_bits(INSN, 3, 3) == 1) { // MULT
		return 1;
	}
	else if (get_bits(INSN, 3, 3) == 2) { // SUB
		return 2;
	}
	else if (get_bits(INSN, 3, 3) == 3) { // DIV
		return 3;
	}
	else { // MOD
		return 4;
	}
}

// returns ArithMux control
unsigned char arithMux_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 12, 4) == 1 && get_bits(INSN, 5, 1) == 0) { // Arith input = Rt
		return 0;
	}
	else if (get_bits(INSN, 12, 4) == 1 && get_bits(INSN, 5, 1) == 1) { // Arith input = SEXT(IMM5)
		return 1;
	}
	else { // Arith input = SEXT(IMM6) -> LDR, STR
		return 2;
	}
}

// returns Logic control
unsigned char logic_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 3, 3) == 0 || get_bits(INSN, 5, 1) == 1) { // AND
		return 0;
	}
	else if (get_bits(INSN, 3, 3) == 1) { // NOT
		return 1;
	}
	else if (get_bits(INSN, 3, 3) == 2) { // OR
		return 2;
	}
	else { // XOR
		return 3;
	}
}

// returns LogicMux control
unsigned char logicMux_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 3, 3) == 0 || get_bits(INSN, 3, 3) == 2 || get_bits(INSN, 3, 3) == 3) { // Logic input = Rt
		return 0;
	}
	else { // Logic input = SEXT(IMM5)
		return 1;
	}
}

// returns Shift control
unsigned char shift_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 4, 2) == 0) { // Shift Left Logical
		return 0;
	}
	else if (get_bits(INSN, 4, 2) == 1) { // Shift Right Arithmetic
		return 1;
	}
	else { // Shift right Logical
		return 2;
	}
}

// returns Const control
unsigned char cons_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 12, 4) == 9) { // Const
		return 0;
	}
	else { // Hi-const
		return 1;
	}
}

// returns Comparator control
unsigned char cmp_ctl(unsigned short int INSN) {
	if (get_bits(INSN, 7, 2) == 0) { // Signed R-R Comparison
		return 0;
	}
	else if (get_bits(INSN, 7, 2) == 1) { // Unsigned R-R Comparison
		return 1;
	}
	else if (get_bits(INSN, 7, 2) == 2){ // Signed R-IMM Comparison
		return 2;
	}
	else { // Unsigned R-UIMM Comparison
		return 3;
	}
}

// returns ALUMux control
unsigned char aluMux_ctl(unsigned short int INSN) { 
	if (get_bits(INSN, 12, 4) == 1 || get_bits(INSN, 13, 3) == 3) { // ALUoutput = Arith output
		return 0;
	}
	else if (get_bits(INSN, 12, 4) == 5) { // ALUoutput = Logic output
		return 1;
	}
	else if (get_bits(INSN, 12, 4) == 10) { // ALUoutput = Shifter output
		return 2;
	}
	else if (get_bits(INSN, 12, 4) == 9 || get_bits(INSN, 12, 4) == 13) { // ALUoutput = Const output
		return 3;
	}
	else { //ALUoutput = Comparator output
		return 4;
	}
}

// returns NZP Write Enable
unsigned short int NZP_WE(INSN) { // NZP.WE is 1 when we are updating a register
	if (get_bits(INSN, 12, 4) == 1 || get_bits(INSN, 12, 4) == 4 || get_bits(INSN, 12, 4) == 5 || get_bits(INSN, 12, 4) == 6 || get_bits(INSN, 12, 4) == 9 || get_bits(INSN, 12, 4) == 10 || get_bits(INSN, 12, 4) == 13 || get_bits(INSN, 12, 4) == 15) {
 		return 1;
	}
	else {
		return 0;
	}
}

// returns DATA Write Enable
unsigned short int DATA_WE(INSN) { // DATA.WE is 1 only when using STR
	if (get_bits(INSN, 12, 4) == 7) {
		return 1;
	}
	else {
		return 0;
	}
}

//*******************************************

// Reset the machine state as Pennsim would do
void Reset (MachineState *theMachineState) {
	theMachineState->PC = 0x8200;
	theMachineState->PSR = 0x8002;
	for (i = 0; i < 8; i++) {
		theMachineState->R[i] = 0;
	}
	for (i = 0; i < 65536; i++) {
		theMachineState->memory[i] = 0;
	}
}

// Update Machine State - simulate how the state of the machine changes over a single clock cycle
int UpdateMachineState (MachineState *theMachineState) {
	unsigned short int INSN = theMachineState->memory[theMachineState->PC];
	unsigned short int rsMux_c = rsMux_ctl(INSN);
	unsigned short int rtMux_c = rtMux_ctl(INSN);
	unsigned short int rdMux_c = rdMux_ctl(INSN);
	unsigned short int pcMux_c = pcMux_ctl(INSN);
	unsigned short int regInputMux_c = regInputMux_ctl(INSN);
	unsigned short int NZP = NZP_WE(INSN);
	unsigned short int DATA = DATA_WE(INSN);
	unsigned char arith_c = arith_ctrl(INSN);
	unsigned char arithMux_c = arithMux_ctl(INSN);
	unsigned char logic_c = logic_ctl(INSN);
	unsigned char logicMux_c = logicMux_ctl(INSN);
	unsigned char shift_c = shift_ctl(INSN);
	unsigned char cons_c = cons_ctl(INSN);
	unsigned char cmp_c = cmp_ctl(INSN);
	unsigned char aluMux_c = aluMux_ctl(INSN);
 	unsigned short int registerInput = ALUMux(INSN, RS(theMachineState, rsMux_c), RT(theMachineState, rtMux_c),
			   arith_c, arithMux_c, logic_c, logicMux_c, shift_c, cons_c, cmp_c, aluMux_c);
 	unsigned short int rd;

 	if ((theMachineState->PC > 0x1FFF && theMachineState->PC < 0x8000) || (theMachineState->PC > 0x9FFF && theMachineState->PC <= 0xFFFF)) { // Exception 1
 		printf("You are attempting to execute a data section address as code\n");
 		exit(1);
 	}
 	if ((get_bits(INSN, 12, 4) == 6 || get_bits(INSN, 12, 4) == 7) && (registerInput < 0x2000 || (registerInput > 0x7FFF && registerInput < 0xA000))) { // Exception 2
 		printf("You are attempting to read or write a code section address as data!\n");
 		exit(2);
 	}

 	if ((get_bits(theMachineState->PSR, 15, 1) == 0 && theMachineState->PC > 0x81FF) || (get_bits(INSN, 12, 4) == 7 && get_bits(theMachineState->PSR, 15, 1) == 0 && registerInput > 0x9FFF)) { // Exception 3
 		printf("You are attempting to access an address or instruction in the OS section of memory when the processor is in user mode!\n");
 		exit(3);
 	}

 	if (get_bits(INSN, 12, 4) == 8 && get_bits(theMachineState->PSR, 15, 1) == 1) { // Update privilege bit for RTI
 		theMachineState->PSR = theMachineState->PSR - 0x8000;
 	}
 	if (get_bits(INSN, 12, 4) == 15 && get_bits(theMachineState->PSR, 15, 1) == 0) { // Update privilege bit for TRAP
 		theMachineState->PSR = theMachineState->PSR + 0x8000;
 	}

 	// Write on Register only when RdMux is either 0 or 1
 	if (rdMux_c == 0) {
 		rd = get_bits(INSN, 9, 3);
 		theMachineState->R[rd] = regInputMux(theMachineState, registerInput, regInputMux_c);
 	}
 	else if (rdMux_c == 1) {
 		rd = 7;
 		theMachineState->R[rd] = regInputMux(theMachineState, registerInput, regInputMux_c);
 	}

 	if (DATA == 1) {
 		theMachineState->memory[registerInput] = theMachineState->R[RT(theMachineState, rtMux_c)];
 	}

 	// Update PC
 	theMachineState->PC = PCMux(theMachineState, RS(theMachineState, rsMux_c), pcMux_c);

 	if (NZP == 1 || get_bits(INSN, 12, 4) == 2) { // Update NZP - 1. When register is updated, 2. When Comparator is used;
 		if (get_bits(INSN, 12, 4) == 1 || get_bits(INSN, 12, 4) == 9 || get_bits(INSN, 12, 4) == 7) {
 			if (sext(registerInput, 16) > 0) {
 				theMachineState->PSR = ((theMachineState->PSR >> 3) << 3) | 1; 
	 		}
	 		else if (sext(registerInput, 16) == 0) {
	 			theMachineState->PSR = ((theMachineState->PSR >> 3) << 3) | 2;
	 		}
	 		else if (sext(registerInput, 16) < 0){
	 			theMachineState->PSR = ((theMachineState->PSR >> 3) << 3) | 4;
	 		}
 		}
 		else {
 			if (registerInput > 0) {
 			theMachineState->PSR = ((theMachineState->PSR >> 3) << 3) | 1; 
	 		}
	 		else if (registerInput == 0) {
	 			theMachineState->PSR = ((theMachineState->PSR >> 3) << 3) | 2;
	 		}
	 		else if (registerInput < 0){
	 			theMachineState->PSR = ((theMachineState->PSR >> 3) << 3) | 4;
	 		}
 		}
 	}

	return 1;

}

// Note that the UpdateMachineState function must perform its action using the helper functions
// declared below which should be used to simulate the operation of portions of the datapath.
//
// Note that all of the control signals passed as arguments to these functions are represented
// as unsigned 8 bit values although none of them use more than 3 bits. You should use the lower
// bits of the fields to store the mandated control bits. Please refer to the LC4 information sheets
// on Canvas for an explanation of the clcontrol signals and their role in the datapath.


// Compute the current output of the RS port
unsigned short int RS (MachineState *theMachineState, unsigned char rsMux_CTL) {

	unsigned short int INSN = theMachineState->memory[theMachineState->PC];
	unsigned short int RSOut;

	if (rsMux_CTL == 0) { // I[8:6]
		RSOut = theMachineState->R[get_bits(INSN, 6, 3)];
	}
	else if (rsMux_CTL == 1) { // 0x07
		RSOut = theMachineState->R[7];
	}
	else { // rsMux_CTL == 2 -> I[11:9]
		RSOut = theMachineState->R[get_bits(INSN, 9, 3)];
	}
	return RSOut;
}

// Compute the current output of the RT port
unsigned short int RT (MachineState *theMachineState, unsigned char rtMux_CTL) {

	unsigned short int INSN = theMachineState->memory[theMachineState->PC];
	unsigned short int RTOut;
	
	if (rtMux_CTL == 0) { // I[2:0]
		RTOut = theMachineState->R[get_bits(INSN, 0, 3)];
	}
	else { // I[11:9]
		RTOut = theMachineState->R[get_bits(INSN, 9, 3)];
	}
	return RTOut;
}

// Compute the current output of the ALUMux
unsigned short int ALUMux (unsigned short int INSN,   // current instruction
			   unsigned short int RSOut,  // current RS port output
			   unsigned short int RTOut,  // current RT port output
			   unsigned char Arith_CTL,
			   unsigned char ArithMux_CTL,
			   unsigned char LOGIC_CTL,
			   unsigned char LogicMux_CTL,
			   unsigned char SHIFT_CTL,
			   unsigned char CONST_CTL,
			   unsigned char CMP_CTL,
			   unsigned char ALUMux_CTL) {

	unsigned short int ALUOut;

	if (ALUMux_CTL == 0) { // Arithmetic
		ALUOut = ALU_Arith(INSN, RSOut, RTOut, Arith_CTL, ArithMux_CTL);
	}
	else if (ALUMux_CTL == 1) { // Logical
		ALUOut = ALU_Logic(INSN, RSOut, RTOut, LOGIC_CTL, LogicMux_CTL);
	}
	else if (ALUMux_CTL == 2) { // Shifter
		ALUOut = ALU_Shift(INSN, RSOut, RTOut, SHIFT_CTL);
	}
	else if (ALUMux_CTL == 3) { // Constant
		ALUOut = ALU_Const(INSN, RSOut, RTOut, CONST_CTL);
	}
	else  { //ALUMux_CTL == 4 -> Comparator
		ALUOut = ALU_CMP(INSN, RSOut, RTOut, CMP_CTL);
	}
	return ALUOut;
}

// Compute the current output of the regInputMux
unsigned short int regInputMux (MachineState *theMachineState,
				unsigned short int ALUMuxOut, // current ALUMux output
			unsigned char regInputMux_CTL) {
	
	unsigned short int writeInput;

	if (regInputMux_CTL == 0) { // input = ALUMuxOut
		writeInput = ALUMuxOut;
	}
	else if (regInputMux_CTL == 1) { // input = from Data Memory
		writeInput = theMachineState->memory[ALUMuxOut];
	}
	else { // regInputMux_CTL == 2 -> input = PC + 1
		writeInput = theMachineState->PC + 1;
	}
	return writeInput;
}

// Compute the current output of the PCMux
unsigned short int PCMux (MachineState *theMachineState,
			  unsigned short int RSOut,
			  unsigned char PCMux_CTL) {

	unsigned short int INSN = theMachineState->memory[theMachineState->PC];
	unsigned short int result;
	
	if (PCMux_CTL == 0) { // if NZP == I[11:9] -> next PC = (PC + 1) + SEXT(IMM9), else next PC = PC + 1
		unsigned short int NZPbit = theMachineState->PSR & 0x0007;
		if ((NZPbit & get_bits(INSN, 9, 3)) != 0) {
			result = (theMachineState->PC + 1) + sext(get_bits(INSN, 0, 9), 9);
		}
		else {
			result = theMachineState->PC + 1;
		}
	}
	else if (PCMux_CTL == 1) { // next PC = PC +1
		result = theMachineState->PC + 1;
	}
	else if (PCMux_CTL == 2) { // next PC = (PC + 1) + SEXT(IMM11)
		unsigned short int shift = sext(get_bits(INSN, 0, 11), 11);
		result = (theMachineState->PC + 1) + shift;	
	}
	else if (PCMux_CTL == 3) { // next PC = RS
		result = RSOut;
	}
	else if (PCMux_CTL == 4) { // next PC = (0x8000 | UIMM8)
		result = 0x8000 | get_bits(INSN, 0, 8);
	}
	else { // PCMux_CTL == 5 -> next PC = (PC & 0x8000) | (IMM11 << 4)
		result = (theMachineState->PC & 0x8000) | (sext(get_bits(INSN, 0, 11), 11) << 4);
	}
	return result;
}

