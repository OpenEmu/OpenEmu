/* Decode a Game Genie code into an M68000 address/data pair.
 * The Game Genie code is made of the characters
 * ABCDEFGHJKLMNPRSTVWXYZ0123456789 (notice the missing I, O, Q and U).
 * Where A = 00000, B = 00001, C = 00010, ... , on to 9 = 11111.
 * 
 * These come out to a very scrambled bit pattern like this:
 * (SCRA-MBLE is just an example)
 *
 *   S     C     R     A  -  M     B     L     E
 * 01111 00010 01110 00000 01011 00001 01010 00100
 * ijklm nopIJ KLMNO PABCD EFGHd efgha bcQRS TUVWX
 *
 * Our goal is to rearrange that to this:
 *
 * 0000 0101 1001 1100 0100 0100 : 1011 0000 0111 1000
 * ABCD EFGH IJKL MNOP QRST UVWX : abcd efgh ijkl mnop
 *
 * which in Hexadecimal is 059C44:B078. Simple, huh? ;)
 *
 * So, then, we dutifully change memory location 059C44 to B078!
 * (of course, that's handled by a different source file :)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "emulator/g_main.hpp"
#include "ggenie.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "util/file/rom.hpp"


struct GG_Code Game_Genie_Codes[256];
static const char genie_chars[] =
  "AaBbCcDdEeFfGgHhJjKkLlMmNnPpRrSsTtVvWwXxYyZz0O1I2233445566778899";
static const char hex_chars[] = "00112233445566778899AaBbCcDdEeFf";
char Patch_Dir[GENS_PATH_MAX] = "/";


/**
 * Init_GameGenie(): Initialize the Game_Genie_Codes[] array.
 */
void Init_GameGenie(void)
{
	int i;
	
	// Address and restore values are both 0xFFFFFFFF when the entry is unused.
	for (i = 0; i < 256; i++)
	{
		Game_Genie_Codes[i].code[0] = 0;
		Game_Genie_Codes[i].name[0] = 0;
		Game_Genie_Codes[i].active = 0;
		Game_Genie_Codes[i].addr = 0xFFFFFFFF;
		Game_Genie_Codes[i].data = 0;
		Game_Genie_Codes[i].restore = 0xFFFFFFFF;
	}
}


/* genie_decode
 * This function converts a Game Genie code to an address:data pair.
 * The code is given as an 8-character string, like "BJX0SA1C". It need not
 * be null terminated, since only the first 8 characters are taken. It is
 * assumed that the code is already made of valid characters, i.e. there are no
 * Q's, U's, or symbols. If such a character is
 * encountered, the function will return with a warning on stderr.
 *
 * The resulting address:data pair is returned in the struct patch pointed to
 * by result. If an error results, both the address and data will be set to -1.
 */

static void genie_decode(const char *code, struct patch *result)
{
	int i, n;
	char *x;
	
	for (i = 0; i < 8; i++)
	{
		/* If strchr returns NULL, we were given a bad character */
		if (!(x = strchr(genie_chars, code[i])))
		{
			result->addr = -1;
			result->data = -1;
			return;
		}
		
		// TODO: Pointer arithmetic is a bad thing. Clean this up.
		n = (x - genie_chars) >> 1;
		
		/* Now, based on which character this is, fit it into the result */
		switch (i)
		{
			case 0:
				/* ____ ____ ____ ____ ____ ____ : ____ ____ ABCD E___ */
				result->data |= n << 3;
				break;
			case 1:
				/* ____ ____ DE__ ____ ____ ____ : ____ ____ ____ _ABC */
				result->data |= n >> 2;
				result->addr |= (n & 3) << 14;
				break;
			case 2:
				/* ____ ____ __AB CDE_ ____ ____ : ____ ____ ____ ____ */
				result->addr |= n << 9;
				break;
			case 3:
				/* BCDE ____ ____ ___A ____ ____ : ____ ____ ____ ____ */
				result->addr |= (n & 0xF) << 20 | (n >> 4) << 8;
				break;
			case 4:
				/* ____ ABCD ____ ____ ____ ____ : ___E ____ ____ ____ */
				result->data |= (n & 1) << 12;
				result->addr |= (n >> 1) << 16;
				break;
			case 5:
				/* ____ ____ ____ ____ ____ ____ : E___ ABCD ____ ____ */
				result->data |= (n & 1) << 15 | (n >> 1) << 8;
				break;
			case 6:
				/* ____ ____ ____ ____ CDE_ ____ : _AB_ ____ ____ ____ */
				result->data |= (n >> 3) << 13;
				result->addr |= (n & 7) << 5;
				break;
			case 7:
				/* ____ ____ ____ ____ ___A BCDE : ____ ____ ____ ____ */
				result->addr |= n;
				break;
		}
		/* Go around again */
	}
	return;
}


