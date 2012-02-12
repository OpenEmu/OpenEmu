/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 2002 Xodnizel
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "types.h"
#include "x6502.h"
#include "cheat.h"
#include "fceu.h"
#include "file.h"
#include "cart.h"
#include "driver.h"
#include "utils/memory.h"

using namespace std;

static uint8 *CheatRPtrs[64];

vector<uint16> FrozenAddresses;		//List of addresses that are currently frozen
void UpdateFrozenList(void);		//Function that populates the list of frozen addresses
unsigned int FrozenAddressCount=0;	//Keeps up with the Frozen address count, necessary for using in other dialogs (such as hex editor)


void FCEU_CheatResetRAM(void)
{
	int x;

	for(x=0;x<64;x++)
		CheatRPtrs[x]=0;
}

void FCEU_CheatAddRAM(int s, uint32 A, uint8 *p)
{
	uint32 AB=A>>10;
	int x;

	for(x=s-1;x>=0;x--)
		CheatRPtrs[AB+x]=p-A;
}


struct CHEATF {
	struct CHEATF *next;
	char *name;
	uint16 addr;
	uint8 val;
	int compare;	/* -1 for no compare. */
	int type;	/* 0 for replace, 1 for substitute(GG). */
	int status;
};

typedef struct {
	uint16 addr;
	uint8 val;
	int compare;
	readfunc PrevRead;
} CHEATF_SUBFAST;


static CHEATF_SUBFAST SubCheats[256];
static int numsubcheats=0;
struct CHEATF *cheats=0,*cheatsl=0;


#define CHEATC_NONE     0x8000
#define CHEATC_EXCLUDED 0x4000
#define CHEATC_NOSHOW   0xC000

static uint16 *CheatComp=0;
static int savecheats;

static DECLFR(SubCheatsRead)
{
	CHEATF_SUBFAST *s=SubCheats;
	int x=numsubcheats;

	do
	{
		if(s->addr==A)
		{
			if(s->compare>=0)
			{
				uint8 pv=s->PrevRead(A);

				if(pv==s->compare)
					return(s->val);
				else return(pv);
			}
			else return(s->val);
		}
		s++;
	} while(--x);
	return(0);	/* We should never get here. */
}

void FCEUI_DisableAllCheats()
{
        struct CHEATF *next=cheats;
        uint32 x=0;

        while(next)
        {
                next->status=false;
                next=next->next;
                x++;
        }
}

void RebuildSubCheats(void)
{
	int x;
	struct CHEATF *c=cheats;
	for(x=0;x<numsubcheats;x++)
		SetReadHandler(SubCheats[x].addr,SubCheats[x].addr,SubCheats[x].PrevRead);

	numsubcheats=0;
	while(c)
	{
		if(c->type==1 && c->status)
		{
			if(GetReadHandler(c->addr)==SubCheatsRead)
			{
				/* Prevent a catastrophe by this check. */
				//FCEU_DispMessage("oops",0);
			}
			else
			{
				SubCheats[numsubcheats].PrevRead=GetReadHandler(c->addr);
				SubCheats[numsubcheats].addr=c->addr;
				SubCheats[numsubcheats].val=c->val;
				SubCheats[numsubcheats].compare=c->compare;
				SetReadHandler(c->addr,c->addr,SubCheatsRead);
				numsubcheats++;
			}
		}
		c=c->next;
	}
	FrozenAddressCount = numsubcheats;		//Update the frozen address list
	UpdateFrozenList();
	//FCEUI_DispMessage("Active Cheats: %d",0, FrozenAddresses.size()/*FrozenAddressCount*/); //Debug
}

void FCEU_PowerCheats()
{
	numsubcheats=0;	/* Quick hack to prevent setting of ancient read addresses. */
	RebuildSubCheats();
}

static void CheatMemErr(void)
{
	FCEUD_PrintError("Error allocating memory for cheat data.");
}

/* This function doesn't allocate any memory for "name" */
int AddCheatEntry(char *name, uint32 addr, uint8 val, int compare, int status, int type)
{
	struct CHEATF *temp = (struct CHEATF *)malloc(sizeof(struct CHEATF));
	if(!(temp))
	{
		CheatMemErr();
		return(0);
	}
	temp->name=name;
	temp->addr=addr;
	temp->val=val;
	temp->status=status;
	temp->compare=compare;
	temp->type=type;
	temp->next=0;

	if(cheats)
	{
		cheatsl->next=temp;
		cheatsl=temp;
	}
	else
		cheats=cheatsl=temp;

	return(1);
}

