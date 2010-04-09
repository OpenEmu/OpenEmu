/* Mednafen - Multi-system Emulator
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

#include "main.h"
#include <ctype.h>
#include <trio/trio.h>
#include "console.h"


static SDL_Thread *CheatThread = NULL;
static SDL_mutex *CheatMutex = NULL;
static bool isactive = 0;
static char *pending_text = NULL;

class CheatConsoleT : public MDFNConsole
{
	public:

	CheatConsoleT(void)
	{
	 SetShellStyle(1);
	 SetSmallFont(0);
	}

        virtual bool TextHook(UTF8 *text)
        {
	 SDL_mutexP(CheatMutex);
	 pending_text = strdup((char *)text);
	 SDL_mutexV(CheatMutex);
         return(1);
        }
        void Draw(SDL_Surface *surface, const SDL_Rect *src_rect)
	{
	 SDL_mutexP(CheatMutex);
	 MDFNConsole::Draw(surface, src_rect);
	 SDL_mutexV(CheatMutex);
	}
	void WriteLine(UTF8 *text)
	{
	 SDL_mutexP(CheatMutex);
	 MDFNConsole::WriteLine(text);
 	 SDL_mutexV(CheatMutex);
	}
        void AppendLastLine(UTF8 *text)
        {
         SDL_mutexP(CheatMutex);
         MDFNConsole::AppendLastLine(text);
         SDL_mutexV(CheatMutex);
        }
};

static CheatConsoleT CheatConsole;


static void CHEAT_printf(const char *format, ...)
{
 char temp[2048];

 va_list ap;

 va_start(ap, format);
 trio_vsnprintf(temp, 2048, format, ap);

 CheatConsole.WriteLine((UTF8*) temp);
 //MDFND_PrintError(temp);

 va_end(ap);
}

static void CHEAT_puts(const char *string)
{
 CheatConsole.WriteLine((UTF8 *)string);
}

static void CHEAT_gets(char *s, int size)
{
 SDL_mutexP(CheatMutex);
 while(!pending_text)
 {
  SDL_mutexV(CheatMutex);
  SDL_Delay(5);
  SDL_mutexP(CheatMutex);
 }
 strncpy(s, pending_text, size - 1);
 s[size - 1] = 0;
 free(pending_text);
 pending_text = NULL;
 CheatConsole.AppendLastLine((UTF8*)s);
 SDL_mutexV(CheatMutex);
}

static char CHEAT_getchar(char def)
{
 uint8 buf[2];

 CHEAT_gets((char *)buf, 2);
 if(buf[0] == 0)
  return(def);
 return(buf[0]);
}

static void GetString(char *s, int max)
{
 CHEAT_gets(s,max);
}

static uint64 GetUI(uint64 def)
{
 char buf[64];

 memset(buf, 0, sizeof(buf));

 CHEAT_gets(buf,64);

 if(!buf[0])
  return(def);

 if(buf[0] == '$')
  trio_sscanf(buf + 1, "%llx", &def);	// $0FCE
 else if(buf[0] == '0' && tolower(buf[1]) == 'x')
  trio_sscanf(buf + 2, "%llx", &def);	// 0x0FCE
 else if(tolower(buf[strlen(buf) - 1]) == 'h') // 0FCEh
  trio_sscanf(buf, "%llx", &def);
 else
  trio_sscanf(buf,"%lld", &def);

 return def;
}


static int GetYN(int def)
{
 char buf[32];
 CHEAT_printf("(Y/N)[%s]: ",def?"Y":"N");
 CHEAT_gets(buf,32);
 if(buf[0]=='y' || buf[0]=='Y')
  return(1);
 if(buf[0]=='n' || buf[0]=='N')
  return(0);
 return(def);
}

/*
**	Begin list code.
**
*/
static int listcount;
static int listids[10];
static int listsel;
static int mordoe;

void BeginListShow(void)
{
 listcount=0;
 listsel=-1;
 mordoe=0;
}

