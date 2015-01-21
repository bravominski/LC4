/*
 * ObjectFiles.c - Minsu Kim
 */
#include "ObjectFiles.h"
#include <math.h>
#include <ctype.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

 unsigned short int combine (int upperEight, int lowerEight) {
 	return (upperEight << 8) | lowerEight;
 }

 int ReadObjectFile (char *filename, MachineState *theMachineState) {
 	FILE *theFile;
 	int upper, lower;
 	theFile = fopen(filename, "rb");
 	if (theFile == NULL) {
 		printf ("Couldn't open file\n");
 		return 1;
 	}
  	do {
 		int addrUpper, addrLower, numUpper, numLower, conUpper, conLower, content2, i;
 		upper = fgetc(theFile);
 		lower = fgetc(theFile);
 		unsigned short int content, addr, num;
 		unsigned short int section = combine(upper, lower);
 		if (section == 0xCADE) { // Code Section
 			addrUpper = fgetc(theFile);
 			addrLower = fgetc(theFile); 			
 			numUpper = fgetc(theFile);
 			numLower = fgetc(theFile);
 			if (addrLower == EOF || addrUpper == EOF || numLower == EOF || numUpper == EOF) {
 				exit(1);
 			}
 			addr = combine(addrUpper, addrLower);
 			num = combine(numUpper, numLower);
 			for (i = 0; i < num; i++) { 			    
 			    conUpper = fgetc(theFile);
 			    conLower = fgetc(theFile);
 			    if (conLower == EOF || conUpper == EOF) {
 					exit(1);
 				}
 			    content = combine(conUpper, conLower);
 			    theMachineState->memory[addr] = content;
 			    addr += 1;
 			}
 		}
 		else if (section == 0xDADA) { // Data Section
 			addrUpper = fgetc(theFile);
 			addrLower = fgetc(theFile); 			
 			numUpper = fgetc(theFile);
 			numLower = fgetc(theFile);
 			if (addrLower == EOF || addrUpper == EOF || numLower == EOF || numUpper == EOF) {
 				exit(1);
 			}
 			addr = combine(addrUpper, addrLower);
 			num = combine(numUpper, numLower);
 			for (i = 0; i < num; i++) { 			    
 			    conUpper = fgetc(theFile);
 			    conLower = fgetc(theFile);
 			    if (conLower == EOF || conUpper == EOF) {
 					exit(1);
 				}
 			    content = combine(conUpper, conLower);
 			    theMachineState->memory[addr] = content;
 			    addr += 1;
 			}
 		}
 		else if (section == 0xC3B7) { // Symbol
 			addrUpper = fgetc(theFile);
 			addrLower = fgetc(theFile); 			
 			numUpper = fgetc(theFile);
 			numLower = fgetc(theFile);
 			if (addrLower == EOF || addrUpper == EOF || numLower == EOF || numUpper == EOF) {
 				exit(1);
 			}
 			addr = combine(addrUpper, addrLower);
 			num = combine(numUpper, numLower);
 			for (i = 0; i < num; i++) {
 			    content2 = fgetc(theFile);
 			    if (content2 == EOF) {
 					exit(1);
 				}
 			}
 		}
 		else if (section == 0xF17E) { // Filename 			
 			numUpper = fgetc(theFile);
 			numLower = fgetc(theFile);
 			if (numLower == EOF || numUpper == EOF) {
 				exit(1);
 			}
 			num = combine(numUpper, numLower);
 			for (i = 0; i < num; i++) { // Just jump n steps
 			    content2 = fgetc(theFile);
 			    if (content2 == EOF) {
 					exit(1);
 				}
 			}
 		}
 		else if (section == 0x715E) { // Line Number
 			content2 = fgetc(theFile); // Address
 			content2 = fgetc(theFile);
 			content2 = fgetc(theFile); // Line
 			content2 = fgetc(theFile);
 			content2 = fgetc(theFile); // File-index
 			content2 = fgetc(theFile);
 		}
 	} while (lower != EOF && upper != EOF);

 	rewind (theFile);

 	fclose(theFile);
 	return 0;
 }