void FCEU_LoadGameCheats(FILE *override)
{
	FILE *fp;
	unsigned int addr;
	unsigned int val;
	unsigned int status;
	unsigned int type;
	unsigned int compare;
	int x;

	char linebuf[2048];
	char *namebuf;
	int tc=0;
	char *fn;

	numsubcheats=savecheats=0;

	if(override)
		fp = override;
	else
	{
		fn=strdup(FCEU_MakeFName(FCEUMKF_CHEAT,0,0).c_str());
		fp=fopen(fn,"rb");
		free(fn);
		if(!fp) return;
	}

	//FCEU_DispMessage("Cheats file loaded.",0); //Tells user a cheats file was loaded.
	//FCEU_printf("Cheats file loaded.\n",0);	 //Sends message to message log.
	while(fgets(linebuf,2048,fp)>0)
	{
		char *tbuf=linebuf;
		int doc=0;

		addr=val=compare=status=type=0;

		if(tbuf[0]=='S')
		{
			tbuf++;
			type=1;
		}
		else type=0;

		if(tbuf[0]=='C')
		{
			tbuf++;
			doc=1;
		}

		if(tbuf[0]==':')
		{
			tbuf++;
			status=0;
		}
		else status=1;

		if(doc)
		{
			char *neo=&tbuf[4+2+2+1+1+1];
			if(sscanf(tbuf,"%04x%*[:]%02x%*[:]%02x",&addr,&val,&compare)!=3)
				continue;
			namebuf = (char *)malloc(strlen(neo)+1);
			if (!namebuf)
				return;
			strcpy(namebuf,neo);
		}
		else
		{
			char *neo=&tbuf[4+2+1+1];
			if(sscanf(tbuf,"%04x%*[:]%02x",&addr,&val)!=2)
				continue;
			namebuf = (char *)malloc(strlen(neo)+1);
			if (!namebuf)
				return;
			strcpy(namebuf,neo);
		}

		for(x=0;x<(int)strlen(namebuf);x++)
		{
			if(namebuf[x]==10 || namebuf[x]==13)
			{
				namebuf[x]=0;
				break;
			}
			else if(namebuf[x]<0x20) namebuf[x]=' ';
		}

		AddCheatEntry(namebuf,addr,val,doc?compare:-1,status,type);
		tc++;
	}
	RebuildSubCheats();
	if(!override)
		fclose(fp);
}

void FCEU_FlushGameCheats(FILE *override, int nosave)
{
	if(CheatComp)
	{
		free(CheatComp);
		CheatComp=0;
	}
	if((!savecheats || nosave) && !override)	/* Always save cheats if we're being overridden. */
	{
		if(cheats)
		{
			struct CHEATF *next=cheats;
			for(;;)
			{
				struct CHEATF *last=next;
				next=next->next;
				free(last->name);
				free(last);
				if(!next) break;
			}
			cheats=cheatsl=0;
		}
	}
	else
	{
		char *fn = 0;

		if(!override)
			fn = strdup(FCEU_MakeFName(FCEUMKF_CHEAT,0,0).c_str());

		if(cheats)
		{
			struct CHEATF *next=cheats;
			FILE *fp;

			if(override)
				fp = override;
			else
				fp=fopen(fn,"wb");

			if(fp)
			{
				for(;;)
				{
					struct CHEATF *t;
					if(next->type)
						fputc('S',fp);
					if(next->compare>=0)
						fputc('C',fp);

					if(!next->status)
						fputc(':',fp);

					if(next->compare>=0)
						fprintf(fp,"%04x:%02x:%02x:%s\n",next->addr,next->val,next->compare,next->name);
					else
						fprintf(fp,"%04x:%02x:%s\n",next->addr,next->val,next->name);

					free(next->name);
					t=next;
					next=next->next;
					free(t);
					if(!next) break;
				}
				if(!override)
					fclose(fp);
			}
			else
				FCEUD_PrintError("Error saving cheats.");
			cheats=cheatsl=0;
		}
		else if(!override)
			remove(fn);
		if(!override)
			free(fn);
	}

	RebuildSubCheats();  /* Remove memory handlers. */

}