/* "Decode" an address/data pair into a structure. This is for "012345:ABCD"
 * type codes. You're more likely to find Genie codes circulating around, but
 * there's a chance you could come on to one of these. Which is nice, since
 * they're MUCH easier to implement ;) Once again, the input should be depunc-
 * tuated already. */

static void hex_decode(const char *code, struct patch *result)
{
	char *x;
	int i;
	
	/* 6 digits for address */
	for (i = 0; i < 6; i++)
	{
		if (!(x = strchr(hex_chars, code[i])))
		{
			result->addr = result->data = -1;
			return;
		}
		
		// TODO: More pointer arithmetic. Get rid of it!
		result->addr = (result->addr << 4) | ((x - hex_chars) >> 1);
	}
	
	/* 4 digits for data */
	for (i = 6; i < 10; i++)
	{
		if (!(x = strchr(hex_chars, code[i])))
		{
			result->addr = result->data = -1;
			return;
		}
		
		// TODO: More pointer arithmetic. Get rid of it!
		result->data = (result->data << 4) | ((x - hex_chars) >> 1);
	}
}


/* THIS is the function you call from the MegaDrive or whatever. This figures
 * out whether it's a genie or hex code, depunctuates it, and calls the proper
 * decoder. */
void decode(const char *code, struct patch *result)
{
	int len, i, j;
	char code_to_pass[16], *x;
	const char *ad, *da;
	int adl, dal;
	
	// Length of the code.
	len = strlen(code);
	
	/* Initialize the result */
	result->addr = result->data = 0;
	
	/* If it's 9 chars long and the 5th is a hyphen, we have a Game Genie
	 * code. */
	
	if (len == 9 && code[4] == '-')
	{
		/* Remove the hyphen and pass to genie_decode */
		code_to_pass[0] = code[0];
		code_to_pass[1] = code[1];
		code_to_pass[2] = code[2];
		code_to_pass[3] = code[3];
		code_to_pass[4] = code[5];
		code_to_pass[5] = code[6];
		code_to_pass[6] = code[7];
		code_to_pass[7] = code[8];
		code_to_pass[8] = '\0';
		genie_decode(code_to_pass, result);
		return;
	}
	
	/* Otherwise, we assume it's a hex code.
	 * Find the colon so we know where address ends and data starts. If there's
	 * no colon, then we haven't a code at all! */
	if (!(x = strchr(code, ':')))
	{
		// TODO: AHHHHHHH GOTOs
		goto bad_code;
	}
	ad = code;
	da = x + 1;
	adl = x - code;
	dal = len - adl - 1;
	
	/* If a section is empty or too long, toss it */
	if (adl == 0 || adl > 6 || dal == 0 || dal > 4)
		goto bad_code;
	
	/* Pad the address with zeros, then fill it with the value */
	for (i = 0; i < (6 - adl); ++i)
		code_to_pass[i] = '0';
	for (j = 0; i < 6; ++i, ++j)
		code_to_pass[i] = ad[j];
	
	/* Do the same for data */
	for (i = 6; i < (10 - dal); ++i)
		code_to_pass[i] = '0';
	for (j = 0; i < 10; ++i, ++j)
		code_to_pass[i] = da[j];
	
	code_to_pass[10] = '\0';
	
	/* Decode and goodbye */
	hex_decode(code_to_pass, result);
	return;
	
bad_code:
	/* AGH! Invalid code! */
	result->data = -1;
	result->addr = -1;
	return;
}


