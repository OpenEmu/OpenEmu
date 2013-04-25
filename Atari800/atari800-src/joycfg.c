/******************************************************************
 Joystick configuration utility for David Firth's Atari800 emulator
 22.5.1998 Robert Golias   (golias@fi.muni.cz)
 ******************************************************************/
#include "config.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <go32.h>
#include <sys/farptr.h>
#include <dpmi.h>
#include <pc.h>
#include <conio.h>

#include "pcjoy.h"

/*color attributes*/
#define BACKGROUND 0x10
#define MENUHEADER 0xe
#define MENUITEM 0xb
#define MENUSELECT 0x0f
#define TITLE 0xf

char joysticks[JOYSTICKTYPES][21]={"Off","PC Analog joystick",
                       "LPT1 joystick","LPT2 joystick","LPT3 joystick",
                       "Keyset 0","Keyset 1","Keyset 2","Keyset 3"};

int joytypes[4]={joy_keyset0,joy_keyset1,0,0};
/*note: keyset order: fire, downleft, down, downright, left, right, upleft, up, upright*/
int keysets[4][9]={ {82,79,80,81,75,77,71,72,73},
                    {15,44,45,46,30,32,16,17,18},
                    {255,255,255,255,255,255,255,255,255},
                    {255,255,255,255,255,255,255,255,255}};

char keynames[256][11]={
/*keys 0-127 = normal scancodes*/
      "none","ESC","1","2","3","4","5","6",
      "7","8","9","0","-","+","Backspace","Tab",
      "Q","W","E","R","T","Y","U","I",
      "O","P","[","]","Enter","L Ctrl","A","S",
      "D","F","G","H","J","K","L",";",
      "\"","`","L Shift","\\","Z","X","C","V",
      "B","N","M",",",".","/","R Shift","Num *",
      "L Alt","Space","CapsLock","F1","F2","F3","F4","F5",
      "F6","F7","F8","F9","F10","NumLock","ScrLock","Num Home",
      "Num Up","Num PgDn","Num -","Num Left","Num 5","Num Right","Num +","Num End",
      "Num Down","Num PgDn","Num Ins","Num Del","Alt+PrScr","?","?","F11",
      "F12","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
/*keys 127 - 255 = extended scancodes (preceded with 0xe0) */
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","Num Enter","R Ctrl","?","?",
      "?","?","?","?","?","?","?","PrScr",
      "?","?","#","?","?","?","?","?",
      "?","?","?","?","?","Num /","?","PrScr",
      "R Alt","?","?","?","?","?","?","?",
      "?","?","?","?","?","Pause","?","Gray Home",
      "Gray Up","Gray PgUp","?","Gray Left","?","Gray Right","?","Gray End",
      "Gray Down","Gray PgDn","Gray Ins","Gray Del","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "#","#","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","?",
      "?","?","?","?","?","?","?","none"
  };
/* Note:  "?" = unknown combination
          "#" = special prefix, ignore  (like 0xe0 0xaa with Shift+Insert)
          "@" = known, but unusable
*/

/*-----------------------------------------------------------------------*/

void Screen(char *title,int attr)
{
  int i;
  window(1,1,80,25);
  textattr(attr);
  clrscr();
  gotoxy(1,1);
  cputs("旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커");
  for (i=2;i<24;i++)
  {
    gotoxy(1,i);cputs("�");
    gotoxy(80,i);cputs("�");
  }
  gotoxy(1,3);
  cputs("쳐컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴캑");
  gotoxy(1,24);
  cputs("읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸");
  gotoxy((80-strlen(title))/2,2);
  cputs(title);
  window(2,4,79,23);
}