int FCEUI_AddCheat(const char *name, uint32 addr, uint8 val, int compare, int type)
{
	char *t;
	t = (char *)malloc(strlen(name)+1);
	if(!t)
	{
		CheatMemErr();
		return(0);
	}
	strcpy(t,name);
	if(!AddCheatEntry(t,addr,val,compare,1,type))
	{
		free(t);
		return(0);
	}
	savecheats=1;
	RebuildSubCheats();
	
	return(1);
}

int FCEUI_DelCheat(uint32 which)
{
	struct CHEATF *prev;
	struct CHEATF *cur;
	uint32 x=0;

	for(prev=0,cur=cheats;;)
	{
		if(x==which)          // Remove this cheat.
		{
			if(prev)             // Update pointer to this cheat.
			{
				if(cur->next)       // More cheats.
					prev->next=cur->next;
				else                // No more.
				{
					prev->next=0;
					cheatsl=prev;      // Set the previous cheat as the last cheat.
				}
			}
			else                 // This is the first cheat.
			{
				if(cur->next)       // More cheats
					cheats=cur->next;
				else
					cheats=cheatsl=0;  // No (more) cheats.
			}
			free(cur->name);     // Now that all references to this cheat are removed,
			free(cur);           // free the memory.
			break;
		}                     // *END REMOVE THIS CHEAT*


		if(!cur->next)        // No more cheats to go through(this shouldn't ever happen...)
			return(0);
		prev=cur;
		cur=prev->next;
		x++;
	}

	savecheats=1;
	RebuildSubCheats();
	return(1);
}

void FCEU_ApplyPeriodicCheats(void)
{
	struct CHEATF *cur=cheats;
	if(!cur) return;

	for(;;)
	{
		if(cur->status && !(cur->type))
			if(CheatRPtrs[cur->addr>>10])
				CheatRPtrs[cur->addr>>10][cur->addr]=cur->val;
		if(cur->next)
			cur=cur->next;
		else
			break;
	}
}


void FCEUI_ListCheats(int (*callb)(char *name, uint32 a, uint8 v, int compare, int s, int type, void *data), void *data)
{
	struct CHEATF *next=cheats;

	while(next)
	{
		if(!callb(next->name,next->addr,next->val,next->compare,next->status,next->type,data)) break;
		next=next->next;
	}
}

const char * FCEUI_GetCheatLabel(uint32 which)
{
        struct CHEATF *next=cheats;
        uint32 x=0;

        while(next)
        {
                if(x==which)
                {
                        return next->name;
                }
                next=next->next;
                x++;
        }
        return "";
}

int FCEUI_GetCheat(uint32 which, char **name, uint32 *a, uint8 *v, int *compare, int *s, int *type)
{
	struct CHEATF *next=cheats;
	uint32 x=0;

	while(next)
	{
		if(x==which)
		{
			if(name)
				*name=next->name;
			if(a)
				*a=next->addr;
			if(v)
				*v=next->val;
			if(s)
				*s=next->status;
			if(compare)
				*compare=next->compare;
			if(type)
				*type=next->type;
			return(1);
		}
		next=next->next;
		x++;
	}
	return(0);
}

static int GGtobin(char c)
{
	static char lets[16]={'A','P','Z','L','G','I','T','Y','E','O','X','U','K','S','V','N'};
	int x;

	for(x=0;x<16;x++)
		if(lets[x] == toupper(c)) return(x);
	return(0);
}

/* Returns 1 on success, 0 on failure. Sets *a,*v,*c. */
int FCEUI_DecodeGG(const char *str, int *a, int *v, int *c)
{
	uint16 A;
	uint8 V,C;
	uint8 t;
	int s;

	A=0x8000;
	V=0;
	C=0;

	s=strlen(str);
	if(s!=6 && s!=8) return(0);

	t=GGtobin(*str++);
	V|=(t&0x07);
	V|=(t&0x08)<<4;

	t=GGtobin(*str++);
	V|=(t&0x07)<<4;
	A|=(t&0x08)<<4;

	t=GGtobin(*str++);
	A|=(t&0x07)<<4;
	//if(t&0x08) return(0);	/* 8-character code?! */

	t=GGtobin(*str++);
	A|=(t&0x07)<<12;
	A|=(t&0x08);

	t=GGtobin(*str++);
	A|=(t&0x07);
	A|=(t&0x08)<<8;

	if(s==6)
	{
		t=GGtobin(*str++);
		A|=(t&0x07)<<8;
		V|=(t&0x08);

		*a=A;
		*v=V;
		*c=-1;
		return(1);
	}
	else
	{
		t=GGtobin(*str++);
		A|=(t&0x07)<<8;
		C|=(t&0x08);

		t=GGtobin(*str++);
		C|=(t&0x07);
		C|=(t&0x08)<<4;

		t=GGtobin(*str++);
		C|=(t&0x07)<<4;
		V|=(t&0x08);
		*a=A;
		*v=V;
		*c=C;
		return(1);
	}
	return(0);
}

