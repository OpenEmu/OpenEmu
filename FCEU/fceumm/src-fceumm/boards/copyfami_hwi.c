/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
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

/* *** COPY FAMICOM HARDWARE INTERFACE ****/

#ifdef COPYFAMI

#define NO_CACHE
#define NO_RAM

#include "__serial.h"
#include "mapinc.h"

#define FNV_32_PRIME    ((uint32)0x01000193)

#define CHR_CACHE_SIZE	(1024 * 4)
#define WRAM_CACHE_SIZE	(1024 / 2)
#define PRG_CACHE_SIZE	(1024 / 4)
#define CMD_CACHE_SIZE	(1024 * 128)

#define CMD_MAX_SIZE    (5)
#define CMD_MAX_RETEST  (16)
#define CMD_MAX_VERIFY  (16)

static uint8 *WRAM = NULL;

uint8 InitVector[] =   {0xDE, 0xAD, 0xBE, 0xEF}; /* args none,               return DE AD BE EF*/
uint8 ResetCmd[] =     {0x00};					 /* args none,				 return none*/
uint8 StateCmd[] =     {0x01};                   /* args none,               return 7 bytes status*/
uint8 StatusCmd[] =    {0x02};                   /* args none,               return 32 bytes status*/
uint8 LoadPlugCmd[] =  {0x03, 0x00, 0x00};       /* args 2b size, Nb data    return none*/
uint8 RunPlugCmd[] =   {0x04};                   /* args none,               return none*/
uint8 RunGameCmd[] =   {0x05};                   /* args none,               return none*/
uint8 NROMSave[] =     {0x06};                   /* args none,               return 16b + 32kb + 8kb*/

uint8 PRGWBCmd[] =     {0x08, 0x00, 0x00, 0x00}; /* args 2b addr, 1b data    return none*/
uint8 PRGRBCmd[] =     {0x09, 0x00, 0x00};       /* args 2b addr             return 1b data*/
uint8 CHRWBCmd[] =     {0x0A, 0x00, 0x00, 0x00}; /* args 2b addr, 1b data    return none*/
uint8 CHRRBCmd[] =     {0x0B, 0x00, 0x00};       /* args 2b addr,            return 1b data*/

uint8 PRGSUMCmd[] =    {0x10, 0x00, 0x00};       /* args 1b addr, 1b size    return (256 * N)b*/
uint8 PRG32KSUMCmd[] = {0x10, 0x80, 0x80};       /* args 1b addr, 1b size    return 32kb*/
uint8 PRG16KSUMCmd[] = {0x10, 0x00, 0x40};       /* args 1b addr, 1b size    return 16kb*/
uint8 PRG8KSUMCmd[] =  {0x10, 0x00, 0x20};       /* args 1b addr, 1b size    return 8kb*/
uint8 PRG4KSUMCmd[] =  {0x10, 0x00, 0x10};       /* args 1b addr, 1b size    return 4kb*/

uint8 CHRSUMCmd[] =    {0x11, 0x00, 0x00};       /* args 1b addr, 1b size    return (256 * N)b*/
uint8 CHR8KSUMCmd[] =  {0x11, 0x00, 0x20};       /* args 1b addr, 1b size    return 8kb*/
uint8 CHR4KSUMCmd[] =  {0x11, 0x00, 0x10};       /* args 1b addr, 1b size    return 4kb*/
uint8 CHR2KSUMCmd[] =  {0x11, 0x00, 0x08};       /* args 1b addr, 1b size    return 2kb*/
uint8 CHR1KSUMCmd[] =  {0x11, 0x00, 0x04};       /* args 1b addr, 1b size    return 1kb*/

uint8 PRGGetCmd[] =    {0x12, 0x00, 0x00};       /* args 1b addr, 1b size    return (256 * N)b*/
uint8 PRG32KGetCmd[] = {0x12, 0x80, 0x80};       /* args 1b addr, 1b size    return 32kb*/
uint8 PRG16KGetCmd[] = {0x12, 0x00, 0x40};       /* args 1b addr, 1b size    return 16kb*/
uint8 PRG8KGetCmd[] =  {0x12, 0x00, 0x20};       /* args 1b addr, 1b size    return 8kb*/
uint8 PRG4KGetCmd[] =  {0x12, 0x00, 0x10};       /* args 1b addr, 1b size    return 4kb*/

