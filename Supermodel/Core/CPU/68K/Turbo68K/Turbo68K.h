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
 * Turbo68K: Motorola 680X0 emulator
 * Copyright 2000-2002 Bart Trzynadlowski, see "README.TXT" for terms of use
 */

#ifndef _TURBO68K_H_
#define _TURBO68K_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
* Generic Macros                                                            */

#define TURBO68K_OKAY           0       /* no error */
#define TURBO68K_NULL           0       /* null */
#define TURBO68K_SUPERVISOR     0       /* affects supervisor space */
#define TURBO68K_USER           1       /* function affects user space */
#define TURBO68K_UNINITIALIZED  15      /* uninitialized interrupt */
#define TURBO68K_SPURIOUS       24      /* spurious interrupt */
#define TURBO68K_AUTOVECTOR     256     /* autovectored interrupt */


/*****************************************************************************
* Error Codes                                                               */

#define TURBO68K_ERROR_FETCH        1   /* fetch error (PC bounds) */
#define TURBO68K_ERROR_INVINST      2   /* invalid instruction */
#define TURBO68K_ERROR_INTLEVEL     3   /* invalid interrupt level */
#define TURBO68K_ERROR_INTVECTOR    4   /* invalid interrupt vector */
#define TURBO68K_ERROR_INTPENDING   5   /* level already pending */
#define TURBO68K_ERROR_STACKFRAME   6   /* unhandled stack frame */


/*****************************************************************************
* Data Types                                                                */

typedef unsigned int    TURBO68K_UINT32;    /* unsigned 32-bit */
typedef signed int      TURBO68K_INT32;     /* signed 32-bit */
typedef unsigned short  TURBO68K_UINT16;    /* unsigned 16-bit */
typedef signed short    TURBO68K_INT16;     /* signed 16-bit */
typedef unsigned char   TURBO68K_UINT8;     /* unsigned 8-bit */
typedef signed char     TURBO68K_INT8;      /* signed 8-bit */


/*****************************************************************************
* Data Structures                                                           */

struct TURBO68K_FETCHREGION
{
    TURBO68K_UINT32 base;
    TURBO68K_UINT32 limit;
    TURBO68K_UINT32 ptr;
};

struct TURBO68K_DATAREGION
{
    TURBO68K_UINT32 base;
    TURBO68K_UINT32 limit;
    TURBO68K_UINT32 ptr;
    void            *handler;
};

struct TURBO68K_CONTEXT_68000
{
    void            *fetch, *pcfetch;
    void            *read_byte, *read_word, *read_long;
    void            *write_byte, *write_word, *write_long;
    void            *super_fetch, *super_pcfetch;
    void            *super_read_byte, *super_read_word, *super_read_long;
    void            *super_write_byte, *super_write_word, *super_write_long;
    void            *user_fetch, *user_pcfetch;
    void            *user_read_byte, *user_read_word, *user_read_long;
    void            *user_write_byte, *user_write_word, *user_write_long;
    TURBO68K_UINT32 intr[8], cycles, remaining;
    TURBO68K_UINT32 d[8], a[8], sp, sr, pc, status;
    void            *InterruptAcknowledge;
    void            *Reset;
    void            *Debug;
};

struct TURBO68K_CONTEXT_68010
{
    void            *fetch, *pcfetch;
    void            *read_byte, *read_word, *read_long;
    void            *write_byte, *write_word, *write_long;
    void            *super_fetch, *super_pcfetch;
    void            *super_read_byte, *super_read_word, *super_read_long;
    void            *super_write_byte, *super_write_word, *super_write_long;
    void            *user_fetch, *user_pcfetch;
    void            *user_read_byte, *user_read_word, *user_read_long;
    void            *user_write_byte, *user_write_word, *user_write_long;
    TURBO68K_UINT32 intr[8], cycles, remaining;
    TURBO68K_UINT32 d[8], a[8], sp, sr, pc, fc, vbr, status;
    void            *InterruptAcknowledge;
    void            *Reset, *Bkpt;
    void            *Debug;
};


