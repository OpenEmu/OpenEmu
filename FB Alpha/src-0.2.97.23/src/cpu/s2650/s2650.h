/*******************************************************
 *
 *      Portable Signetics 2650 cpu emulation
 *
 *      Written by Juergen Buchmueller for use with MAME
 *
 *******************************************************/

#define PMSK    0x1fff          /* mask page offset */
#define PLEN    0x2000          /* page length */
#define PAGE    0x6000          /* mask page */
#define AMSK    0x7fff          /* mask address range */

/* processor status lower */
#define C       0x01            /* carry flag */
#define COM     0x02            /* compare: 0 binary, 1 2s complement */
#define OVF     0x04            /* 2s complement overflow */
#define WC      0x08            /* with carry: use carry in arithmetic / rotate ops */
#define RS      0x10            /* register select 0: R0/R1/R2/R3 1: R0/R4/R5/R6 */
#define IDC     0x20            /* inter digit carry: bit-3-to-bit-4 carry */
#define CC      0xc0            /* condition code */

/* processor status upper */
#define SP      0x07            /* stack pointer: indexing 8 15bit words */
#define PSU34   0x18            /* unused bits */
#define II      0x20            /* interrupt inhibit 0: allow, 1: inhibit */
#define FO      0x40            /* flag output */
#define SI      0x80            /* sense input */

#define R0      S.reg[0]
#define R1      S.reg[1]
#define R2      S.reg[2]
#define R3      S.reg[3]