int SelectItem(char *title,int *selection,int background,...)
{
  va_list params;
  int y;
  int i,pos;
  char *text[20];
  int lines;
  int key;

  lines=0;
  va_start(params,background);
  while (lines<20 && (text[lines]=va_arg(params,char*))!=NULL)
    lines++;
  va_end(params);

  clrscr();
  y=(20-lines)/2+2;
  gotoxy((78-strlen(title))/2,y-1);
  textattr(background+MENUHEADER);
  cputs(title);
  textattr(background+MENUITEM);
  for (i=0;i<lines;i++)
  {
     gotoxy((78-strlen(text[i]))/2,y+1+i);
     if (*text[i]!='-') cputs(text[i]);
  }
  pos=*selection;
  while (*text[pos]=='-') pos++;

  do{
    textattr(MENUSELECT);
    gotoxy((78-strlen(text[pos]))/2,y+1+pos);
    cputs(text[pos]);
    while (!kbhit());
    textattr(background+MENUITEM);
    gotoxy((78-strlen(text[pos]))/2,y+1+pos);
    cputs(text[pos]);
    key=getch();
    if (key==0) key=256+getch();
    switch(key)
    {
      case 0x148: /*Up*/
        do{
          pos--;
          if (pos==-1) pos=lines-1;
        }while (*text[pos]=='-');
        break;
      case 0x150: /*Down*/
        do{
          pos++;
          if (pos==lines) pos=0;
        }while (*text[pos]=='-');
        break;
      case 0x147: /*Home*/
        pos=0;
        while (*text[pos]=='-') pos++;
        break;
      case 0x14f: /*End*/
        pos=lines-1;
        while (*text[pos]=='-') pos--;
        break;
      case 0x149: /*PgUp*/
        if (pos>1) pos-=2;
        while (pos>0 && *text[pos]!='-') pos--;
        if (pos!=0) pos++;
        while (*text[pos]=='-') pos++;
        break;
      case 0x151: /*PgDn*/
        while (pos<lines-1 && *text[pos]!='-') pos++;
        if (pos<lines-1) pos++;
        while (*text[pos]=='-') pos--;
        break;
    }
  }while (key!=27 && key!=13 && key!=0x14d && key!=0x14b);

  *selection=pos;
  return key;
}
/*------------------------------------------------------------------------*/

_go32_dpmi_seginfo old_key_handler, new_key_handler;
volatile unsigned char raw_key=0;
volatile int ext=0,left,key;
volatile int hi,lo;
volatile int buffer[100];

void key_handler(void)
{
	asm("cli; pusha");
	raw_key = inportb(0x60);
        if (ext==2) ext=1;else /*ext 2 is used for pause*/
        if (ext==1)
        {
            left=raw_key&0x80;
            key=(raw_key&0x7f)+128;
            buffer[hi++]=key+(left<<8);
            if (hi==100) hi=0;
            ext=0;
        } else
        {
          key=raw_key&0x7f;
          left=raw_key&0x80;
          switch(raw_key)
          {
            case 0xe1:ext=2;break;
            case 0xe0:ext=1;break;
            default:
              buffer[hi++]=key+(left<<8);
              if (hi==100) hi=0;
          }
        }
	outportb(0x20, 0x20);
	asm("popa; sti");
}

void key_init(void)
{
        lo=hi=0;
	new_key_handler.pm_offset = (int) key_handler;
	new_key_handler.pm_selector = _go32_my_cs();
	_go32_dpmi_get_protected_mode_interrupt_vector(0x9, &old_key_handler);
	_go32_dpmi_allocate_iret_wrapper(&new_key_handler);
	_go32_dpmi_set_protected_mode_interrupt_vector(0x9, &new_key_handler);
}
void key_done(void)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(0x9, &old_key_handler);
}