/* Hmm =0 for in list choices, hmm=1 for end of list choices. */
/* Return equals 0 to continue, -1 to stop, otherwise a number. */
int ListChoice(int hmm)
{
  char buf[32];

  if(!hmm)
  {
   int num=0;

   tryagain:
   CHEAT_printf(" <'Enter' to continue, (S)top, or #> ");
   CHEAT_gets(buf,32);

   if(buf[0]=='s' || buf[0]=='S') return(-1);
   if(!buf[0]) return(0);
   if(!trio_sscanf(buf,"%d",&num))
    return(0);  
   if(num<1) goto tryagain;
   return(num);
  }
  else
  {
   int num=0;

   tryagain2:
   CHEAT_printf(" <'Enter' to make no selection, or #> ");
   CHEAT_gets(buf,32);
   if(!buf[0]) return(0);
   if(!trio_sscanf(buf,"%d",&num))
    return(0);
   if(num<1) goto tryagain2;
   return(num);
  }
}

int EndListShow(void)
{
  if(mordoe)
  {
   int r=ListChoice(1);
   if(r>0 && r<=listcount)
    listsel=listids[r-1];
  }
  return(listsel);
}

/* Returns 0 to stop listing, 1 to continue. */
int AddToList(char *text, uint32 id)
{
 if(listcount==10)
 {
  int t=ListChoice(0);
  mordoe=0;

  if(t==-1) return(0);  // Stop listing.
  else if(t>0 && t<11)
  {
   listsel=listids[t-1];
   return(0);
  }
  listcount=0;
 }
 mordoe=1;
 listids[listcount]=id;
 CHEAT_printf("%2d) %s",listcount+1,text);
 listcount++; 
 return(1);
}

/*
**	
**	End list code.
**/

typedef struct MENU {
	const char *text;
	void *action;
	int type;	// 0 for menu, 1 for function.
} MENU;

static void SetOC(void)
{
 MDFNI_CheatSearchSetCurrentAsOriginal();
}

static void UnhideEx(void)
{
 MDFNI_CheatSearchShowExcluded();
}

static void ToggleCheat(int num)
{
 CHEAT_printf("Cheat %d %sabled.",1+num,
 MDFNI_ToggleCheat(num)?"en":"dis");
}

static void ModifyCheat(int num)
{
 char *name;
 char buf[256];
 uint32 A;
 uint64 V;
 uint64 compare;
 char type;
 int status;
 unsigned int bytelen;
 bool bigendian;

 MDFNI_GetCheat(num, &name, &A, &V, &compare, &status, &type, &bytelen, &bigendian);

 CHEAT_printf("Name [%s]: ",name);
 GetString(buf,256);

 /* This obviously doesn't allow for cheats with no names.  Bah.  Who wants
    nameless cheats anyway... 
 */

 if(buf[0])
  name=buf;	// Change name when MDFNI_SetCheat() is called.
 else
  name=0;	// Don't change name when MDFNI_SetCheat() is called.

 CHEAT_printf("Address [$%08x]: ",(unsigned int)A);
 A=GetUI(A);

 CHEAT_printf("Byte length [%d]: ", bytelen);
 bytelen = GetUI(bytelen);

 if(bytelen > 1)
 {
  CHEAT_printf("Big endian? [%c]: ", bigendian ? 'Y' : 'N');
  bigendian = GetYN(bigendian);
 }
 else
  bigendian = 0;

 CHEAT_printf("Value [%03lld]: ",(unsigned int)V);
 V=GetUI(V);

 
 do
 {
  CHEAT_printf("Type('R'=replace,'S'=Read Substitute(or 'C' with compare)) [%c]: ",type);
  type = toupper(CHEAT_getchar(type));
 } while(type != 'R' && type !='S' && type !='C');

 if(type == 'C')
 {
  CHEAT_printf("Compare [%03lld]: ",compare);
  compare = GetUI(compare);
 }

 CHEAT_printf("Enable? ");
 status = GetYN(status);

 MDFNI_SetCheat(num, name, A, V, compare, status, type, bytelen, bigendian);
}