int FCEUI_DecodePAR(const char *str, int *a, int *v, int *c, int *type)
{
	int boo[4];
	if(strlen(str)!=8) return(0);

	sscanf(str,"%02x%02x%02x%02x",boo,boo+1,boo+2,boo+3);

	*c=-1;

	if(1)
	{
		*a=(boo[3]<<8)|(boo[2]+0x7F);
		*v=0;
	}
	else
	{
		*v=boo[3];
		*a=boo[2]|(boo[1]<<8);
	}
	/* Zero-page addressing modes don't go through the normal read/write handlers in FCEU, so
	we must do the old hacky method of RAM cheats.
	*/
	if(*a<0x0100)
		*type=0;
	else
		*type=1;
	return(1);
}

/* name can be NULL if the name isn't going to be changed. */
/* same goes for a, v, and s(except the values of each one must be <0) */

int FCEUI_SetCheat(uint32 which, const char *name, int32 a, int32 v, int compare,int s, int type)
{
	struct CHEATF *next=cheats;
	uint32 x=0;

	while(next)
	{
		if(x==which)
		{
			if(name)
			{
				char *t;

				if((t=(char *)realloc(next->name,strlen(name+1))))
				{
					next->name=t;
					strcpy(next->name,name);
				}
				else
					return(0);
			}
			if(a>=0)
				next->addr=a;
			if(v>=0)
				next->val=v;
			if(s>=0)
				next->status=s;
			next->compare=compare;
			next->type=type;

			savecheats=1;
			RebuildSubCheats();

			return(1);
		}
		next=next->next;
		x++;
	}
	return(0);
}

/* Convenience function. */
int FCEUI_ToggleCheat(uint32 which)
{
	struct CHEATF *next=cheats;
	uint32 x=0;

	while(next)
	{
		if(x==which)
		{
			next->status=!next->status;
			savecheats=1;
			RebuildSubCheats();
			return(next->status);
		}
		next=next->next;
		x++;
	}

	return(-1);
}

static int InitCheatComp(void)
{
	uint32 x;

	CheatComp = (uint16*)malloc(65536*sizeof(uint16));
	if(!CheatComp)
	{
		CheatMemErr();
		return(0);
	}
	for(x=0;x<65536;x++)
		CheatComp[x]=CHEATC_NONE;

	return(1);
}

void FCEUI_CheatSearchSetCurrentAsOriginal(void)
{
	uint32 x;
	for(x=0x000;x<0x10000;x++)
		if(!(CheatComp[x]&CHEATC_NOSHOW))
		{
			if(CheatRPtrs[x>>10])
				CheatComp[x]=CheatRPtrs[x>>10][x];
			else
				CheatComp[x]|=CHEATC_NONE;
		}
}

void FCEUI_CheatSearchShowExcluded(void)
{
	uint32 x;

	for(x=0x000;x<0x10000;x++)
		CheatComp[x]&=~CHEATC_EXCLUDED;
}


int32 FCEUI_CheatSearchGetCount(void)
{
	uint32 x,c=0;

	if(CheatComp)
	{
		for(x=0x0000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW) && CheatRPtrs[x>>10])
				c++;
	}

	return c;
}
/* This function will give the initial value of the search and the current value at a location. */

void FCEUI_CheatSearchGet(int (*callb)(uint32 a, uint8 last, uint8 current, void *data),void *data)
{
	uint32 x;

	if(!CheatComp)
	{
		if(!InitCheatComp())
			CheatMemErr();
		return;
	}

	for(x=0;x<0x10000;x++)
		if(!(CheatComp[x]&CHEATC_NOSHOW) && CheatRPtrs[x>>10])
			if(!callb(x,CheatComp[x],CheatRPtrs[x>>10][x],data))
				break;
}