uint8 CHRGetCmd[] =    {0x13, 0x00, 0x00};       /* args 1b addr, 1b size    return (256 * N)b*/
uint8 CHR8KGetCmd[] =  {0x13, 0x00, 0x20};       /* args 1b addr, 1b size    return 8kb*/
uint8 CHR4KGetCmd[] =  {0x13, 0x00, 0x10};       /* args 1b addr, 1b size    return 4kb*/
uint8 CHR2KGetCmd[] =  {0x13, 0x00, 0x08};       /* args 1b addr, 1b size    return 2kb*/
uint8 CHR1KGetCmd[] =  {0x13, 0x00, 0x04};       /* args 1b addr, 1b size    return 1kb*/

uint8 CPUTestCmd[] =   {0x14, 0x00, 0x00};       /* args 1b addr, 1b size    return (2b + 1b) * N + 3b*/

typedef struct {
  int32 mirror;
  int32 chrsum[8];
  int32 prgsum[4];
} SYNC_STATE;

static SYNC_STATE    state_cur, state_new, state_def;

typedef struct {
  uint8 *buf;
  int32 count;
} DATA_BANKS;

static DATA_BANKS  chr_data;
static int32       chr_bank[0x10000];
static DATA_BANKS  prg_data;
static int32       prg_bank[0x10000];

typedef struct {
  SYNC_STATE   states[CMD_CACHE_SIZE];
  int32        seqs[CMD_CACHE_SIZE][CMD_MAX_SIZE];
  int32        count;
} SYNC_CMDS;

typedef struct {
  int32        seq[CMD_MAX_SIZE];
  int32        size;
  int32        found;
  uint32       hash;
  uint16       hashf;
} SYNC_CMD;

static SYNC_CMD     cmd;
static SYNC_CMDS    cmds;

typedef struct {
  int32 index;
  int32 size;
  int32 retest;
  int32 verify; 
} CMD_CACHE;

static CMD_CACHE   cmd_cache[0x10000];

static SFORMAT StateRegs[]=
{
  {state_cur.chrsum, sizeof(state_cur.chrsum), "CHRREG"},
  {state_cur.prgsum, sizeof(state_cur.prgsum), "ROMREG"},
  {&state_cur.mirror, sizeof(state_cur.mirror), "MIRREG"},
  {0}
};

#define MI_U 4

static char *mirror_names[5] = {"Horizontal", "Vertical", "Mirror 0", "Mirror 1", "Unknown mirror"};
static int32 mirror_modes[16] = {
  MI_0, MI_U, MI_U, MI_H, MI_U, MI_V, MI_U, MI_U,
  MI_U, MI_U, MI_U, MI_U, MI_U, MI_U, MI_U, MI_1 };

#define CHRDEF(slot)   (chr_bank[state_def.chrsum[slot]])
#define PRGDEF(slot)   (prg_bank[state_def.prgsum[slot]])
#define CHRCUR(slot)   (chr_bank[state_cur.chrsum[slot]])
#define PRGCUR(slot)   (prg_bank[state_cur.prgsum[slot]])
#define CHRNEW(slot)   (chr_bank[state_new.chrsum[slot]])
#define PRGNEW(slot)   (prg_bank[state_new.prgsum[slot]])

static void GetStatus(SYNC_STATE *state)
{
  uint8  resp0;
  uint16 resp1, i;
  SEND(StatusCmd);
  GET(resp0, 1);
  state->mirror = resp0;
  GET(resp0, 1);
  for(i=0; i<8; i++) {
    GET(resp1, 2);
    state->chrsum[i] = resp1;
  }
  for(i=0; i<4; i++) {
    GET(resp1, 2);
    state->prgsum[i] = resp1;
  }  
}