/*****************************************************************************
* Functions and External Data                                               */

#define TURBO68K_ID(ID)                                                 \
                                                                        \
extern struct TURBO68K_CONTEXT_68000    ID##turbo68kcontext_68000;      \
extern struct TURBO68K_CONTEXT_68010    ID##turbo68kcontext_68010;      \
                                                                        \
TURBO68K_INT32  ID##Turbo68KInit();                                     \
TURBO68K_INT32  ID##Turbo68KReset();                                    \
TURBO68K_INT32  ID##Turbo68KRun(TURBO68K_INT32);                        \
TURBO68K_INT32  ID##Turbo68KProcessInterrupts();                        \
TURBO68K_INT32  ID##Turbo68KInterrupt(TURBO68K_INT32, TURBO68K_UINT32); \
TURBO68K_INT32  ID##Turbo68KCancelInterrupt(TURBO68K_INT32);            \
                                                                        \
TURBO68K_UINT32 ID##Turbo68KReadPC();                                   \
                                                                        \
void            ID##Turbo68KSetFetch(void *, TURBO68K_INT32);           \
void            ID##Turbo68KSetPCFetch(void *, TURBO68K_INT32);         \
void            ID##Turbo68KSetReadByte(void *, TURBO68K_INT32);        \
void            ID##Turbo68KSetReadWord(void *, TURBO68K_INT32);        \
void            ID##Turbo68KSetReadLong(void *, TURBO68K_INT32);        \
void            ID##Turbo68KSetWriteByte(void *, TURBO68K_INT32);       \
void            ID##Turbo68KSetWriteWord(void *, TURBO68K_INT32);       \
void            ID##Turbo68KSetWriteLong(void *, TURBO68K_INT32);       \
void            *ID##Turbo68KGetFetch(TURBO68K_INT32);                  \
void            *ID##Turbo68KGetPCFetch(TURBO68K_INT32);                \
void            *ID##Turbo68KGetReadByte(TURBO68K_INT32);               \
void            *ID##Turbo68KGetReadWord(TURBO68K_INT32);               \
void            *ID##Turbo68KGetReadLong(TURBO68K_INT32);               \
void            *ID##Turbo68KGetWriteByte(TURBO68K_INT32);              \
void            *ID##Turbo68KGetWriteWord(TURBO68K_INT32);              \
void            *ID##Turbo68KGetWriteLong(TURBO68K_INT32);              \
                                                                        \
TURBO68K_UINT8  *ID##Turbo68KFetchPtr(TURBO68K_UINT32);                 \
TURBO68K_UINT8  ID##Turbo68KReadByte(TURBO68K_UINT32);                  \
TURBO68K_UINT16 ID##Turbo68KReadWord(TURBO68K_UINT32);                  \
TURBO68K_UINT32 ID##Turbo68KReadLong(TURBO68K_UINT32);                  \
void            ID##Turbo68KWriteByte(TURBO68K_UINT32, TURBO68K_UINT8); \
void            ID##Turbo68KWriteWord(TURBO68K_UINT32, TURBO68K_UINT16);\
void            ID##Turbo68KWriteLong(TURBO68K_UINT32, TURBO68K_UINT32);\
                                                                        \
void            ID##Turbo68KSetContext(void *);                         \
void            ID##Turbo68KGetContext(void *);                         \
TURBO68K_UINT32 ID##Turbo68KGetContextSize();                           \
                                                                        \
void            ID##Turbo68KClearCycles();                              \
void            ID##Turbo68KFreeTimeSlice();                            \
TURBO68K_INT32  ID##Turbo68KGetElapsedCycles();                         \

TURBO68K_ID( );   /* default identifiers begin with nothing */


#ifdef __cplusplus
}
#endif

#endif  /* _TURBO68K_H_ */