bool MDFNI_DecodeGBGG(const char *str, uint32 *a, uint8 *v, uint8 *c, char *type);
static void AddCheatGGPAR(int which)
{
 uint32 A;
 uint8 V;
 uint8 C;
 char type;
 char name[256],code[256];

 CHEAT_printf("Name: ");
 GetString(name,256); 
 
 CHEAT_printf("Code: ");
 GetString(code,256);

 CHEAT_printf("Add cheat \"%s\" for code \"%s\"?",name,code);
 if(GetYN(0))
 {
  if(which)
  {
   if(!MDFNI_DecodePAR(code,&A,&V,&C,&type))
   {
    CHEAT_puts("Invalid Game Genie code.");
    return;
   }
  }
  else 
  {
   if(!strcmp(CurGame->shortname, "gb"))
   {
    if(!MDFNI_DecodeGBGG(code, &A, &V, &C, &type))
    {
     CHEAT_puts("Invalid Game Genie code.");
     return;
    }
   }
   else
   {
    if(!MDFNI_DecodeGG(code,&A,&V,&C, &type))
    {
     CHEAT_puts("Invalid Game Genie code.");
     return;
    }
   }
  }

  if(MDFNI_AddCheat(name,A,V,C,type, 1, 0))
   CHEAT_puts("Cheat added.");
  else
   CHEAT_puts("Error adding cheat.");
 }
}

static void AddCheatGG(void)
{
 AddCheatGGPAR(0);
}

static void AddCheatPAR(void)
{  
 AddCheatGGPAR(1); 
}

static void AddCheatParam(uint32 A, uint64 V, unsigned int bytelen, bool bigendian)
{
 char name[256];

 CHEAT_printf("Name: ");
 GetString(name,256);

 CHEAT_printf("Address [$%08x]: ", A);
 A=GetUI(A);

 CHEAT_printf("Byte length [%d]: ", bytelen);
 bytelen = GetUI(bytelen);

 if(bytelen > 1)
 {
  CHEAT_printf("Big endian? [%c]: ", bigendian ? 'Y' : 'N');
  bigendian = GetYN(bigendian);
 }
 else
  bigendian = 0;

 CHEAT_printf("Value [%llu]: ", V);
 V=GetUI(V);

 CHEAT_printf("Add cheat \"%s\" for address $%08x with value %llu?",name,(unsigned int)A,(unsigned long long)V);
 if(GetYN(0))
 {
  if(MDFNI_AddCheat(name,A,V,0, 'R', bytelen, bigendian))
   CHEAT_puts("Cheat added.");
  else
   CHEAT_puts("Error adding cheat.");
 }
}

static void AddCheat(void)
{
 AddCheatParam(0, 0, 1, 0);
}

static int lid;
static int clistcallb(char *name, uint32 a, uint64 v, uint64 compare, int s, char type, unsigned int length, bool bigendian, void *data)
{
 char tmp[512];
 int ret;

 if(type == 'C')
  trio_snprintf(tmp, 512, "%s   $%08x:%03lld:%03lld - %s",s?"*":" ",a,v,compare,name);
 else
  trio_snprintf(tmp, 512, "%s   $%08x:%03lld     - %s",s?"*":" ",a,v,name);

 if(type != 'R')
  tmp[2]='S';

 ret = AddToList(tmp,lid);
 lid++;
 return(ret);
}

static void ListCheats(void)
{
 int which;
 lid=0;

 BeginListShow();
 MDFNI_ListCheats(clistcallb,0);
 which=EndListShow();
 if(which>=0)
 {
  char tmp[32];
  CHEAT_printf(" <(T)oggle status, (M)odify, or (D)elete this cheat.> ");
  CHEAT_gets(tmp,32);
  switch(tolower(tmp[0]))
  {
   case 't':ToggleCheat(which);
	    break;
   case 'd':if(!MDFNI_DelCheat(which))
 	     CHEAT_puts("Error deleting cheat!");
	    else 
	     CHEAT_puts("Cheat has been deleted.");
	    break;
   case 'm':ModifyCheat(which);
	    break;
  }
 }
}

static void ResetSearch(void)
{
 MDFNI_CheatSearchBegin();
 CHEAT_puts("Done.");
}