static int32 FetchNewCHRBank(int32 slot)
{
  FILE *ofile;
  char name[256];
  int32 bank = chr_data.count++;
  CHR1KGetCmd[1] = slot << 2;
  SENDGET(CHR1KGetCmd, chr_data.buf[bank * 1024], 1024);
  sprintf(name,"%04x.chr",bank);
  ofile=fopen(name,"wb");
  fwrite((void *)&chr_data.buf[bank * 1024], 1, 1024, ofile);
  fclose(ofile);
  return bank;
}

static int32 FetchNewPRGBank(int32 slot)
{
  FILE *ofile;
  char name[256];
  int32 bank = prg_data.count++;
  PRG8KGetCmd[1] = 0x80 + (slot << 5);
  SENDGET(PRG8KGetCmd, prg_data.buf[bank * 8192], 8192);
  sprintf(name,"%04x.prg",bank);
  ofile=fopen(name,"wb");
  fwrite((void *)&prg_data.buf[bank * 8192], 1, 8192, ofile);
  fclose(ofile);
  return bank;
}

static int CheckStatus(void)
{
  int32 i, ischanged = 0;
  GetStatus(&state_new);
  if(state_cur.mirror != state_new.mirror) {
    state_cur.mirror = state_new.mirror;
#ifdef FCEU_LOG
    FCEU_printf(">> mirror changed to %s (%02X)\n",mirror_names[mirror_modes[state_cur.mirror]], state_cur.mirror);
#endif
    ischanged = 1;
  } else {
    state_new.mirror = -1;
  }
  for(i=0; i<8; i++) {
    if(state_cur.chrsum[i] != state_new.chrsum[i]) {
      state_cur.chrsum[i] = state_new.chrsum[i];
      if(CHRCUR(i) == -1) {
        CHRCUR(i) = FetchNewCHRBank(i);
#ifdef FCEU_LOG
        FCEU_printf(">> chr[%d] bank %d loaded\n", i, CHRCUR(i));
#endif
      }
#ifdef FCEU_LOG
      else
        FCEU_printf(">> chr[%d] bank %d switched\n", i, CHRCUR(i));
#endif
      ischanged = 1;
    } else {
      state_new.chrsum[i] = -1;
    }
  }
  for(i=0; i<4; i++) {
    if(state_cur.prgsum[i] != state_new.prgsum[i]) {
      state_cur.prgsum[i] = state_new.prgsum[i];                                
      if(PRGCUR(i) == -1) {
        PRGCUR(i) = FetchNewPRGBank(i);
#ifdef FCEU_LOG
        FCEU_printf(">> prg[%d] bank %d loaded\n", i, PRGCUR(i));
#endif
      }
#ifdef FCEU_LOG
      else
        FCEU_printf(">> prg[%d] bank %d switched\n", i, PRGCUR(i));
#endif
      ischanged = 1;
    } else {
      state_new.prgsum[i] = -1;
    }
  }
  return ischanged;
}

#ifndef NO_CACHE
static void ApplyStatus()
{
  int32 i;
  if ((cmds.states[cmd.found].mirror != -1) && (cmds.states[cmd.found].mirror != state_cur.mirror)) {
    state_cur.mirror = cmds.states[cmd.found].mirror;
    setmirror(mirror_modes[state_cur.mirror]);
#ifdef FCEU_LOG
    FCEU_printf(">> mirror changed to %s (%02X)\n",mirror_names[mirror_modes[state_cur.mirror]], state_cur.mirror);
#endif
  }
  for(i=0; i<8; i++) {
    int32 sum = cmds.states[cmd.found].chrsum[i];
    if (sum != -1) {
      if (sum != state_cur.chrsum[i]) {
        state_cur.chrsum[i] = sum;
        setchr1r(1, i * 1024, CHRCUR(i));
#ifdef FCEU_LOG
        FCEU_printf(">> chr[%d] bank %d switched\n", i, chr_bank[sum]);
#endif
      }
      else
#ifdef FCEU_LOG
        FCEU_printf(">> chr[%d] bank %d switched the same\n", i, chr_bank[sum]);
    }
#endif
  }
  for(i=0; i<4; i++) {
    int32 sum = cmds.states[cmd.found].prgsum[i];
    if (sum != -1) {
      if (sum != state_cur.prgsum[i]) {
        state_cur.prgsum[i] = sum;
        setprg8r(2, 0x8000 + (i * 8192), PRGCUR(i));
#ifdef FCEU_LOG
        FCEU_printf(">> prg[%d] bank %d switched\n", i, prg_bank[sum]);
#endif
      }
      else
#ifdef FCEU_LOG
        FCEU_printf(">> prg[%d] bank %d switched the same\n", i, prg_bank[sum]);
    }
#endif
  }
}