/*------------------------------------------------------------------------*/
int pressed[256];
void showjoy(int x,int y,int num)
{
  textattr(BACKGROUND+MENUITEM);
  gotoxy(x,y);
  cprintf("Keyset %i",num);
  gotoxy(x+1,y+2);
  cputs("\\ | /");
  gotoxy(x+1,y+3);
  cputs("-   -");
  gotoxy(x+1,y+4);
  cputs("/ | \\");
  textattr(BACKGROUND+MENUHEADER);
  gotoxy(x+3,y+3);putch(pressed[keysets[num][0]]?'@':' ');
  gotoxy(x,y+5);putch(pressed[keysets[num][1]]?'@':' ');
  gotoxy(x+3,y+5);putch(pressed[keysets[num][2]]?'@':' ');
  gotoxy(x+6,y+5);putch(pressed[keysets[num][3]]?'@':' ');
  gotoxy(x,y+3);putch(pressed[keysets[num][4]]?'@':' ');
  gotoxy(x+6,y+3);putch(pressed[keysets[num][5]]?'@':' ');
  gotoxy(x,y+1);putch(pressed[keysets[num][6]]?'@':' ');
  gotoxy(x+3,y+1);putch(pressed[keysets[num][7]]?'@':' ');
  gotoxy(x+6,y+1);putch(pressed[keysets[num][8]]?'@':' ');
}
void testcollisions()
{
  int i;

  clrscr();
  textattr(BACKGROUND+MENUITEM);
  gotoxy((78-24)/2,3);
  cputs("Test keyboard collisions");
  gotoxy((78-61)/2,5);
  cputs("Here you can test, if the keys you've choosen do not collide.");
  gotoxy((78-43)/2,6);
  cputs("(only few keyboards allow all combinations)");
  gotoxy((78-18)/2,8);
  cputs("Press ESC to exit.");
  for (i=0;i<256;i++) pressed[i]=0;
  key_init();
  do{
    showjoy(17,12,0);
    showjoy(29,12,1);
    showjoy(41,12,2);
    showjoy(53,12,3);
    while (hi==lo);
    pressed[buffer[lo]&0xff]=buffer[lo]<0x100;
    lo++;
    if (lo==100) lo=0;
  }while(!pressed[1]);
  /*wait until ESC is depressed*/
  do{
    while (hi==lo);
    pressed[buffer[lo]&0xff]=buffer[lo]<0x100;
    lo++;
    if (lo==100) lo=0;
  }while(pressed[1]);
  key_done();
}
/*------------------------------------------------------------------------*/
int choosekey(char *title,int oldval)
{
  char msg[80];
  int lastkey;
  int newval;
  int bad;

  clrscr();
  sprintf(msg,"Press key for %s",title);
  gotoxy((78-strlen(msg))/2,8);
  cputs(msg);
  gotoxy(34,9);
  cputs("(ESC=none)");
  sprintf(msg,"Current setting: %s",keynames[oldval]);
  gotoxy((78-strlen(msg))/2,11);
  cputs(msg);
  while (kbhit());
  key_init();

  do{
    while (hi==lo);
    lastkey=buffer[lo++];
    if (lo==100) lo=0;
    /*do not allow F1-F12, Pause, PrintScreen or Scrollock */
    bad=(lastkey>=59 && lastkey<=68) || lastkey==87 || lastkey==88 || lastkey==167
        || lastkey==183 || lastkey==70 || lastkey==197;
  }while ((lastkey&0xff00) || *keynames[lastkey&0xff]=='?' || *keynames[lastkey&0xff]=='#' || bad);
  newval=lastkey&0xff;
  if (newval==1) newval=255;
  sprintf(msg,"Current setting: %s",keynames[newval]);
  gotoxy(1,11);
  clreol();
  gotoxy((78-strlen(msg))/2,11);
  cputs(msg);
  /*wait until key is depressed*/
  do{
    while (hi==lo);
    lastkey=buffer[lo++];
    if (lo==100) lo=0;
  }while(!(lastkey&0xff00));
  key_done();
  return newval;
}
/*------------------------------------------------------------------------*/
void setupkeyset(int no)
{
  int select,key,i;
  int keyset[9];
  char tUL[80],tU[80],tUR[80],tL[80],tR[80],tDL[80],tDR[80],tD[80],tF[80];

  for (i=0;i<9;i++) keyset[i]=keysets[no][i];
  select=0;
  do{
    sprintf(tF,"Fire: %s",keynames[keyset[0]]);
    sprintf(tDL,"Down+left: %s",keynames[keyset[1]]);
    sprintf(tD,"Down: %s",keynames[keyset[2]]);
    sprintf(tDR,"Down+right: %s",keynames[keyset[3]]);
    sprintf(tL,"Left: %s",keynames[keyset[4]]);
    sprintf(tR,"Right: %s",keynames[keyset[5]]);
    sprintf(tUL,"Up+left: %s",keynames[keyset[6]]);
    sprintf(tU,"Up: %s",keynames[keyset[7]]);
    sprintf(tUR,"Up+right: %s",keynames[keyset[8]]);

    key=SelectItem("Change settings:",&select,BACKGROUND,
        tUL,tU,tUR,tL,tR,tDL,tD,tDR,tF,"-","Clear all","-",
        "Accept changes",
        "Discard changes",NULL);
    if (key==13)
      switch(select)
      {
        case 0:keyset[6]=choosekey("up+left",keyset[6]);select++;break;
        case 1:keyset[7]=choosekey("up",keyset[7]);select++;break;
        case 2:keyset[8]=choosekey("up+right",keyset[8]);select++;break;
        case 3:keyset[4]=choosekey("left",keyset[4]);select++;break;
        case 4:keyset[5]=choosekey("right",keyset[5]);select++;break;
        case 5:keyset[1]=choosekey("down+left",keyset[1]);select++;break;
        case 6:keyset[2]=choosekey("down",keyset[2]);select++;break;
        case 7:keyset[3]=choosekey("down+right",keyset[3]);select++;break;
        case 8:keyset[0]=choosekey("fire",keyset[0]);break;
        case 10:for (i=0;i<9;i++) keyset[i]=255;break;
      }
  }while (key!=27 && !(key==13 && select>11));
  if (key==13 && select==12)
    for (i=0;i<9;i++) keysets[no][i]=keyset[i];
}
/*------------------------------------------------------------------------*/