void FCEUI_CheatSearchGetRange(uint32 first, uint32 last, int (*callb)(uint32 a, uint8 last, uint8 current))
{
	uint32 x;
	uint32 in=0;

	if(!CheatComp)
	{
		if(!InitCheatComp())
			CheatMemErr();
		return;
	}

	for(x=0;x<0x10000;x++)
		if(!(CheatComp[x]&CHEATC_NOSHOW) && CheatRPtrs[x>>10])
		{
			if(in>=first)
				if(!callb(x,CheatComp[x],CheatRPtrs[x>>10][x]))
					break;
			in++;
			if(in>last) return;
		}
}

void FCEUI_CheatSearchBegin(void)
{
	uint32 x;

	if(!CheatComp)
	{
		if(!InitCheatComp())
		{
			CheatMemErr();
			return;
		}
	}
	for(x=0;x<0x10000;x++)
	{
		if(CheatRPtrs[x>>10])
			CheatComp[x]=CheatRPtrs[x>>10][x];
		else
			CheatComp[x]=CHEATC_NONE;
	}
}


static int INLINE CAbs(int x)
{
	if(x<0)
		return(0-x);
	return x;
}

void FCEUI_CheatSearchEnd(int type, uint8 v1, uint8 v2)
{
	uint32 x;

	if(!CheatComp)
	{
		if(!InitCheatComp())
		{
			CheatMemErr();
			return;
		}
	}


	if(!type)      // Change to a specific value.
	{
		for(x=0;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CheatComp[x]==v1 && CheatRPtrs[x>>10][x]==v2)
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}
	}
	else if(type==1)           // Search for relative change(between values).
	{
		for(x=0;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CheatComp[x]==v1 && CAbs(CheatComp[x]-CheatRPtrs[x>>10][x])==v2)
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}
	}
	else if(type==2)                          // Purely relative change.
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CAbs(CheatComp[x]-CheatRPtrs[x>>10][x])==v2)
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}
	}
	else if(type==3)                          // Any change.
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CheatComp[x]!=CheatRPtrs[x>>10][x])
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}

	}
	else if(type==4)                          // new value = known
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CheatRPtrs[x>>10][x]==v1)
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}

	}
	else if(type==5)                          // new value greater than
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CheatComp[x]<CheatRPtrs[x>>10][x])
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}

	}
	else if(type==6)                          // new value less than
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if(CheatComp[x]>CheatRPtrs[x>>10][x])
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}

	}
	else if(type==7)                          // new value greater than by known value
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if((CheatRPtrs[x>>10][x]-CheatComp[x])==v2)
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}

	}
	else if(type==8)                          // new value less than by known value
	{
		for(x=0x000;x<0x10000;x++)
			if(!(CheatComp[x]&CHEATC_NOSHOW))
			{
				if((CheatComp[x]-CheatRPtrs[x>>10][x])==v2)
				{

				}
				else
					CheatComp[x]|=CHEATC_EXCLUDED;
			}

	}
}

int FCEU_CheatGetByte(uint32 A)
{
 //  if(CheatRPtrs[A>>10])
 //   return CheatRPtrs[A>>10][A]; //adelikat-commenting this stuff out so that lua can see frozen addresses, I hope this doesn't bork stuff.
   /*else*/ if(A < 0x10000)
    return ARead[A](A);
   else
    return 0;
}

void FCEU_CheatSetByte(uint32 A, uint8 V)
{
   if(CheatRPtrs[A>>10])
    CheatRPtrs[A>>10][A]=V;
   else if(A < 0x10000)
    BWrite[A](A, V);
}

void UpdateFrozenList(void)
{
	//The purpose of this function is to keep an up to date list of addresses that are currently frozen
	//and make these accessible to other dialogs that deal with memory addresses such as
	//memwatch, hex editor, ramfilter, etc.
	
	int x;
	FrozenAddresses.clear();		//Clear vector and repopulate
	for(x=0;x<numsubcheats;x++)
	{
		FrozenAddresses.push_back(SubCheats[x].addr);
		//FCEU_printf("Address %d: %d \n",x,FrozenAddresses[x]); //Debug
	}
	//FCEUI_DispMessage("FrozenCount: %d",0,FrozenAddressCount);//Debug
}