static void LogCmd()
{
  int32 i;
  FCEU_printf(">> new cmd size %d [", cmd_cache[cmd.hashf].size);
  for(i=0; i<cmd_cache[cmd.hashf].size; i++)
    FCEU_printf(" %06X",cmds.seqs[cmd.found][i]);
  FCEU_printf(" ], switched to (");    
  if (cmds.states[cmd.found].mirror != -1)
    FCEU_printf(" mirror=%s",mirror_names[mirror_modes[cmds.states[cmd.found].mirror]]);
  for(i=0; i<8; i++)
    if (cmds.states[cmd.found].chrsum[i] != -1)
      FCEU_printf(" chr%d=%02X", i, chr_bank[cmds.states[cmd.found].chrsum[i]]);
  for(i=0; i<4; i++)
    if (cmds.states[cmd.found].prgsum[i] != -1)
      FCEU_printf(" prg%d=%02X", i, prg_bank[cmds.states[cmd.found].prgsum[i]]);
  FCEU_printf(" )\n");    
}
#endif
static void Sync()
{
  setchr1r(1, 0x0000, CHRCUR(0));
  setchr1r(1, 0x0400, CHRCUR(1));
  setchr1r(1, 0x0800, CHRCUR(2));
  setchr1r(1, 0x0C00, CHRCUR(3));
  setchr1r(1, 0x1000, CHRCUR(4));
  setchr1r(1, 0x1400, CHRCUR(5));
  setchr1r(1, 0x1800, CHRCUR(6));
  setchr1r(1, 0x1C00, CHRCUR(7));
#ifndef NO_RAM
  setprg8r(1, 0x6000, 0);
#endif
  setprg8r(2, 0x8000, PRGCUR(0));
  setprg8r(2, 0xA000, PRGCUR(1));
  setprg8r(2, 0xC000, PRGCUR(2));
  setprg8r(2, 0xE000, PRGCUR(3));
  setmirror(mirror_modes[state_cur.mirror]);
}
#ifndef NO_CACHE
static void UpdateCmd(uint32 val)
{
  int32 index;
  if(cmd.size < CMD_MAX_SIZE) {
    index = cmd.size++;
  } else {
    /* если достигнут максимум для команды, выбросить последнуюю, добавить новую,*/
	/* продолжать до банксвичинга*/
	cmd.hash = 0;
	for(index = 0; index < (CMD_MAX_SIZE - 1); index++) {
	  cmd.seq[index] = cmd.seq[index + 1];
      cmd.hash *= FNV_32_PRIME;
      cmd.hash ^= cmd.seq[index];
	}
  }
    cmd.seq[index] = val;
    cmd.hash *= FNV_32_PRIME;
    cmd.hash ^= val;
    cmd.hashf = (cmd.hash >> 16) ^ (cmd.hash & 0xffff);
    cmd.found = cmd_cache[cmd.hashf].index;
  }
#endif