int mainmenu(void)
{
  char joytext[4][80];
  int i,select,key;

  select=0;
  do{
    for (i=0;i<4;i++)
      sprintf(joytext[i],"Joystick %i: %s",i,joysticks[joytypes[i]]);

    key=SelectItem("Select function",&select,BACKGROUND,joytext[0],joytext[1],
               joytext[2],joytext[3],"-",
               "Configure keyset 0","Configure keyset 1",
               "Configure keyset 2","Configure keyset 3","-","Test keyboard collisions",
                "-","Save and exit","Exit without saving",NULL);
    switch(key)
    {
      case 0x14d: /*right*/
        if (select<4)
        {
          joytypes[select]++;
          if (joytypes[select]==JOYSTICKTYPES) joytypes[select]=0;
        }
        break;
      case 0x14b: /*left*/
        if (select<4)
        {
          joytypes[select]--;
          if (joytypes[select]==-1) joytypes[select]=JOYSTICKTYPES-1;
        }
        break;
      case 13: /*enter*/
        if (select<4)
        {
          joytypes[select]++;
          if (joytypes[select]==JOYSTICKTYPES) joytypes[select]=0;
        }else
        if (select<9)
        {
          setupkeyset(select-5);
        }else
        if (select==10)
        {
          testcollisions();
        }
        break;
    }
  }while(key!=27 && !(key==13 && select>=12));
  return (key==13 && select==12); /*Save only when user pressed ENTER on 'Save&Exit' */
}
/*------------------------------------------------------------------------*/
void read_config()
{
  FILE *fr;
  char string[256];
  char *ptr;
  int i,no;
  int bad=0;

  if ((fr=fopen("atari800.cfg","r"))==NULL)
  {
    printf("Error opening configuration file.\n"
           "Execute this utility from atari800 dir after configuring it!\n");
    exit(1);
  }
  while (fgets(string,sizeof(string),fr))
  {
    i=strlen(string);
    if (i >= 2 && string[i-2]=='\r' && string[i-1]=='\n') string[i-2]='\0';
    else if (i >= 1 && (string[i-1]=='\n' || string[i-1]=='\r')) string[i-1]='\0';
    ptr=strchr(string,'=');
    if (ptr)
    {
      *ptr++='\0';
      if (strncmp(string,"JOYSTICK_",9)==0)
      {
        no=string[9]-'0';
        if (no<0 || no>3) {printf("Error in config lines: %s\n",string);bad=1;}
        else{
          for (i=0;i<JOYSTICKTYPES;i++)
            if (strcmp(ptr,joyparams[i])==0) {
              joytypes[no]=i;
              break;
            }
          if (i==JOYSTICKTYPES) {printf("Unrecognized joystick type %s\n",ptr);bad=1;}
        }
      }else
      if (strncmp(string,"KEYSET_",7)==0)
      {
        no=string[7]-'0';
        if (no<0 || no>4) {printf("Error in config line: %s\n",string);bad=1;}
        else
        {
          i=sscanf(ptr,"%d %d %d %d %d %d %d %d %d",keysets[no]+0,keysets[no]+1,
                 keysets[no]+2,keysets[no]+3,keysets[no]+4,keysets[no]+5,
                 keysets[no]+6,keysets[no]+7,keysets[no]+8);
          if (i!=9) {printf("Not enough parametres for KEYSET_x: %s\n",ptr);bad=1;}
          for (i=0;i<9;i++)
            if (keysets[no][i]<0 || keysets[no][i]>255)
            {
              keysets[no][i]=255;
              printf("Invalid %i. scancode in KEYSET parameter: %s\n",i+1,ptr);
              bad=1;
            }
        }
      }/*other commands are ignored*/
    }
  } /*while*/
  fclose(fr);
  if (bad)
    exit(1);
}
void save_config()
{
  FILE *fr;
  FILE *fw;
  char string[256];
  int i;

  if ((fr=fopen("atari800.cfg","r"))==NULL)
  {
    printf("Error opening atari800.cfg!\n");
    return;
  }
  if ((fw=fopen("atari800.tmp","w"))==NULL)
  {
    printf("Error creating temporary file atari800.tmp!\n");
    fclose(fr);
    return;
  }
  while (fgets(string,sizeof(string),fr))
  {
    if (strncmp(string,"JOYSTICK_",9)!=0 && strncmp(string,"KEYSET_",7)!=0)
      fputs(string,fw);  /*copy all lines except those with JOYSTICK_ and KEYSET_ parameters*/
  }
  fclose(fr);
  i=strlen(string)-1;
  if (i>=0 && string[i]!='\n') fprintf(fw,"\n");
  for (i=0;i<4;i++)
    fprintf(fw,"KEYSET_%i=%i %i %i %i %i %i %i %i %i\n",i,
      keysets[i][0],keysets[i][1],keysets[i][2],keysets[i][3],
      keysets[i][4],keysets[i][5],keysets[i][6],keysets[i][7],
      keysets[i][8]);
  for (i=0;i<4;i++)
    fprintf(fw,"JOYSTICK_%i=%s\n",i,joyparams[joytypes[i]]);
  fclose(fw);
  if (rename("atari800.tmp","atari800.cfg")==0)
    printf("Configuration successfully saved.\n");
  else
    printf("Error when renaming atari800.tmp to atari800.cfg!\n");
}
/*------------------------------------------------------------------------*/
int main()
{
  int ok;

  read_config();
  _set_screen_lines(25);
  _setcursortype(_NOCURSOR);

  Screen("Joystick configuration utility for Atari 800 emulator",BACKGROUND+TITLE);
  ok=mainmenu();

  window(1,1,80,25);
  textattr(0xf);
  clrscr();
  _setcursortype(_NORMALCURSOR);

  if (ok) save_config();

  return 0;
}