static unsigned int searchbytelen = 1;
static bool searchbigendian = 0;

static int srescallb(uint32 a, uint64 last, uint64 current, void *data)
{
 char tmp[256];

 if(searchbytelen == 8)
  trio_snprintf(tmp, 256, "$%08x:%020llu:%020llu",(unsigned int)a,(unsigned long long)last,(unsigned long long)current);
 if(searchbytelen == 7)
  trio_snprintf(tmp, 256, "$%08x:%017llu:%017llu",(unsigned int)a,(unsigned long long)last,(unsigned long long)current);
 if(searchbytelen == 6)
  trio_snprintf(tmp, 256, "$%08x:%015llu:%015llu",(unsigned int)a,(unsigned long long)last,(unsigned long long)current);
 if(searchbytelen == 5)
  trio_snprintf(tmp, 256, "$%08x:%013llu:%013llu",(unsigned int)a,(unsigned long long)last,(unsigned long long)current);
 if(searchbytelen == 4)
  trio_snprintf(tmp, 256, "$%08x:%10u:%10u",(unsigned int)a,(unsigned int)last,(unsigned int)current);
 else if(searchbytelen == 3)
  trio_snprintf(tmp, 256, "$%08x:%08u:%08u",(unsigned int)a,(unsigned int)last,(unsigned int)current);
 else if(searchbytelen == 2)
  trio_snprintf(tmp, 256, "$%08x:%05u:%05u",(unsigned int)a,(unsigned int)last,(unsigned int)current);
 else if(searchbytelen == 1)
  trio_snprintf(tmp, 256, "$%08x:%03u:%03u",(unsigned int)a,(unsigned int)last,(unsigned int)current);
 else // > 4
  trio_snprintf(tmp, 256, "$%08x:%020llu:%020llu",(unsigned int)a,(unsigned long long)last,(unsigned long long)current);
 return(AddToList(tmp,a));
}

static void ShowRes(void)
{
 int n=MDFNI_CheatSearchGetCount();
 CHEAT_printf(" %d results:",n);
 if(n)
 {
  int which;
  BeginListShow();
  MDFNI_CheatSearchGet(srescallb, 0);
  which=EndListShow();
  if(which>=0)
   AddCheatParam(which,0, searchbytelen, searchbigendian);
 }
}

static int ShowShortList(const char *moe[], unsigned int n, int def)
{
 unsigned int x;
 int c;
 unsigned int baa;
 char tmp[256];

 red:
 for(x=0;x<n;x++)
  CHEAT_printf("%d) %s",x+1,moe[x]);
 CHEAT_puts("D) Display List");
 clo:

 CHEAT_puts("");
 CHEAT_printf("Selection [%d]> ",def+1);
 CHEAT_gets(tmp,256);
 if(!tmp[0])
  return def;
 c=tolower(tmp[0]);
 baa=c-'1';

 if(baa<n)
  return baa;
 else if(c=='d')
  goto red;
 else
 {
  CHEAT_puts("Invalid selection.");
  goto clo;
 }
}

static void DoSearch(void)
{
 static int v1=0,v2=0;
 static int method=0;

 const char *m[6]={"O==V1 && C==V2","O==V1 && |O-C|==V2","|O-C|==V2","O!=C","Value decreased","Value increased"};
 CHEAT_puts("");
 CHEAT_printf("Search Filter:");

 method = ShowShortList(m,6,method);

 if(method<=1)
 {
  CHEAT_printf("V1 [%03d]: ",v1);
  v1=GetUI(v1);
 }

 if(method<=2)
 {
  CHEAT_printf("V2 [%03d]: ",v2);
  v2=GetUI(v2);
 }

 CHEAT_printf("Byte length(1-8)[%1d]: ", searchbytelen);
 searchbytelen = GetUI(searchbytelen);

 if(searchbytelen > 1)
 {
  CHEAT_printf("Big endian? [%c]: ", searchbigendian ? 'Y' : 'N');
  searchbigendian = GetYN(searchbigendian);
 }
 else
  searchbigendian = 0;

 MDFNI_CheatSearchEnd(method, v1, v2, searchbytelen, searchbigendian);
 CHEAT_puts("Search completed.");
}