static DECLFW(MCopyFamiWrite)
{
#ifndef NO_CACHE
  int32 i;
#endif

#ifdef FCEU_LOG
  FCEU_printf("> WRITE %04X:%02X\n",A,V);
#endif

  PRGWBCmd[1] = A & 0xFF;
  PRGWBCmd[2] = A >> 8;
  PRGWBCmd[3] = V & 0xFF;
  SEND(PRGWBCmd);
#ifdef NO_CACHE
  CheckStatus();
  Sync();
#else
  UpdateCmd((A << 8) | V);
  /* ищем команду в кеше*/
  if(cmd.found == -1) {
    /* не найдена, проверяем, изменилось ли состояние банков*/
    /* либо не предельной ли она длины для команды*/
    cmd_cache[cmd.hashf].index = cmd.found = cmds.count++;
    cmd_cache[cmd.hashf].retest = 0;
    cmd_cache[cmd.hashf].verify = 0;
    for(i=0; i<cmd.size; i++)
      cmds.seqs[cmd.found][i] = cmd.seq[i];
    cmd_cache[cmd.hashf].size = cmd.size;
    if(CheckStatus()) {
      cmds.states[cmd.found] = state_new;
      LogCmd();
      cmd.size = 0;
      cmd.hash = 0;
      Sync();
    } else {
      /* если добавлена полная команда без банксвитчинга*/
      cmd_cache[cmd.hashf].index = -2;
    }
  } else if(cmd.found == -2) {
    /* частичное совпадение, если число проверок не превысило лимит*/
    if(cmd_cache[cmd.hashf].retest < CMD_MAX_RETEST) {
      /* то проверим состояние банков*/
      if(CheckStatus()) {
        /* изменилось, запишем новую команду*/
        cmd_cache[cmd.hashf].index = cmd.found = cmds.count++;
        cmd_cache[cmd.hashf].retest = 0;
        cmd_cache[cmd.hashf].verify = 0;
        for(i=0; i<cmd.size; i++)
          cmds.seqs[cmd.found][i] = cmd.seq[i];
        cmd_cache[cmd.hashf].size = cmd.size;
        cmds.states[cmd.found] = state_new;
        LogCmd();
        cmd.size = 0;
        cmd.hash = 0;
        Sync();
      } else {
        /* не изменилось, отметим увеличим счетчик проверок*/
        cmd_cache[cmd.hashf].retest++;
      }
    }
  } else {
    /* найдена, последний рубеж обороны от говна*/
/*
    if(cmd_cache[cmd.hashf].verify < CMD_MAX_VERIFY) {
      if(CheckStatus()) {
        int32 changed = 0;
      /* если есть изменения, сравним новое состояние с записанным*/
        if(cmds.states[cmd.found].mirror != state_new.mirror)
          changed = 1;
        for(i=0; i<8; i++)
          if(cmds.states[cmd.found].chrsum[i] != state_new.chrsum[i])
            changed = 1;
        for(i=0; i<4; i++)
          if(cmds.states[cmd.found].prgsum[i] != state_new.prgsum[i])
            changed = 1;
        if(changed) {
          cmd_cache[cmd.hashf].index = -1;
          cmd_cache[cmd.hashf].retest = 0;
          cmd_cache[cmd.hashf].verify = 0;
          Sync();
        }
      } else
        cmd_cache[cmd.hashf].verify++;
    } else */ {
      /* применяем без малейшего зазрения совести*/
      ApplyStatus();
      cmd.size = 0;
      cmd.hash = 0;
    }
  }
#endif
}

static DECLFR(MCopyFamiRead)
{
  uint8 result;
  PRGRBCmd[1] = A & 0xFF;
  PRGRBCmd[2] = A >> 8;
  SENDGET(PRGRBCmd, result, 1);
#ifdef FCEU_LOG
  FCEU_printf("> READ %04X:%02X\n",A,result);
#endif
  return result;
}

static void MCopyFamiReset(void)
{
  state_cur = state_def;
  Sync();
}

static void MCopyFamiPower(void)
{
/*  uint32 resp, presp;*/

  FCEU_printf("NOW POWERING... ");
  
  Sync();
  
  SetWriteHandler(0x4018, 0x7fff, MCopyFamiWrite);
  SetReadHandler(0x4018, 0x7fff, MCopyFamiRead);
#ifndef NO_RAM
  SetWriteHandler(0x6000, 0x7fff, CartBW);
  SetReadHandler(0x6000, 0x7fff, CartBR);
#endif

/*
  FCEU_printf("READING MEMORY MAP...\n");
  CPUTestCmd[1] = 0x50;
  CPUTestCmd[2] = 0x30;
  SEND(CPUTestCmd);
  resp = 0;
  presp = 0xffffffff;
  while (presp != 0x00ff0000) {
    GET(resp, 3);
    if(presp != 0xffffffff) {
      switch(presp & 0x00FF0000) {
        case 0x00000000: /* BUS*/
          FCEU_printf("  %04X-%04X OPEN BUS\n",presp & 0x7fff, (resp - 1) & 0x7fff);
          break;
        case 0x00010000: /* RAM*/
          FCEU_printf("  %04X-%04X RAM\n",presp & 0x7fff, (resp - 1) & 0x7fff);
          SetWriteHandler(presp & 0x7fff, (resp - 1) & 0x7fff, CartBW);
          SetReadHandler(presp & 0x7fff, (resp - 1) & 0x7fff, CartBR);
          break;
      }      
    }
    presp = resp;
  }
