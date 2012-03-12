/*** konami: Portable Konami cpu emulator ******************************************/


#ifndef __KONAMI_H__
#define __KONAMI_H__


enum
{
	KONAMI_PC=1, KONAMI_S, KONAMI_CC ,KONAMI_A, KONAMI_B, KONAMI_U, KONAMI_X, KONAMI_Y,
	KONAMI_DP
};

/* PUBLIC FUNCTIONS */

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define KONAMI_RDMEM(Addr) konami_read(Addr)

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define KONAMI_WRMEM(Addr,Value) konami_write(Addr,Value)

/****************************************************************************/
/* Z80_RDOP() is identical to Z80_RDMEM() except it is used for reading     */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define KONAMI_RDOP(Addr) konami_fetch(Addr)

/****************************************************************************/
/* Z80_RDOP_ARG() is identical to Z80_RDOP() except it is used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define KONAMI_RDOP_ARG(Addr) konami_fetch(Addr)

#ifndef FALSE
#    define FALSE 0
#endif
#ifndef TRUE
#    define TRUE (!FALSE)
#endif


#endif /* __KONAMI_H__ */