void Patch_Codes(void)
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		if ((Game_Genie_Codes[i].code[0] != 0) &&
		    (Game_Genie_Codes[i].addr != 0xFFFFFFFF) &&
		    (Game_Genie_Codes[i].active))
		{
			if (Game_Genie_Codes[i].addr < Rom_Size)
			{
				// ROM modifying code
				Rom_Data[Game_Genie_Codes[i].addr] = Game_Genie_Codes[i].data & 0xFF;
				Rom_Data[Game_Genie_Codes[i].addr + 1] = (Game_Genie_Codes[i].data & 0xFF00) >> 8;
			}
			else
			{
				// RAM modifying code
				M68K_WW(Game_Genie_Codes[i].addr, Game_Genie_Codes[i].data);
			}
		}
	}
	
	return;
}


int check_code(char *Code, unsigned int ind)
{
	if ((strlen(Code) > 11) || (strlen(Code) < 8))
		return 0;
	
	if (strlen(Code) == 8)
	{
		// Game Genie code, without hyphen.
		Game_Genie_Codes[ind].code[0] = Code[0];
		Game_Genie_Codes[ind].code[1] = Code[1];
		Game_Genie_Codes[ind].code[2] = Code[2];
		Game_Genie_Codes[ind].code[3] = Code[3];
		Game_Genie_Codes[ind].code[4] = '-';
		Game_Genie_Codes[ind].code[5] = Code[4];
		Game_Genie_Codes[ind].code[6] = Code[5];
		Game_Genie_Codes[ind].code[7] = Code[6];
		Game_Genie_Codes[ind].code[8] = Code[7];
		Game_Genie_Codes[ind].code[9] = 0;
	}
	else if (strlen(Code) == 10)
	{
		// Hex code, without colon.
		Game_Genie_Codes[ind].code[0] = Code[0];
		Game_Genie_Codes[ind].code[1] = Code[1];
		Game_Genie_Codes[ind].code[2] = Code[2];
		Game_Genie_Codes[ind].code[3] = Code[3];
		Game_Genie_Codes[ind].code[4] = Code[4];
		Game_Genie_Codes[ind].code[5] = Code[5];
		Game_Genie_Codes[ind].code[6] = ':';
		Game_Genie_Codes[ind].code[7] = Code[6];
		Game_Genie_Codes[ind].code[8] = Code[7];
		Game_Genie_Codes[ind].code[9] = Code[8];
		Game_Genie_Codes[ind].code[10] = Code[9];
		Game_Genie_Codes[ind].code[11] = 0;
	}
	else
	{
		// Other type of code.
		strcpy(Game_Genie_Codes[ind].code, Code);
	}
	
	return 1;
}


/**
 * Load_Patch_File(): Load a patch file.
 * @return 1 on success; 0 on error.
 */