*/
  SetWriteHandler(0x8000, 0xffff, MCopyFamiWrite);
  SetReadHandler(0x8000, 0xffff, CartBR);

  FCEU_printf("DONE!\nNOW COLLECTING DATA...\n");
}

static void MCopyFamiClose(void)
{
  if(chr_data.buf)
    free(chr_data.buf);
  chr_data.buf=NULL;
  if(prg_data.buf)
    free(prg_data.buf);
  prg_data.buf=NULL;
  if(WRAM)
    free(WRAM);
  WRAM = NULL;
  
  SerialClose();
}

static void StateRestore(int version)
{
  Sync();
}

void MapperCopyFami_Init(CartInfo *info)
{
  uint32 resp = 0, i, size;

  memset(chr_bank, -1, sizeof(chr_bank));
  memset(prg_bank, -1, sizeof(chr_bank));
  memset(cmd_cache, -1, sizeof(cmd_cache));
  memset(&cmds, 0, sizeof(cmds));
  memset(&cmd, 0, sizeof(cmd));
  
  info->Reset=MCopyFamiReset;
  info->Power=MCopyFamiPower;
  info->Close=MCopyFamiClose;
  GameStateRestore=StateRestore;

  size = 1024 * CHR_CACHE_SIZE;	/* размер страницы 1кб*/
  chr_data.buf = (uint8*)FCEU_gmalloc(size);
  SetupCartCHRMapping(1, chr_data.buf, size, 1); /* проверять ппу рам, иначе игра может портить данные*/
  AddExState(chr_data.buf, size, 0, "COPYCHR");

  size = 8192; /* размер страницы 8кб*/
  WRAM = (uint8*)FCEU_gmalloc(size);
  SetupCartPRGMapping(1, WRAM, size, 1);
  AddExState(WRAM, size, 0, "COPYWRAM");

  size = 8192 * PRG_CACHE_SIZE; /* размер страницы 8кб*/
  prg_data.buf = (uint8*)FCEU_gmalloc(size);
  SetupCartPRGMapping(2, prg_data.buf, size, 0);
  AddExState(prg_data.buf, size, 0, "COPYPRG");


  FCEU_printf("WAITING FOR SERIAL PORT... ");
  while(!SerialOpen(19, 921600)) { Sleep(500); }
  FCEU_printf("READY!\n");

  FCEU_printf("WAITING FOR DEVICE... ");

  while(resp != *(uint32 *)&InitVector[0]) {
    SEND(ResetCmd);
    SENDGET(InitVector, resp, 4);
    Sleep(500);
  }

  FCEU_printf("READY!\n");
  FCEU_printf("READING STATUS...\n");
  GetStatus(&state_cur);
  FCEU_printf("MIRRORING IS %s (%02X)\n",mirror_names[mirror_modes[state_cur.mirror]], state_cur.mirror);
  FCEU_printf("READING CHR...\n INITIAL STATE:");
    
  for(i=0; i<8; i++) {
    if(CHRCUR(i) == -1)
      CHRCUR(i) = FetchNewCHRBank(i); 
    FCEU_printf(" CHR%d=%02X", i, CHRCUR(i));
  }
  FCEU_printf("\n");
      
  FCEU_printf("READING PRG...\n INITIAL STATE:");
  for(i=0; i<4; i++) {
    if(PRGCUR(i) == -1)
      PRGCUR(i) = FetchNewPRGBank(i);
    FCEU_printf(" PRG%d=%02X", i, PRGCUR(i));
  }
  FCEU_printf("\nDONE!\n");
  
  state_def = state_cur;

  AddExState(&StateRegs, ~0, 0, 0);
}

#endif
