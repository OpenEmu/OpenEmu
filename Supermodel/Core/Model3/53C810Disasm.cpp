/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * 53C810Disasm.cpp
 *
 * NCR 53C810 SCRIPTS disassembler.
 *
 * WARNING: Not all instructions are supported. They are being added as they
 * are encountered in Model 3 games. Some of the complicated operand forms may
 * be decoded incorrectly.
 */

#include <stdio.h>
#include <string.h>
#ifdef STANDALONE
#include <stdlib.h>
#endif
#include "Types.h"

#define DISASM_VERSION  "1.0"


/******************************************************************************
 Disassembler Function
******************************************************************************/

/*
 * DisassembleSCRIPTS(op, addr, mnem):
 *
 * Disassembles one SCRIPTS instruction.
 *
 * Parameters:
 *		op		Three consecutive words.
 *		addr	Current instruction address.
 *		mnem	Buffer to write instruction mnemonic to. If no instruction was
 *				successfully decoded, will be set to '\0'.
 *
 * Returns:
 *		Number of 32-bit words decoded (1, 2, or 3) or 0 if the instruction was
 *		not recognized.
 */
int DisassembleSCRIPTS(UINT32 op[3], UINT32 addr, char *mnem)
{
	static const char	*phase[] = { "data_out", "data_in", "command", "status", "res4", "res5", "message_out", "message_in" };
	int					carry, bitTrue, compData, compPhase, wait, mask, data;
    
    mnem[0] = '\0';

	if ((op[0]&0xC0000000) == 0)
	{
		// Block Move
		if ((op[0]&0x10000000))	// FROM
			sprintf(mnem, "move (%d) from %08X, when ", (op[0]>>27)&1, op[1]);
		else					// count
		{
			if ((op[0]&0x20000000))
				sprintf(mnem, "move (%d) %X, ptr %08X, when ", (op[0]>>27)&1, op[0]&0x00FFFFFF, op[1]);
			else
				sprintf(mnem, "move (%d) %X, %08X, when ", (op[0]>>27)&1, op[0]&0x00FFFFFF, op[1]);
		}
		strcat(mnem, phase[(op[0]>>24)&7]);
		return 2;
	}
	else if ((op[0]&0xE0000000) == 0xC0000000)
	{
		// Move Memory
		if ((op[0]&0x01000000))
			sprintf(mnem, "move memory no flush %X, %08X, %08X", op[0]&0x00FFFFFF, op[1], op[2]);
		else
			sprintf(mnem, "move memory %X, %08X, %08X", op[0]&0x00FFFFFF, op[1], op[2]);
		return 3;
	}
	else if ((op[0]&0xF8000000) == 0x98000000)
	{
		// INT and INTFLY
		mnem += sprintf(mnem, "%s %08X, ", (op[0]&0x00100000)?"intfly ":"int ", op[1]);
		
		carry = op[0]&0x00200000;
		bitTrue = op[0]&0x00080000;
		compData = op[0]&0x00040000;	// data (if set)
		compPhase = op[0]&0x00020000;	// phase (if set) (or atn if neither phase nor data set)
		wait = op[0]&0x00010000;
		data = op[0]&0xFF;
		mask = (op[0]>>8)&0xFF;
		
		if (carry)
			sprintf(mnem, "%s%s carry", wait?"if":"when", bitTrue?"":" not");
		else
		{
			mnem += sprintf(mnem, "%s%s ", wait?"if":"when", bitTrue?"":" not");
			if (!compPhase && !compData)
				sprintf(mnem, "atn");
			else
			{
				if (compPhase)
				{
					mnem += sprintf(mnem, phase[(op[0]>>24)&7]);
					if (compData)
						mnem += sprintf(mnem, " and ");
				}
				
				if (compData)
					mnem += sprintf(mnem, "%02X mask %02X", data, mask);
			}
				
		}
		
		
		return 2;
	}
    return 0;	// no match found
}


/******************************************************************************
 Standalone Disassembler
 
 Define STANDALONE to build a command line-driven SCRIPTS disassembler.
******************************************************************************/