int Load_Patch_File(void)
{
	FILE *Patch_File = 0;
	unsigned char *Patch_String;
	char Name[2048], Code[16], Comment[256], c;
	unsigned int i_code = 0, i_comment = 0, Ind_GG;
	unsigned int Length, Bytes_Read, i;
	
	enum etat_sec
	{
		DEB_LIGNE,
		CODE,
		BLANC,
		COMMENT,
		ERR
	} etat = DEB_LIGNE;
	
	for (i = 0; i < 256; i++)
	{
		Game_Genie_Codes[i].code[0] = 0;
		Game_Genie_Codes[i].name[0] = 0;
		Game_Genie_Codes[i].active = 0;
		Game_Genie_Codes[i].addr = 0xFFFFFFFF;
		Game_Genie_Codes[i].data = 0;
		Game_Genie_Codes[i].restore = 0xFFFFFFFF;
	}
	
	// Create the patch filename based on the ROM filename.
	strcpy(Name, Patch_Dir);
	strcat(Name, ROM_Name);
	strcat(Name, ".pat");
	
	Patch_File = fopen(Name, "rb");
	if (!Patch_File)
		return 0;
	
	fseek(Patch_File, 0, SEEK_END);
	Length = ftell(Patch_File);
	fseek(Patch_File, 0, SEEK_SET);
	
	Patch_String = (unsigned char*)malloc(Length);
	
	if (!Patch_String)
	{
		fclose(Patch_File);
		return(0);
	}
	
	// TODO: Optimize this!
	if (fread(Patch_String, Length, 1, Patch_File) == 0)
	{
		// No data available from the patch file.
		fclose(Patch_File);
		free(Patch_String);
		return 1;
	}
	
	// Data read from the patch file. Process it.
	Bytes_Read = Length;
	i = 0;
	Ind_GG = 0;
	
	while ((i < Bytes_Read) && (Ind_GG < 256))
	{
		c = Patch_String[i++];
		
		switch (etat)
		{
			case DEB_LIGNE:
				switch (c)
				{
					case '\n':
					case '\r':
					case '\t':
					case ' ':
						break;
					
					default:
						etat = CODE;
						i_code = 0;
						Code[i_code++] = c;
						break;
				}
				break;
			
			case CODE:
				switch (c)
				{
					case '\n':
					case '\r':
						Code[i_code] = 0;
						if (check_code(Code, Ind_GG))
							Ind_GG++;
						etat = DEB_LIGNE;
						break;
					
					case '\t':
					case ' ':
						Code[i_code] = 0;
						if (check_code(Code, Ind_GG))
							etat = BLANC;
						else
							etat = ERR;
						break;
					
					default:
						if (i_code < 14)
							Code[i_code++] = c;
						break;
				}
				break;
				
			case BLANC:
				switch (c)
				{
					case '\n':
						etat = DEB_LIGNE;
						Ind_GG++;
						break;
					
					case '\t':
					case ' ':
						break;
					
					default:
						i_comment = 0;
						Comment[i_comment++] = c;
						etat = COMMENT;
						break;
				}
				break;
			
			case COMMENT:
			switch (c)
			{
				case '\r':
					break;
				
				case '\n':
					Comment[i_comment] = 0;
					strcpy(Game_Genie_Codes[Ind_GG].name, Comment);
					Ind_GG++;
					etat = DEB_LIGNE;
					break;
				
				default:
					if (i_comment < 240)
						Comment[i_comment++] = c;
					break;
			}
			break;
			
			case ERR:
				switch (c)
				{
					case '\n':
						etat = DEB_LIGNE;
						break;
					
					default:
						break;
				}
				break;
		}
	}

	switch (etat)
	{
		case CODE:
			Code[i_code] = 0;
			if (check_code(Code, Ind_GG))
				Ind_GG++;
			break;
		
		case COMMENT:
			Comment[i_comment] = 0;
			strcpy(Game_Genie_Codes[Ind_GG].name, Comment);
			Ind_GG++;
			break;
		
		case DEB_LIGNE:
		case BLANC:
		case ERR:
			break;
	}
	
	fclose(Patch_File);
	free(Patch_String);
	
	if (i_code > 0)
		ice = 0;
	
	return 1;
}


/**
 * Save_Patch_File(): Save a patch file.
 * @return 1 on success; 0 on error.
 */
int Save_Patch_File(void)
{
	FILE *Patch_File;
	char Name[2048];
	int i;
	
	// Don't bother saving anything if there's no codes.
	if (Game_Genie_Codes[0].code[0] == 0)
		return 0;
	
	// Create the patch filename based on the ROM filename.
	strcpy(Name, Patch_Dir);
	strcat(Name, ROM_Name);
	strcat(Name, ".pat");
	
	Patch_File = fopen(Name, "w");
	if (!Patch_File)
	{
		printf("Error: %s\n", strerror(errno));
		return 0;
	}
	
	for (i = 0; i < 256; i++)
	{
		if (Game_Genie_Codes[i].code[0] != 0)
			fprintf(Patch_File, "%s\t%s\n", Game_Genie_Codes[i].code, Game_Genie_Codes[i].name);
	}
	
	fclose(Patch_File);
	
	return 1;
}