static void DoMenu(MENU *men, bool topmost = 0)
{
 int x=0;

 redisplay:
 x=0;
 CHEAT_puts("");
 while(men[x].text)
 {
  CHEAT_printf("%d) %s",x+1,men[x].text);
  x++;
 }
 CHEAT_puts("D) Display Menu");
 if(!topmost)
  CHEAT_puts("X) Return to Previous");

 {
  char buf[32];
  int c;

  recommand:
  CHEAT_printf("Command> ");
  CHEAT_gets(buf,32);
  c=tolower(buf[0]);
  if(c == 0)
   goto recommand;
  else if(c=='d')
   goto redisplay;
  else if(c=='x' && !topmost)
  {
   return;
  }
  else if(trio_sscanf(buf,"%d",&c))
  {
   if(c>x) goto invalid;
   if(men[c-1].type)
   {
    void (*func)(void)=(void(*)())men[c-1].action;
    func();
   }
   else
    DoMenu((MENU*)men[c-1].action);	/* Mmm...recursivey goodness. */
   goto redisplay;
  }
  else
  {
   invalid:
   CHEAT_puts("Invalid command.");
   goto recommand;
  }

 }
}

int CheatLoop(void *arg)
{
 MENU NewCheatsMenuNES[] =
 {
  {"Add Cheat",(void *)AddCheat, 1},
  {"Reset Search",(void *)ResetSearch, 1},
  {"Do Search",(void *)DoSearch, 1},
  {"Set Original to Current",(void *)SetOC, 1},
  {"Unhide Excluded",(void *)UnhideEx, 1},
  {"Show Results",(void *)ShowRes, 1},
  {"Add Game Genie Cheat",(void *)AddCheatGG, 1},
  {"Add PAR Cheat",(void *)AddCheatPAR, 1},
  {0}
 };

 MENU NewCheatsMenuGB[] =
 {
  {"Add Cheat",(void *)AddCheat, 1},
  {"Reset Search",(void *)ResetSearch, 1},
  {"Do Search",(void *)DoSearch, 1},
  {"Set Original to Current",(void *)SetOC, 1},
  {"Unhide Excluded",(void *)UnhideEx, 1},
  {"Show Results",(void *)ShowRes, 1},
  {"Add Game Genie Cheat",(void *)AddCheatGG, 1},
  {0}
 };

 MENU NewCheatsMenu[] =
 {
  {"Add Cheat",(void *)AddCheat, 1},
  {"Reset Search",(void *)ResetSearch, 1},
  {"Do Search",(void *)DoSearch, 1},
  {"Set Original to Current",(void *)SetOC, 1},
  {"Unhide Excluded",(void *)UnhideEx, 1},
  {"Show Results",(void *)ShowRes, 1},
  {0}
 };

 MENU *thenewcm = NewCheatsMenu;

 if(!strcmp(CurGame->shortname, "nes"))
  thenewcm = NewCheatsMenuNES;
 else if(!strcmp(CurGame->shortname, "gb"))
  thenewcm = NewCheatsMenuGB;

 MENU MainMenu[] = {
  {"List Cheats",(void *)ListCheats,1},
  {"New Cheats...",(void *)thenewcm,0},
  {0}
 };

 DoMenu(MainMenu, 1);

 return(1);
}

void ShowConsoleCheatConfig(bool show)
{
 if(!CheatMutex)
  CheatMutex = SDL_CreateMutex();

 PauseGameLoop(show);
 if(show)
 {
  if(!CheatThread)
   CheatThread = SDL_CreateThread(CheatLoop, NULL);
 }
 isactive = show;
}

bool IsConsoleCheatConfigActive(void)
{
 return(isactive);
}

void DrawCheatConsole(SDL_Surface *surface, const SDL_Rect *src_rect)
{
 if(!isactive) return;
 CheatConsole.Draw(surface, src_rect);
}

int CheatEventHook(const SDL_Event *event)
{
 if(!isactive) return(1);

 return(CheatConsole.Event(event));
}