#ifdef STANDALONE

static void PrintUsage(void)
{
    puts("scriptsd Version "DISASM_VERSION" by Bart Trzynadlowski: NCR 53C810 SCRIPTS Disassembler");
    puts("Usage:    scriptsd <file> [options]");
    puts("Options:  -?,-h       Show this help text");
    puts("          -s <offset> Start offset (hexadecimal)");
    puts("          -l <num>    Number of instructions (Default=16)");
    puts("          -o <addr>   Set origin (hexadecimal)");
    exit(0);
}

/*
 * main(argc, argv):
 *
 * Standalone SCRIPTS disassembler.
 */
int main(int argc, char **argv)
{
    char    	mnem[64];
    FILE    	*fp;
    UINT8   	*buffer;
    unsigned	i, num, offset, fsize, start = 0, len, org, file = 0;
    UINT32  	op[3];
    bool    	len_specified = 0, org_specified = 0;
    char    	*c;


    if (argc <= 1)
        PrintUsage();

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-?"))
            PrintUsage();
        else if (!strcmp(argv[i], "-s"))
        {
            ++i;
            if (i >= argc)
                fprintf(stderr, "scriptsd: warning: no argument to %s\n", "-s");
            else
                start = strtoul(argv[i], &c, 16);
        }
        else if (!strcmp(argv[i], "-l"))
        {
            ++i;
            if (i >= argc)
                fprintf(stderr, "scriptsd: warning: no argument to %s\n", "-l");
            else
            {
                len = atoi(argv[i]);
                len_specified = 1;
            }
        }
        else if (!strcmp(argv[i], "-o"))
        {
            ++i;
            if (i >= argc)
                fprintf(stderr, "scriptsd: warning: no argument to %s\n", "-o");
            else
            {
                org = strtoul(argv[i], &c, 16);
                org_specified = 1;
            }
        }
        else
            file = i;
    }

    if (!file)
    {
        fprintf(stderr, "scriptsd: no input file specified\n");
        exit(1);
    }
            
    /*
     * Load file
     */

    if ((fp = fopen(argv[file], "rb")) == NULL)
    {
        fprintf(stderr, "scriptsd: failed to open file: %s\n", argv[file]);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);

    // Allocate some extra padding for operands
    if ((buffer = (UINT8 *) calloc(fsize+4*3, sizeof(UINT8))) == NULL)
    {              
        fprintf(stderr, "scriptsd: not enough memory to load input file: %s, %lu bytes\n", argv[file], (unsigned long) fsize);
        fclose(fp);
        exit(1);
    }
    fread(buffer, sizeof(UINT8), fsize, fp);
    fclose(fp);

    if (!len_specified)
        len = 16;

    if (!org_specified)
        org = start;

    /*
     * Disassemble!
     */

	offset = start;
 	for (i = 0; (i<len) && (offset<fsize); i++)
 	{
 		op[0] = (buffer[offset+3]<<24)|(buffer[offset+2]<<16)|(buffer[offset+1]<<8)|buffer[offset+0];
 		op[1] = (buffer[offset+7]<<24)|(buffer[offset+6]<<16)|(buffer[offset+5]<<8)|buffer[offset+4];
 		op[2] = (buffer[offset+11]<<24)|(buffer[offset+10]<<16)|(buffer[offset+9]<<8)|buffer[offset+8];
 		
 		num = DisassembleSCRIPTS(op, org, mnem);
 		if (0 == num)
 		{
 			printf("%08X: %08X                   ?\n", org, op[0]);
 			offset += 4;
 			org += 4;
 		}
 		else
 		{
 			if (num == 3)
 				printf("%08X: %08X %08X %08X %s\n", org, op[0], op[1], op[2], mnem);
 			else if (num == 2)
 				printf("%08X: %08X %08X          %s\n", org, op[0], op[1], mnem);
 			else
 				printf("%08X: %08X                     %s\n", org, op[0], mnem);
 			offset += num*4;
 			org += num*4;
 		}
 	}
 		
    free(buffer);

    return 0;
}

#endif
