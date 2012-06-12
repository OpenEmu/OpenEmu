/******************************************************************************
*
* FILENAME: boom6502.h
*
* DESCRIPTION:  This contains defs and headers form  6502 processor functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
******************************************************************************/
#ifndef boom5200_h
#define boom5200_h

#define WORD   unsigned int
#define BYTE   unsigned int
#define UINT8  unsigned char
#define UINT16  unsigned short
#define UINT32  unsigned int

/******************************/
/* Define interupt addresses  */
/******************************/
#define INT_NONE   0x0000U
#define INT_IRQ    0xfffeU
#define INT_NMI    0xfffaU
#define INT_RESET  0xfffcU
#define INT_QUIT   0xffffU

/******************************/
/* Define Debug constants     */
/******************************/
#define BEFORE_OP  1
#define DURING_OP  2
#define AFTER_OP   3
#define READ_MEM   1
#define WRITE_MEM  2

/* define 6502 status register */
#define B6502_CARRY_FLG     0x01
#define B6502_ZERO_FLG      0x02
#define B6502_INTERPT_FLG   0x04
#define B6502_DECIMAL_FLG   0x08
#define B6502_BREAK_FLG     0x10
#define B6502_RESRVD_FLG    0x20
#define B6502_OVRFLW_FLG    0x40
#define B6502_SIGN_FLG      0x80

/* define inverses of status  */
#define NOT_B6502_CARRY_FLG     0xfe
#define NOT_B6502_ZERO_FLG      0xfd
#define NOT_B6502_INTERPT_FLG   0xfb
#define NOT_B6502_DECIMAL_FLG   0xf7
#define NOT_B6502_BREAK_FLG     0xef
#define NOT_B6502_RESRVD_FLG    0xdf
#define NOT_B6502_OVRFLW_FLG    0xbf
#define NOT_B6502_SIGN_FLG      0x7f

/* Get flag from status reg   */
#define GET_B6502_CARRY   ((B6502_CARRY_FLG & cpu6502.status) ? 1 : 0)
#define GET_B6502_ZERO    ((B6502_ZERO_FLG & cpu6502.status) ? 1 : 0)
#define GET_B6502_INTERPT ((B6502_INTERPT_FLG & cpu6502.status) ? 1 : 0)
#define GET_B6502_DECIMAL ((B6502_DECIMAL_FLG & cpu6502.status) ? 1 : 0)
#define GET_B6502_BREAK   ((B6502_BREAK_FLG & cpu6502.status) ? 1 : 0)
#define GET_B6502_OVRFLW  ((B6502_OVRFLW_FLG & cpu6502.status) ? 1 : 0)
#define GET_B6502_SIGN    ((B6502_SIGN_FLG & cpu6502.status) ? 1 : 0)

/* Test flag from status reg  */
#define IF_B6502_CARRY   (B6502_CARRY_FLG & cpu6502.status)
#define IF_B6502_ZERO    (B6502_ZERO_FLG & cpu6502.status)
#define IF_B6502_INTERPT (B6502_INTERPT_FLG & cpu6502.status)
#define IF_B6502_DECIMAL (B6502_DECIMAL_FLG & cpu6502.status)
#define IF_B6502_BREAK   (B6502_BREAK_FLG & cpu6502.status)
#define IF_B6502_OVRFLW  (B6502_OVRFLW_FLG & cpu6502.status)
#define IF_B6502_SIGN    (B6502_SIGN_FLG & cpu6502.status)

/*
 * Escape Codes for Patches
 */
#define ESC_CODE_SIO  1

struct cpu {

	int i_req;
	int irq_flag;
	int i_cnt;
	BYTE status;
	WORD pc;
	WORD a_reg;
	WORD x_reg;
	WORD y_reg;
	WORD s_reg;
	WORD t_reg;
	WORD h_reg;
	WORD stack[ 0x100 ];
	WORD stack_ptr;
	void ( *do_instruct[ 0x100 ] ) ( void );
	BYTE ( *read_mem[ 0x100 ] ) ( WORD addr );
	void ( *write_mem[ 0x100 ] ) ( WORD addr, BYTE value );
	int op_cycles[ 0x100 ];
	char op_name[ 0x100 ][ 15 ];
};

struct debug_cpu {
	BYTE stop_on_addr_match_flag;
	BYTE write_debug_flag;
	WORD stop_on_match_value;
	BYTE ram_dump_flag;
	BYTE single_step_flag;
	BYTE debug_on;
};

/* Functions used for the 6502 */
void reset_6502 ( void  );
void run_6502 ( void );
void int_6502 ( void );
void set_irq_flag_6502 ( void );
void clear_irq_flag_6502 ( void );
void set_int_6502 ( WORD interupt  );
void set_irq_flag_6502 ( void  );
void clear_irq_flag_6502 ( void  );
BYTE cpu_get_stack_value ( BYTE address  );
void cpu_set_stack_value ( BYTE address, BYTE value  );
void assign_ops ( void );

#endif
