/******************************************************************************
*
* FILENAME: boom6502.c
*
* DESCRIPTION:  Implementation of 6502 processor.  The read and write to mem
*               functions will be implemented elsewhere and be system dependant
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.3.0   04/06/06  bberlin      Added ASO and SHX instructions (opcode 0x1f)
* 0.4.0   04/23/06  bberlin      Change Branch instructions to use extra
*                                  cycle when branch taken.
* 0.4.0   06/02/06  bberlin      AND instruction not setting sign bit
* 0.6.0   10/04/08  bberlin      Added INS instruction (opcode 0xff)
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "boom6502.h"
#include "memory.h"
#include "sio.h"
#include "debug.h"
#include "../kat5200.h"

#define IMMEDIATE   
#define ABSOLUTE    
#define ZPAGE       
#define ABSOLUTE_X  
#define ABSOLUTE_Y  
#define INDIRECT_X  
#define INDIRECT_Y  
#define ZPAGE_X     
#define ZPAGE_Y     

struct cpu cpu6502;
extern struct debug debug;

/******************************************************************************
**  Function   :  reset_6502                                           
**                                                                    
**  Objective  :  This function will reset registers to initial values
**                                                                   
**  Parameters :  NONE                                              
**                                                                 
**  return     :  NONE                                            
******************************************************************************/ 
void reset_6502 ( void  ) {

	/* Put pointers to instruction functions in array */
	assign_ops();

	cpu6502.a_reg = cpu6502.x_reg = cpu6502.y_reg = 0;
	cpu6502.status = 0x22;
	cpu6502.stack_ptr = 0xff;
	cpu6502.irq_flag = 0;

	cpu6502.pc = rd_6502(INT_RESET) + ( rd_6502((INT_RESET + 1)) << 8 );

	cpu6502.i_cnt = NTSC_SCREEN_TIME - NTSC_V_BLANK_TIME;
	cpu6502.i_req = INT_NONE;

} /* end reset_6502 */

/******************************************************************************
**  Function   :  int_6502                              
**                                                     
**  Objective  :  This function will process interupts
**                                                   
**  Parameters :  NONE                              
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void int_6502 ( void  ) {

	if ( cpu6502.i_req == INT_NMI ) {
		cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = (cpu6502.pc >> 8) & 0xff;
		cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.pc & 0xff;
		cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.status;
		cpu6502.pc = rd_6502(INT_NMI) + ( rd_6502((INT_NMI + 1)) << 8 );
		cpu6502.i_req = INT_NONE;
	}

	if ( cpu6502.i_req == INT_IRQ ) {
		if ( cpu6502.status & B6502_INTERPT_FLG ) {
			cpu6502.i_req = INT_NONE;
			return;
		}

		cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = (cpu6502.pc >> 8) & 0xff;
		cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.pc & 0xff;
		cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.status;
		cpu6502.pc = rd_6502(INT_IRQ) + ( rd_6502((INT_IRQ + 1)) << 8 );
		cpu6502.i_req = INT_NONE;
	}

} /* end int_6502 */

/******************************************************************************
**  Function   :  set_int_6502                          
**                                                     
**  Objective  :  This function sets what interupt to perform
**                                                   
**  Parameters :  NONE                              
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void set_int_6502 ( WORD interupt  ) {

	cpu6502.i_req = interupt;

} /* end set_int_6502 */

/******************************************************************************
**  Function   :  set_irq_flag_6502                     
**                                                     
**  Objective  :  This function sets the irq held down flag  
**                                                   
**  Parameters :  NONE                              
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void set_irq_flag_6502 ( void  ) {

	cpu6502.irq_flag = 1;

	/* Go ahead and try to interupt */
	if ( !IF_B6502_INTERPT ) {
		cpu6502.i_req = INT_IRQ;
		int_6502 ();
	}

	/* Otherwise CLI will check for irq flag and interupt */

} /* end set_irq_flag_6502 */

/******************************************************************************
**  Function   :  clear_irq_flag_6502                     
**                                                     
**  Objective  :  This function clears the irq held down flag
**                                                   
**  Parameters :  NONE                              
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void clear_irq_flag_6502 ( void  ) {

	cpu6502.irq_flag = 0;

} /* end clear_irq_flag_6502 */

/******************************************************************************
**  Function   :  cpu_get_stack_value                          
**                                                     
**  Objective  :  This function gets a value from the stack
**                                                   
**  Parameters :  NONE                              
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
BYTE cpu_get_stack_value ( BYTE address  ) {

	return cpu6502.stack [ address ];

} /* end cpu_get_stack_value */

/******************************************************************************
**  Function   :  cpu_get_stack_value                          
**                                                     
**  Objective  :  This function sets a value in the stack     
**                                                   
**  Parameters :  NONE                              
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void cpu_set_stack_value ( BYTE address, BYTE value  ) {

	cpu6502.stack [ address ] = value;

} /* end cpu_set_stack_value */

/******************************************************************************
**  Function   :  run_6502               
**                                  
**  Objective  :  This function will run 6502 code for the number of cycles set
**                                       
**  Parameters :  NONE                  
**                                     
**  return     :  0 if success        
**                1 if failure       
******************************************************************************/ 
void run_6502 ( void ) {

	while ( cpu6502.i_cnt > 0 ) {

		/*
		 * Get Next Instruction 
		 */
		cpu6502.t_reg = rd_6502(cpu6502.pc);
		cpu6502.i_cnt -= cpu6502.op_cycles[ cpu6502.t_reg ];

		/*
		 * Do Instruction
		 */
		cpu6502.pc++;
		if ( debug.single_step_flag ) {
			debug_print_value ( "cpu" );
			debug_get_input ();
		}
		else if ( debug.trace_cpu_flag ) {
			debug_print_cpu_op ( BEFORE_OP );
		}
		cpu6502.do_instruct[ cpu6502.t_reg ] ();
	}

} /* end run_6502 */

/***************************************
*    6502 instruction Set              *
*                                      *
*    pointers to each function will be *
*    stored in an array                *
****************************************/

/******
* ADC *
******/
/* opcode 0x69 */
void adc_d_aa ( void ) { 
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502(cpu6502.pc) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502(cpu6502.pc) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502(cpu6502.pc)) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc++;
	cpu6502.status |= ((cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0);
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x65 */
void adc_aa ( void ) {
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_z_6502(rd_6502(cpu6502.pc)) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_z_6502(rd_6502(cpu6502.pc)) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_z_6502(rd_6502(cpu6502.pc))) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc++;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x75 */
void adc_aa_x ( void ) {
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) + (cpu6502.x_reg)) & 0xff) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) + (cpu6502.x_reg)) & 0xff) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;	
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) + (cpu6502.x_reg)) & 0xff)) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc++;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x6D */
void adc_aaaa ( void ) {
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8))) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc+=2;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x7D */
void adc_aaaa_x ( void ) { 
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff)) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff)) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff))) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc+=2;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x79 */
void adc_aaaa_y ( void ) {
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff)) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff)) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff))) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc+=2;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x61 */
void adc_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8)) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8)) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;	
	}
	else {	
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8))) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc++;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0x71 */
void adc_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	if ( cpu6502.status & B6502_DECIMAL_FLG ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0xf) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff)) & 0xf) + GET_B6502_CARRY;
		if ( cpu6502.t_reg > 9 )
			cpu6502.t_reg += 6;
		cpu6502.t_reg += (cpu6502.a_reg & 0xf0) + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff)) & 0xf0);

		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
		cpu6502.status |= ((cpu6502.a_reg + cpu6502.s_reg + GET_B6502_CARRY) & 0xff) ? 0 : B6502_ZERO_FLG;

		if ( cpu6502.t_reg > 0x9f )
			cpu6502.t_reg += 0x60;	
	}
	else {
		cpu6502.t_reg = cpu6502.a_reg + (cpu6502.s_reg = rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff))) + GET_B6502_CARRY;
		cpu6502.status = cpu6502.status & (~( B6502_CARRY_FLG | B6502_OVRFLW_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG ));
		cpu6502.status |= (((cpu6502.t_reg & 0xff) == 0) ? B6502_ZERO_FLG : 0);
		cpu6502.status |= ((cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0);
		cpu6502.status |= (((~(cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	}
	cpu6502.pc++;
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg = cpu6502.t_reg & 0xff;
}

/******
* AND *
******/
/* opcode 0x29 */
void and_d_aa ( void ) { 
	cpu6502.a_reg &= rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x25 */
void and_aa ( void ) { 
	cpu6502.a_reg &= rd_z_6502(rd_6502(cpu6502.pc));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x35 */
void and_aa_x ( void ) { 
	cpu6502.a_reg &= rd_6502((rd_6502(cpu6502.pc) + cpu6502.x_reg) & 0xff);
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x2D */
void and_aaaa ( void ) { 
	cpu6502.a_reg &= rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x3D */
void and_aaaa_x ( void ) { 
	cpu6502.a_reg &= rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + cpu6502.x_reg);
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x39 */
void and_aaaa_y ( void ) { 
	cpu6502.a_reg &= rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + cpu6502.y_reg);
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x21 */
void and_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	cpu6502.a_reg &= rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x31 */
void and_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.a_reg &= rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + cpu6502.y_reg);
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}

/******
* ASL *
******/
/* opcode 0x0A */
void asl_A ( void ) { 
	cpu6502.a_reg <<= 1; 
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg &= 0xff; 
	cpu6502.status |= (cpu6502.a_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x06 */
void asl_aa ( void ) { 
	cpu6502.t_reg = rd_z_6502(cpu6502.h_reg = rd_6502(cpu6502.pc)) << 1;
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff; 
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
	cpu6502.status |= (cpu6502.t_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x16 */
void asl_aa_x ( void ) { 
	cpu6502.t_reg = rd_6502( cpu6502.h_reg = ((rd_6502(cpu6502.pc) + cpu6502.x_reg)) & 0xff ) << 1;
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff; 
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
	cpu6502.status |= (cpu6502.t_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x0E */
void asl_aaaa ( void ) { 
	cpu6502.t_reg = rd_6502( cpu6502.h_reg = (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) ) << 1;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff; 
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
	cpu6502.status |= (cpu6502.t_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x1E */
void asl_aaaa_x ( void ) { 
	cpu6502.t_reg = rd_6502( cpu6502.h_reg = ((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + cpu6502.x_reg) ) << 1;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff; 
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
	cpu6502.status |= (cpu6502.t_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
}

/******
* AS0 *
******/
/* opcode 0x1F */
void aso_aaaa_x ( void ) { 
	cpu6502.t_reg = rd_6502( cpu6502.h_reg = ((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + cpu6502.x_reg) ) << 1;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff; 
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
	cpu6502.a_reg |= cpu6502.t_reg;
	cpu6502.status |= (cpu6502.t_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
}

/* opcode 0x07 */
void aso_aa ( void ) { 
	cpu6502.t_reg = rd_z_6502(cpu6502.h_reg = rd_6502(cpu6502.pc)) << 1;
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_CARRY_FLG | B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0xf00) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff; 
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
	cpu6502.a_reg |= cpu6502.t_reg;
	cpu6502.status |= (cpu6502.t_reg == 0 ? B6502_ZERO_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
}

/******
* BCC *
******/
/* opcode 0x90 */
void bcc ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( !IF_B6502_CARRY ) {
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* BCS *
******/
/* opcode 0xB0 */
void bcs ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( IF_B6502_CARRY ) {
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* BEQ *
******/
/* opcode 0xF0 */
void beq ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( IF_B6502_ZERO ) {
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* BIT *
******/
/* opcode 0x24 */
void bit_aa ( void ) { 
	cpu6502.t_reg = rd_z_6502(rd_6502(cpu6502.pc));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0x40) ? B6502_OVRFLW_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & cpu6502.a_reg) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x2C */
void bit_aaaa ( void ) { 
	cpu6502.t_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0x40) ? B6502_OVRFLW_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & cpu6502.a_reg) ? 0 : B6502_ZERO_FLG;
}

/******
* BMI *
******/
/* opcode 0x30 */
void bmi ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( IF_B6502_SIGN ) {
		cpu6502.i_cnt--;
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* BNE *
******/
/* opcode 0xD0 */
void bne ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( !IF_B6502_ZERO ) {
		cpu6502.i_cnt--;
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* BPL *
******/
/* opcode 0x10 */
void bpl ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( !IF_B6502_SIGN ) {
		cpu6502.i_cnt--;
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else 
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}


/******
* BRK *
******/
/* opcode 0x00 */
void brk ( void ) { 
	cpu6502.pc++;
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = (cpu6502.pc >> 8) & 0xff;
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.pc & 0xff;
	cpu6502.status |= B6502_BREAK_FLG;
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.status;
	cpu6502.status |= B6502_INTERPT_FLG;
	cpu6502.pc = rd_6502(INT_IRQ) + ( rd_6502((INT_IRQ + 1)) << 8 );
//	printf ( "break encountered\n" );
//	printf ( "CPU has crashed\n" );
//	debug_close_default_trace ();
//	debug_crash_to_gui ();
}


/******
* BVC *
******/
/* opcode 0x50 */
void bvc ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( !IF_B6502_OVRFLW ) {
		cpu6502.i_cnt--;
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* BVS *
******/
/* opcode 0x70 */
void bvs ( void ) { 
	cpu6502.t_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	if ( IF_B6502_OVRFLW ) {
		cpu6502.i_cnt--;
		if ( cpu6502.t_reg & 0x80 )
			cpu6502.pc -= (~(cpu6502.t_reg) + 1) & 0xff;
		else
			cpu6502.pc += cpu6502.t_reg & 0x7f;
	}
}

/******
* CLC *
******/
/* opcode 0x18 */
void clc ( void ) { 
	cpu6502.status &= NOT_B6502_CARRY_FLG;
}

/******
* CLD *
******/
/* opcode 0xD8 */
void cld ( void ) { 
	cpu6502.status &= NOT_B6502_DECIMAL_FLG;
}

/******
* CLI *
******/
/* opcode 0x58 */
void cli ( void ) { 
	cpu6502.status &= NOT_B6502_INTERPT_FLG;
	if ( cpu6502.irq_flag ) {
		cpu6502.i_req = INT_IRQ;
		int_6502 ();
	}
}

/******
* CLV *
******/
/* opcode 0xB8 */
void clv ( void ) { 
	cpu6502.status &= NOT_B6502_OVRFLW_FLG;
}

/******
* CMP *
******/
/* opcode 0xC9 */
void cmp_d_aa ( void ) { 
	cpu6502.t_reg = (rd_6502( cpu6502.pc )) & 0xff; 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xC5 */
void cmp_aa ( void ) { 
	cpu6502.t_reg = rd_z_6502(rd_6502( cpu6502.pc )) & 0xff; 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xD5 */
void cmp_aa_x ( void ) { 
	cpu6502.t_reg = rd_6502((rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff ); 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xCD */
void cmp_aaaa ( void ) { 
	cpu6502.t_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) & 0xff;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xDD */
void cmp_aaaa_x ( void ) { 
	cpu6502.t_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff)) & 0xff;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xD9 */
void cmp_aaaa_y ( void ) { 
	cpu6502.t_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff)) & 0xff;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xC1 */
void cmp_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	cpu6502.t_reg = rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8)) & 0xff;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xD1 */
void cmp_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.t_reg = rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff)) & 0xff;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.a_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.a_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.a_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}

/******
* CPX *
******/
/* opcode 0xE0 */
void cpx_d_aa ( void ) { 
	cpu6502.t_reg = rd_6502( cpu6502.pc ) & 0xff; 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.x_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.x_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.x_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}

/* opcode 0xE4 */
void cpx_aa ( void ) { 
	cpu6502.t_reg = rd_z_6502(rd_6502( cpu6502.pc )) & 0xff; 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.x_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.x_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.x_reg + ~(cpu6502.t_reg)  + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xEC */
void cpx_aaaa ( void ) { 
	cpu6502.t_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) & 0xff;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.x_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.x_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.x_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}

/******
* CPY *
******/
/* opcode 0xC0 */
void cpy_d_aa ( void ) { 
	cpu6502.t_reg = rd_6502( cpu6502.pc ) & 0xff; 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.y_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.y_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.y_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xC4 */
void cpy_aa ( void ) { 
	cpu6502.t_reg = rd_z_6502(rd_6502( cpu6502.pc )) & 0xff; 
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.y_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.y_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.y_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}
/* opcode 0xCC */
void cpy_aaaa ( void ) { 
	cpu6502.t_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) & 0xff;
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	if ( cpu6502.y_reg > cpu6502.t_reg )
		cpu6502.status |= B6502_CARRY_FLG;
	else if ( cpu6502.y_reg == cpu6502.t_reg )
		cpu6502.status |= (B6502_CARRY_FLG | B6502_ZERO_FLG);
	if ( (cpu6502.y_reg + ~(cpu6502.t_reg) + 1) & 0x80 )
		cpu6502.status |= B6502_SIGN_FLG;
}

/******
* DEC *
******/
/* opcode 0xC6 */
void dec_aa ( void ) { 
	cpu6502.h_reg = rd_6502( cpu6502.pc ); 
	cpu6502.pc++;
	cpu6502.t_reg = (rd_z_6502(cpu6502.h_reg) + 0xff) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}
/* opcode 0xD6 */
void dec_aa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff; 
	cpu6502.pc++;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 0xff) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}
/* opcode 0xCE */
void dec_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.pc+=2;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 0xff) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}
/* opcode 0xDE */
void dec_aaaa_x ( void ) { 
	cpu6502.h_reg = (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff);
	cpu6502.pc+=2;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 0xff) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}

/******
* DEX *
******/
/* opcode 0xCA */
void dex ( void ) { 
	cpu6502.x_reg += 0xff;
	cpu6502.x_reg &= 0xff;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* DEY *
******/
/* opcode 0x88 */
void dey ( void ) { 
	cpu6502.y_reg += 0xff;
	cpu6502.y_reg &= 0xff;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* EOR *
******/
/* opcode 0x49 */
void eor_d_aa ( void ) { 
	cpu6502.a_reg ^= rd_6502( cpu6502.pc );
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x45 */
void eor_aa ( void ) { 
	cpu6502.a_reg ^= rd_z_6502(rd_6502( cpu6502.pc ));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x55 */
void eor_aa_x ( void ) { 
	cpu6502.a_reg ^= rd_z_6502((rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff);
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x4D */
void eor_aaaa ( void ) { 
	cpu6502.a_reg ^= rd_6502(rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x5D */
void eor_aaaa_x ( void ) { 
	cpu6502.a_reg ^= rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x59 */
void eor_aaaa_y ( void ) { 
	cpu6502.a_reg ^= rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x41 */
void eor_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	cpu6502.a_reg ^= rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x51 */
void eor_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.a_reg ^= rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* INC *
******/
/* opcode 0xE6 */
void inc_aa ( void ) { 
	cpu6502.h_reg = rd_6502( cpu6502.pc ); 
	cpu6502.pc++;
	cpu6502.t_reg = (rd_z_6502(cpu6502.h_reg) + 1) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}
/* opcode 0xF6 */
void inc_aa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff; 
	cpu6502.pc++;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 1) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}
/* opcode 0xEE */
void inc_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.pc+=2;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 1) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}
/* opcode 0xFE */
void inc_aaaa_x ( void ) { 
	cpu6502.h_reg = (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff);
	cpu6502.pc+=2;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 1) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );
}

/******
* INS *
******/
/* opcode 0xFB */
void ins_aaaa_y ( void ) { 
	cpu6502.h_reg = (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff);
	cpu6502.pc+=2;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 1) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );

	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = cpu6502.t_reg) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}

/* opcode 0xFF */
void ins_aaaa_x ( void ) { 
	cpu6502.h_reg = (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff);
	cpu6502.pc+=2;
	cpu6502.t_reg = (rd_6502(cpu6502.h_reg) + 1) & 0xff; 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502( cpu6502.h_reg, cpu6502.t_reg );

	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = cpu6502.t_reg) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}

/******
* INX *
******/
/* opcode 0xE8 */
void inx ( void ) { 
	cpu6502.x_reg += 1;
	cpu6502.x_reg &= 0xff;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* INY *
******/
/* opcode 0xC8 */
void iny ( void ) { 
	cpu6502.y_reg += 1;
	cpu6502.y_reg &= 0xff;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* JMP *
******/
/* opcode 0x4C */
void jmp_aaaa ( void ) { 
	cpu6502.pc = rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8);
}
/* opcode 0x6C */
void jmp_i_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.pc = rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8);
}

/******
* JSR *
******/
/* opcode 0x20 */
void jsr_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.pc++;
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = (cpu6502.pc >> 8) & 0xff;
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.pc & 0xff;
	cpu6502.pc = cpu6502.h_reg;
}

/******
* LDA *
******/
/* opcode 0xA9 */
void lda_d_aa ( void ) { 
	cpu6502.a_reg = rd_6502( cpu6502.pc );
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xA5 */
void lda_aa ( void ) { 
	cpu6502.a_reg = rd_z_6502(rd_6502( cpu6502.pc ));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xB5 */
void lda_aa_x ( void ) { 
	cpu6502.a_reg = rd_6502((rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff);
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xAD */
void lda_aaaa ( void ) { 
	cpu6502.a_reg = rd_6502(rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xBD */
void lda_aaaa_x ( void ) { 
	cpu6502.a_reg = rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff) );
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xB9 */
void lda_aaaa_y ( void ) { 
	cpu6502.a_reg = rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff) );
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xA1 */
void lda_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	cpu6502.a_reg = rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xB1 */
void lda_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.a_reg = rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* LDX *
******/
/* opcode 0xA2 */
void ldx_d_aa ( void ) { 
	cpu6502.x_reg = rd_6502( cpu6502.pc );
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xA6 */
void ldx_aa ( void ) { 
	cpu6502.x_reg = rd_z_6502(rd_6502( cpu6502.pc ));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xB6 */
void ldx_aa_y ( void ) { 
	cpu6502.x_reg = rd_6502((rd_6502( cpu6502.pc ) + cpu6502.y_reg) & 0xff );
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xAE */
void ldx_aaaa ( void ) { 
	cpu6502.x_reg = rd_6502(rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xBE */
void ldx_aaaa_y ( void ) { 
	cpu6502.x_reg = rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff) );
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* LDY *
******/
/* opcode 0xA0 */
void ldy_d_aa ( void ) { 
	cpu6502.y_reg = rd_6502( cpu6502.pc );
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xA4 */
void ldy_aa ( void ) { 
	cpu6502.y_reg = rd_z_6502(rd_6502( cpu6502.pc ));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xB4 */
void ldy_aa_x ( void ) { 
	cpu6502.y_reg = rd_6502((rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff );
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xAC */
void ldy_aaaa ( void ) { 
	cpu6502.y_reg = rd_6502(rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0xBC */
void ldy_aaaa_x ( void ) { 
	cpu6502.y_reg = rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff) );
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* LAX *
******/
/* opcode 0xBF */
void lax_aaaa_y ( void ) { 
	cpu6502.a_reg = cpu6502.x_reg = rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff) );
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}

/******
* LSR *
******/
/* opcode 0x4A */
void lsr_A ( void ) { 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg >>= 1;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
}
/* opcode 0x46 */
void lsr_aa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.t_reg = rd_z_6502(cpu6502.h_reg);
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x56 */
void lsr_aa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff; 
	cpu6502.pc++;
	cpu6502.t_reg = rd_6502(cpu6502.h_reg);
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x4E */
void lsr_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.pc+=2;
	cpu6502.t_reg = rd_6502(cpu6502.h_reg);
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x5E */
void lsr_aaaa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff);
	cpu6502.pc+=2;
	cpu6502.t_reg = rd_6502(cpu6502.h_reg);
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}

/******
* NOP *
******/
/* opcode 0xEA */
void nop ( void ) { 
}

/* opcode 0x04 */
/* opcode 0x44 */
/* *undoc***** */
void nop_aa ( void ) { 
	cpu6502.pc++;
}

/* opcode 0xc2 */
/* *undoc***** */
void nop_d_aa ( void ) { 
	cpu6502.pc++;
}

/* opcode 0x14 */
/* opcode 0x34 */
/* opcode 0x54 */
/* opcode 0xf4 */
/* *undoc***** */
void nop_aa_x ( void ) { 
	cpu6502.pc++;
}

/* opcode 0x5c */
/* *undoc***** */
void nop_aaaa_x ( void ) { 
	cpu6502.pc+=2;
}

/******
* ORA *
******/
/* opcode 0x09 */
void ora_d_aa ( void ) { 
	cpu6502.a_reg |= rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x05 */
void ora_aa ( void ) { 
	cpu6502.a_reg |= rd_z_6502(rd_6502(cpu6502.pc));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x15 */
void ora_aa_x ( void ) { 
	cpu6502.a_reg |= rd_6502((rd_6502(cpu6502.pc) + cpu6502.x_reg) & 0xff);
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x0D */
void ora_aaaa ( void ) { 
	cpu6502.a_reg |= rd_6502(rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x1D */
void ora_aaaa_x ( void ) { 
	cpu6502.a_reg |= rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x19 */
void ora_aaaa_y ( void ) { 
	cpu6502.a_reg |= rd_6502((rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x01 */
void ora_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	cpu6502.a_reg |= rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x11 */
void ora_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.a_reg |= rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}

/******
* PHA *
******/
/* opcode 0x48 */
void pha ( void ) { 
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.a_reg;
}

/******
* PHP *
******/
/* opcode 0x08 */
void php ( void ) { 
	cpu6502.stack[ (cpu6502.stack_ptr--) & 0xff ] = cpu6502.status;
}

/******
* PLA *
******/
/* opcode 0x68 */
void pla ( void ) { 
	cpu6502.a_reg = cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ];
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}

/******
* PLP *
******/
/* opcode 0x28 */
void plp ( void ) { 
	cpu6502.status = cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ];
}

/******
* ROL *
******/
/* opcode 0x2A */
void rol_A ( void ) { 
	cpu6502.a_reg <<= 1;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.a_reg |= 0x01;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.a_reg > 0xff) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg &= 0xff;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x26 */
void rol_aa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.t_reg = rd_z_6502(cpu6502.h_reg) << 1;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x01;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg > 0xff) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x36 */
void rol_aa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff; 
	cpu6502.pc++;
	cpu6502.t_reg = rd_6502(cpu6502.h_reg) << 1;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x01;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg > 0xff) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x2E */
void rol_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.t_reg = rd_6502(cpu6502.h_reg) << 1;
	cpu6502.pc+=2;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x01;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg > 0xff) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x3E */
void rol_aaaa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff);
	cpu6502.t_reg = rd_6502(cpu6502.h_reg) << 1;
	cpu6502.pc+=2;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x01;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg > 0xff) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg &= 0xff;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}

/******
* ROR *
******/
/* opcode 0x6A */
void ror_A ( void ) { 
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.a_reg |= 0x100;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.a_reg >>= 1;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
}
/* opcode 0x66 */
void ror_aa ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.t_reg = rd_z_6502(cpu6502.h_reg);
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x100;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x76 */
void ror_aa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) + cpu6502.x_reg) & 0xff; 
	cpu6502.pc++;
	cpu6502.t_reg = rd_6502(cpu6502.h_reg);
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x100;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x6E */
void ror_aaaa ( void ) { 
	cpu6502.h_reg = rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8);
	cpu6502.t_reg = rd_6502(cpu6502.h_reg);
	cpu6502.pc+=2;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x100;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}
/* opcode 0x7E */
void ror_aaaa_x ( void ) { 
	cpu6502.h_reg = (rd_6502( cpu6502.pc ) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff);
	cpu6502.t_reg = rd_6502(cpu6502.h_reg);
	cpu6502.pc+=2;
	if ( cpu6502.status & B6502_CARRY_FLG ) cpu6502.t_reg |= 0x100;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x01) ? B6502_CARRY_FLG : 0;
	cpu6502.t_reg >>= 1;
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	wr_6502(cpu6502.h_reg, cpu6502.t_reg);
}

/******
* RTI *
******/
/* opcode 0x40 */
void rti ( void ) { 
	cpu6502.status = cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ];
	cpu6502.pc = cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ];
	cpu6502.pc |= cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ] << 8;
}

/******
* RTS *
******/
/* opcode 0x60 */
void rts ( void ) { 
	cpu6502.pc = cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ];
	cpu6502.pc |= cpu6502.stack[ (++cpu6502.stack_ptr) & 0xff ] << 8;
	cpu6502.pc += 1;
}

/******
* SBC *
******/
/* opcode 0xE9 */
void sbc_d_aa ( void ) { 
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502(cpu6502.pc)) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xE5 */
void sbc_aa ( void ) { 
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_z_6502(rd_6502(cpu6502.pc))) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xF5 */
void sbc_aa_x ( void ) { 
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) + cpu6502.x_reg) & 0xff)) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.pc++;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xED */
void sbc_aaaa ( void ) { 
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502(rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8))) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xFD */
void sbc_aaaa_x ( void ) { 
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff))) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xF9 */
void sbc_aaaa_y ( void ) { 
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff))) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.pc+=2;
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xE1 */
void sbc_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502(rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8))) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}
/* opcode 0xF1 */
void sbc_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	cpu6502.t_reg = cpu6502.a_reg + (~(cpu6502.s_reg = rd_6502((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff))) & 0xff) + 1 + (cpu6502.h_reg=(IF_B6502_CARRY ? 0 : 0xff));
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG | B6502_CARRY_FLG | B6502_OVRFLW_FLG );
	cpu6502.status |= (cpu6502.t_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= ((((cpu6502.a_reg ^ cpu6502.s_reg) & 0x80) && ((cpu6502.a_reg ^ cpu6502.t_reg) & 0x80)) ? B6502_OVRFLW_FLG : 0);
	cpu6502.status |= (cpu6502.t_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.status |= (cpu6502.a_reg >= (cpu6502.s_reg + 
	                  (cpu6502.h_reg ? 1 : 0)) ) ? B6502_CARRY_FLG : 0;

	/* Different for binary than decimal */
	if ( IF_B6502_DECIMAL ) {
		cpu6502.t_reg = (cpu6502.a_reg & 0x0f) + (~(cpu6502.s_reg & 0x0f) & 0xff) + 1 + (cpu6502.h_reg);
		cpu6502.h_reg = (cpu6502.a_reg >> 4) - (cpu6502.s_reg >> 4);
		if ( cpu6502.t_reg & 0x10 ) {
			cpu6502.t_reg -= 6;
			cpu6502.h_reg--;
		}
		if ( cpu6502.h_reg & 0x10 )
			cpu6502.h_reg -= 6;

		cpu6502.a_reg = ((cpu6502.h_reg << 4) | (cpu6502.t_reg & 0x0f)) & 0xff;
	}
	else
		cpu6502.a_reg = cpu6502.t_reg & 0xff;
}

/******
* SEC *
******/
/* opcode 0x38 */
void sec ( void ) { 
	cpu6502.status |= B6502_CARRY_FLG;
}

/******
* SED *
******/
/* opcode 0xF8 */
void sed ( void ) { 
	cpu6502.status |= B6502_DECIMAL_FLG;
}

/******
* SEI *
******/
/* opcode 0x78 */
void sei ( void ) { 
	cpu6502.status |= B6502_INTERPT_FLG;
}

/******
* SHX *
******/
/* opcode 0x9E */
void shx_aaaa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc+1);
	wr_6502( ((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff)), cpu6502.x_reg & cpu6502.h_reg );
	cpu6502.pc+=2;
}

/******
* STA *
******/
/* opcode 0x85 */
void sta_aa ( void ) { 
	wr_6502(rd_6502(cpu6502.pc), cpu6502.a_reg);
	cpu6502.pc++;
}
/* opcode 0x95 */
void sta_aa_x ( void ) { 
	wr_6502(((rd_6502(cpu6502.pc) + cpu6502.x_reg) & 0xff), cpu6502.a_reg);
	cpu6502.pc++;
}
/* opcode 0x8D */
void sta_aaaa ( void ) { 
	wr_6502( (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)), cpu6502.a_reg );
	cpu6502.pc+=2;
}
/* opcode 0x9D */
void sta_aaaa_x ( void ) { 
	wr_6502( ((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.x_reg & 0xff)), cpu6502.a_reg );
	cpu6502.pc+=2;
}
/* opcode 0x99 */
void sta_aaaa_y ( void ) { 
	wr_6502( ((rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)) + (cpu6502.y_reg & 0xff)), cpu6502.a_reg );
	cpu6502.pc+=2;
}
/* opcode 0x81 */
void sta_i_aa_x ( void ) { 
	cpu6502.h_reg = ( rd_6502(cpu6502.pc) + cpu6502.x_reg ) & 0xff;
	cpu6502.pc++;
	wr_6502( (rd_6502(cpu6502.h_reg) | (rd_6502(cpu6502.h_reg+1) << 8)), cpu6502.a_reg );
}
/* opcode 0x91 */
void sta_i_aa_y ( void ) { 
	cpu6502.h_reg = rd_6502(cpu6502.pc);
	cpu6502.pc++;
	wr_6502( ((rd_6502(cpu6502.h_reg) | (rd_6502((cpu6502.h_reg+1) & 0xff) << 8)) + (cpu6502.y_reg & 0xff)), cpu6502.a_reg );
}

/******
* STX *
******/
/* opcode 0x86 */
void stx_aa ( void ) { 
	wr_6502(rd_6502(cpu6502.pc), (cpu6502.x_reg));
	cpu6502.pc++;
}
/* opcode 0x96 */
void stx_aa_y ( void ) { 
	wr_6502(((rd_6502(cpu6502.pc) + cpu6502.y_reg) & 0xff), (cpu6502.x_reg));
	cpu6502.pc++;
}
/* opcode 0x8E */
void stx_aaaa ( void ) { 
	wr_6502( (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)), (cpu6502.x_reg) );
	cpu6502.pc+=2;
}

/******
* STY *
******/
/* opcode 0x84 */
void sty_aa ( void ) { 
	wr_6502(rd_6502(cpu6502.pc), (cpu6502.y_reg));
	cpu6502.pc++;
}
/* opcode 0x94 */
void sty_aa_x ( void ) { 
	wr_6502(((rd_6502(cpu6502.pc) + cpu6502.x_reg) & 0xff), (cpu6502.y_reg));
	cpu6502.pc++;
}
/* opcode 0x8C */
void sty_aaaa ( void ) { 
	wr_6502( (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)), (cpu6502.y_reg) );
	cpu6502.pc+=2;
}

/******
* SAX *
******/
/* opcode 0x8F */
/* *undoc***** */
void sax_aaaa ( void ) { 
	wr_6502( (rd_6502(cpu6502.pc) | (rd_6502(cpu6502.pc+1) << 8)), (cpu6502.a_reg & cpu6502.x_reg) );
	cpu6502.pc+=2;
}

/******
* TAX *
******/
/* opcode 0xAA */
void tax ( void ) { 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.x_reg = cpu6502.a_reg & 0xff;
}

/******
* TAY *
******/
/* opcode 0xA8 */
void tay ( void ) { 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.a_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.a_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.y_reg = cpu6502.a_reg & 0xff;
}

/******
* TSX *
******/
/* opcode 0xBA */
void tsx ( void ) { 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.stack_ptr & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.stack_ptr & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.x_reg = cpu6502.stack_ptr & 0xff;
}

/******
* TXA *
******/
/* opcode 0x8A */
void txa ( void ) { 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.x_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.x_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.a_reg = cpu6502.x_reg;
}

/******
* TXS *
******/
/* opcode 0x9A */
void txs ( void ) { 
	cpu6502.stack_ptr = cpu6502.x_reg & 0xff;
}

/******
* TYA *
******/
/* opcode 0x98 */
void tya ( void ) { 
	cpu6502.status &= ~( B6502_ZERO_FLG | B6502_SIGN_FLG );
	cpu6502.status |= (cpu6502.y_reg & 0x80) ? B6502_SIGN_FLG : 0;
	cpu6502.status |= (cpu6502.y_reg & 0xff) ? 0 : B6502_ZERO_FLG;
	cpu6502.a_reg = cpu6502.y_reg & 0xff;
}

/********
* BADOP *
*********/
void badop ( void ) { 
	//debug_close_default_trace ();
	debug_crash_to_gui(); 
}

/**********
* RUN_ESC *
***********/
void run_esc ( void ) { 
	switch ( rd_6502(cpu6502.pc) ) {
		case ESC_CODE_SIO:
			sio_patch();
			break;
		default:
			debug_crash_to_gui(); 
			break;
	}
	cpu6502.pc++;
}

/*******************************
* Assign ops to function array *
********************************/
void assign_ops ( void ) {

	cpu6502.do_instruct[ 0x00 ] = brk;         cpu6502.op_cycles[ 0x00 ] = 7;
	cpu6502.do_instruct[ 0x01 ] = ora_i_aa_x;  cpu6502.op_cycles[ 0x01 ] = 6;
	cpu6502.do_instruct[ 0x02 ] = badop;       cpu6502.op_cycles[ 0x02 ] = 2;
	cpu6502.do_instruct[ 0x03 ] = badop;       cpu6502.op_cycles[ 0x03 ] = 2;
	cpu6502.do_instruct[ 0x04 ] = nop_aa;      cpu6502.op_cycles[ 0x04 ] = 3;
	cpu6502.do_instruct[ 0x05 ] = ora_aa;      cpu6502.op_cycles[ 0x05 ] = 3;
	cpu6502.do_instruct[ 0x06 ] = asl_aa;      cpu6502.op_cycles[ 0x06 ] = 5;
	cpu6502.do_instruct[ 0x07 ] = aso_aa;      cpu6502.op_cycles[ 0x07 ] = 5;
	cpu6502.do_instruct[ 0x08 ] = php;         cpu6502.op_cycles[ 0x08 ] = 3;
	cpu6502.do_instruct[ 0x09 ] = ora_d_aa;    cpu6502.op_cycles[ 0x09 ] = 2;
	cpu6502.do_instruct[ 0x0a ] = asl_A;       cpu6502.op_cycles[ 0x0a ] = 2;
	cpu6502.do_instruct[ 0x0b ] = badop;       cpu6502.op_cycles[ 0x0b ] = 2;
	cpu6502.do_instruct[ 0x0c ] = badop;       cpu6502.op_cycles[ 0x0c ] = 2;
	cpu6502.do_instruct[ 0x0d ] = ora_aaaa;    cpu6502.op_cycles[ 0x0d ] = 4;
	cpu6502.do_instruct[ 0x0e ] = asl_aaaa;    cpu6502.op_cycles[ 0x0e ] = 6;
	cpu6502.do_instruct[ 0x0f ] = badop;       cpu6502.op_cycles[ 0x0f ] = 2;
	cpu6502.do_instruct[ 0x10 ] = bpl;         cpu6502.op_cycles[ 0x10 ] = 2;
	cpu6502.do_instruct[ 0x11 ] = ora_i_aa_y;  cpu6502.op_cycles[ 0x11 ] = 5;
	cpu6502.do_instruct[ 0x12 ] = badop;       cpu6502.op_cycles[ 0x12 ] = 2;
	cpu6502.do_instruct[ 0x13 ] = badop;       cpu6502.op_cycles[ 0x13 ] = 2;
	cpu6502.do_instruct[ 0x14 ] = nop_aa_x;    cpu6502.op_cycles[ 0x14 ] = 4;
	cpu6502.do_instruct[ 0x15 ] = ora_aa_x;    cpu6502.op_cycles[ 0x15 ] = 4;
	cpu6502.do_instruct[ 0x16 ] = asl_aa_x;    cpu6502.op_cycles[ 0x16 ] = 6;
	cpu6502.do_instruct[ 0x17 ] = badop;       cpu6502.op_cycles[ 0x17 ] = 2;
	cpu6502.do_instruct[ 0x18 ] = clc;         cpu6502.op_cycles[ 0x18 ] = 2;
	cpu6502.do_instruct[ 0x19 ] = ora_aaaa_y;  cpu6502.op_cycles[ 0x19 ] = 4;
	cpu6502.do_instruct[ 0x1a ] = badop;       cpu6502.op_cycles[ 0x1a ] = 2;
	cpu6502.do_instruct[ 0x1b ] = badop;       cpu6502.op_cycles[ 0x1b ] = 2;
	cpu6502.do_instruct[ 0x1c ] = badop;       cpu6502.op_cycles[ 0x1c ] = 2;
	cpu6502.do_instruct[ 0x1d ] = ora_aaaa_x;  cpu6502.op_cycles[ 0x1d ] = 4;
	cpu6502.do_instruct[ 0x1e ] = asl_aaaa_x;  cpu6502.op_cycles[ 0x1e ] = 7;
	cpu6502.do_instruct[ 0x1f ] = aso_aaaa_x;  cpu6502.op_cycles[ 0x1f ] = 7;
	cpu6502.do_instruct[ 0x20 ] = jsr_aaaa;    cpu6502.op_cycles[ 0x20 ] = 6;
	cpu6502.do_instruct[ 0x21 ] = and_i_aa_x;  cpu6502.op_cycles[ 0x21 ] = 6;
	cpu6502.do_instruct[ 0x22 ] = badop;       cpu6502.op_cycles[ 0x22 ] = 2;
	cpu6502.do_instruct[ 0x23 ] = badop;       cpu6502.op_cycles[ 0x23 ] = 2;
	cpu6502.do_instruct[ 0x24 ] = bit_aa;      cpu6502.op_cycles[ 0x24 ] = 3;
	cpu6502.do_instruct[ 0x25 ] = and_aa;      cpu6502.op_cycles[ 0x25 ] = 3;
	cpu6502.do_instruct[ 0x26 ] = rol_aa;      cpu6502.op_cycles[ 0x26 ] = 5;
	cpu6502.do_instruct[ 0x27 ] = badop;       cpu6502.op_cycles[ 0x27 ] = 2;
	cpu6502.do_instruct[ 0x28 ] = plp;         cpu6502.op_cycles[ 0x28 ] = 4;
	cpu6502.do_instruct[ 0x29 ] = and_d_aa;    cpu6502.op_cycles[ 0x29 ] = 2;
	cpu6502.do_instruct[ 0x2a ] = rol_A;       cpu6502.op_cycles[ 0x2a ] = 2;
	cpu6502.do_instruct[ 0x2b ] = badop;       cpu6502.op_cycles[ 0x2b ] = 2;
	cpu6502.do_instruct[ 0x2c ] = bit_aaaa;    cpu6502.op_cycles[ 0x2c ] = 4;
	cpu6502.do_instruct[ 0x2d ] = and_aaaa;    cpu6502.op_cycles[ 0x2d ] = 4;
	cpu6502.do_instruct[ 0x2e ] = rol_aaaa;    cpu6502.op_cycles[ 0x2e ] = 6;
	cpu6502.do_instruct[ 0x2f ] = badop;       cpu6502.op_cycles[ 0x2f ] = 2;
	cpu6502.do_instruct[ 0x30 ] = bmi;         cpu6502.op_cycles[ 0x30 ] = 2;
	cpu6502.do_instruct[ 0x31 ] = and_i_aa_y;  cpu6502.op_cycles[ 0x31 ] = 5;
	cpu6502.do_instruct[ 0x32 ] = badop;       cpu6502.op_cycles[ 0x32 ] = 2;
	cpu6502.do_instruct[ 0x33 ] = badop;       cpu6502.op_cycles[ 0x33 ] = 2;
	cpu6502.do_instruct[ 0x34 ] = nop_aa_x;    cpu6502.op_cycles[ 0x34 ] = 4;
	cpu6502.do_instruct[ 0x35 ] = and_aa_x;    cpu6502.op_cycles[ 0x35 ] = 4;
	cpu6502.do_instruct[ 0x36 ] = rol_aa_x;    cpu6502.op_cycles[ 0x36 ] = 6;
	cpu6502.do_instruct[ 0x37 ] = badop;       cpu6502.op_cycles[ 0x37 ] = 2;
	cpu6502.do_instruct[ 0x38 ] = sec;         cpu6502.op_cycles[ 0x38 ] = 2;
	cpu6502.do_instruct[ 0x39 ] = and_aaaa_y;  cpu6502.op_cycles[ 0x39 ] = 4;
	cpu6502.do_instruct[ 0x3a ] = badop;       cpu6502.op_cycles[ 0x3a ] = 2;
	cpu6502.do_instruct[ 0x3b ] = badop;       cpu6502.op_cycles[ 0x3b ] = 2;
	cpu6502.do_instruct[ 0x3c ] = badop;       cpu6502.op_cycles[ 0x3c ] = 2;
	cpu6502.do_instruct[ 0x3d ] = and_aaaa_x;  cpu6502.op_cycles[ 0x3d ] = 4;
	cpu6502.do_instruct[ 0x3e ] = rol_aaaa_x;  cpu6502.op_cycles[ 0x3e ] = 7;
	cpu6502.do_instruct[ 0x3f ] = badop;       cpu6502.op_cycles[ 0x3f ] = 2;
	cpu6502.do_instruct[ 0x40 ] = rti;         cpu6502.op_cycles[ 0x40 ] = 6;
	cpu6502.do_instruct[ 0x41 ] = eor_i_aa_x;  cpu6502.op_cycles[ 0x41 ] = 6;
	cpu6502.do_instruct[ 0x42 ] = badop;       cpu6502.op_cycles[ 0x42 ] = 2;
	cpu6502.do_instruct[ 0x43 ] = badop;       cpu6502.op_cycles[ 0x43 ] = 2;
	cpu6502.do_instruct[ 0x44 ] = nop_aa;      cpu6502.op_cycles[ 0x44 ] = 3;
	cpu6502.do_instruct[ 0x45 ] = eor_aa;      cpu6502.op_cycles[ 0x45 ] = 3;
	cpu6502.do_instruct[ 0x46 ] = lsr_aa;      cpu6502.op_cycles[ 0x46 ] = 5;
	cpu6502.do_instruct[ 0x47 ] = badop;       cpu6502.op_cycles[ 0x47 ] = 2;
	cpu6502.do_instruct[ 0x48 ] = pha;         cpu6502.op_cycles[ 0x48 ] = 3;
	cpu6502.do_instruct[ 0x49 ] = eor_d_aa;    cpu6502.op_cycles[ 0x49 ] = 2;
	cpu6502.do_instruct[ 0x4a ] = lsr_A;       cpu6502.op_cycles[ 0x4a ] = 2;
	cpu6502.do_instruct[ 0x4b ] = badop;       cpu6502.op_cycles[ 0x4b ] = 2;
	cpu6502.do_instruct[ 0x4c ] = jmp_aaaa;    cpu6502.op_cycles[ 0x4c ] = 3;
	cpu6502.do_instruct[ 0x4d ] = eor_aaaa;    cpu6502.op_cycles[ 0x4d ] = 4;
	cpu6502.do_instruct[ 0x4e ] = lsr_aaaa;    cpu6502.op_cycles[ 0x4e ] = 6;
	cpu6502.do_instruct[ 0x4f ] = badop;       cpu6502.op_cycles[ 0x4f ] = 2;
	cpu6502.do_instruct[ 0x50 ] = bvc;         cpu6502.op_cycles[ 0x50 ] = 2;
	cpu6502.do_instruct[ 0x51 ] = eor_i_aa_y;  cpu6502.op_cycles[ 0x51 ] = 6;
	cpu6502.do_instruct[ 0x52 ] = badop;       cpu6502.op_cycles[ 0x52 ] = 2;
	cpu6502.do_instruct[ 0x53 ] = badop;       cpu6502.op_cycles[ 0x53 ] = 2;
	cpu6502.do_instruct[ 0x54 ] = nop_aa_x;    cpu6502.op_cycles[ 0x54 ] = 4;
	cpu6502.do_instruct[ 0x55 ] = eor_aa_x;    cpu6502.op_cycles[ 0x55 ] = 4;
	cpu6502.do_instruct[ 0x56 ] = lsr_aa_x;    cpu6502.op_cycles[ 0x56 ] = 6;
	cpu6502.do_instruct[ 0x57 ] = badop;       cpu6502.op_cycles[ 0x57 ] = 2;
	cpu6502.do_instruct[ 0x58 ] = cli;         cpu6502.op_cycles[ 0x58 ] = 2;
	cpu6502.do_instruct[ 0x59 ] = eor_aaaa_y;  cpu6502.op_cycles[ 0x59 ] = 4;
	cpu6502.do_instruct[ 0x5a ] = badop;       cpu6502.op_cycles[ 0x5a ] = 2;
	cpu6502.do_instruct[ 0x5b ] = badop;       cpu6502.op_cycles[ 0x5b ] = 2;
	cpu6502.do_instruct[ 0x5c ] = nop_aaaa_x;  cpu6502.op_cycles[ 0x5c ] = 4;
	cpu6502.do_instruct[ 0x5d ] = eor_aaaa_x;  cpu6502.op_cycles[ 0x5d ] = 4;
	cpu6502.do_instruct[ 0x5e ] = lsr_aaaa_x;  cpu6502.op_cycles[ 0x5e ] = 7;
	cpu6502.do_instruct[ 0x5f ] = badop;       cpu6502.op_cycles[ 0x5f ] = 2;
	cpu6502.do_instruct[ 0x60 ] = rts;         cpu6502.op_cycles[ 0x60 ] = 6;
	cpu6502.do_instruct[ 0x61 ] = adc_i_aa_x;  cpu6502.op_cycles[ 0x61 ] = 6;
	cpu6502.do_instruct[ 0x62 ] = badop;       cpu6502.op_cycles[ 0x62 ] = 2;
	cpu6502.do_instruct[ 0x63 ] = badop;       cpu6502.op_cycles[ 0x63 ] = 2;
	cpu6502.do_instruct[ 0x64 ] = badop;       cpu6502.op_cycles[ 0x64 ] = 2;
	cpu6502.do_instruct[ 0x65 ] = adc_aa;      cpu6502.op_cycles[ 0x65 ] = 3;
	cpu6502.do_instruct[ 0x66 ] = ror_aa;      cpu6502.op_cycles[ 0x66 ] = 5;
	cpu6502.do_instruct[ 0x67 ] = badop;       cpu6502.op_cycles[ 0x67 ] = 2;
	cpu6502.do_instruct[ 0x68 ] = pla;         cpu6502.op_cycles[ 0x68 ] = 4;
	cpu6502.do_instruct[ 0x69 ] = adc_d_aa;    cpu6502.op_cycles[ 0x69 ] = 2;
	cpu6502.do_instruct[ 0x6a ] = ror_A;       cpu6502.op_cycles[ 0x6a ] = 2;
	cpu6502.do_instruct[ 0x6b ] = badop;       cpu6502.op_cycles[ 0x6b ] = 2;
	cpu6502.do_instruct[ 0x6c ] = jmp_i_aaaa;  cpu6502.op_cycles[ 0x6c ] = 5;
	cpu6502.do_instruct[ 0x6d ] = adc_aaaa;    cpu6502.op_cycles[ 0x6d ] = 4;
	cpu6502.do_instruct[ 0x6e ] = ror_aaaa;    cpu6502.op_cycles[ 0x6e ] = 6;
	cpu6502.do_instruct[ 0x6f ] = badop;       cpu6502.op_cycles[ 0x6f ] = 2;
	cpu6502.do_instruct[ 0x70 ] = bvs;         cpu6502.op_cycles[ 0x70 ] = 2;
	cpu6502.do_instruct[ 0x71 ] = adc_i_aa_y;  cpu6502.op_cycles[ 0x71 ] = 5;
	cpu6502.do_instruct[ 0x72 ] = badop;       cpu6502.op_cycles[ 0x72 ] = 2;
	cpu6502.do_instruct[ 0x73 ] = badop;       cpu6502.op_cycles[ 0x73 ] = 2;
	cpu6502.do_instruct[ 0x74 ] = badop;       cpu6502.op_cycles[ 0x74 ] = 2;
	cpu6502.do_instruct[ 0x75 ] = adc_aa_x;    cpu6502.op_cycles[ 0x75 ] = 4;
	cpu6502.do_instruct[ 0x76 ] = ror_aa_x;    cpu6502.op_cycles[ 0x76 ] = 6;
	cpu6502.do_instruct[ 0x77 ] = badop;       cpu6502.op_cycles[ 0x77 ] = 2;
	cpu6502.do_instruct[ 0x78 ] = sei;         cpu6502.op_cycles[ 0x78 ] = 2;
	cpu6502.do_instruct[ 0x79 ] = adc_aaaa_y;  cpu6502.op_cycles[ 0x79 ] = 4;
	cpu6502.do_instruct[ 0x7a ] = badop;       cpu6502.op_cycles[ 0x7a ] = 2;
	cpu6502.do_instruct[ 0x7b ] = badop;       cpu6502.op_cycles[ 0x7b ] = 2;
	cpu6502.do_instruct[ 0x7c ] = badop;       cpu6502.op_cycles[ 0x7c ] = 2;
	cpu6502.do_instruct[ 0x7d ] = adc_aaaa_x;  cpu6502.op_cycles[ 0x7d ] = 4;
	cpu6502.do_instruct[ 0x7e ] = ror_aaaa_x;  cpu6502.op_cycles[ 0x7e ] = 7;
	cpu6502.do_instruct[ 0x7f ] = badop;       cpu6502.op_cycles[ 0x7f ] = 2;
	cpu6502.do_instruct[ 0x80 ] = badop;       cpu6502.op_cycles[ 0x80 ] = 2;
	cpu6502.do_instruct[ 0x81 ] = sta_i_aa_x;  cpu6502.op_cycles[ 0x81 ] = 6;
	cpu6502.do_instruct[ 0x82 ] = badop;       cpu6502.op_cycles[ 0x82 ] = 2;
	cpu6502.do_instruct[ 0x83 ] = badop;       cpu6502.op_cycles[ 0x83 ] = 2;
	cpu6502.do_instruct[ 0x84 ] = sty_aa;      cpu6502.op_cycles[ 0x84 ] = 3;
	cpu6502.do_instruct[ 0x85 ] = sta_aa;      cpu6502.op_cycles[ 0x85 ] = 3;
	cpu6502.do_instruct[ 0x86 ] = stx_aa;      cpu6502.op_cycles[ 0x86 ] = 3;
	cpu6502.do_instruct[ 0x87 ] = badop;       cpu6502.op_cycles[ 0x87 ] = 2;
	cpu6502.do_instruct[ 0x88 ] = dey;         cpu6502.op_cycles[ 0x88 ] = 2;
	cpu6502.do_instruct[ 0x89 ] = badop;       cpu6502.op_cycles[ 0x89 ] = 2;
	cpu6502.do_instruct[ 0x8a ] = txa;         cpu6502.op_cycles[ 0x8a ] = 2;
	cpu6502.do_instruct[ 0x8b ] = badop;       cpu6502.op_cycles[ 0x8b ] = 2;
	cpu6502.do_instruct[ 0x8c ] = sty_aaaa;    cpu6502.op_cycles[ 0x8c ] = 4;
	cpu6502.do_instruct[ 0x8d ] = sta_aaaa;    cpu6502.op_cycles[ 0x8d ] = 4;
	cpu6502.do_instruct[ 0x8e ] = stx_aaaa;    cpu6502.op_cycles[ 0x8e ] = 4;
	cpu6502.do_instruct[ 0x8f ] = sax_aaaa;    cpu6502.op_cycles[ 0x8f ] = 4;
	cpu6502.do_instruct[ 0x90 ] = bcc;         cpu6502.op_cycles[ 0x90 ] = 2;
	cpu6502.do_instruct[ 0x91 ] = sta_i_aa_y;  cpu6502.op_cycles[ 0x91 ] = 6;
	cpu6502.do_instruct[ 0x92 ] = badop;       cpu6502.op_cycles[ 0x92 ] = 2;
	cpu6502.do_instruct[ 0x93 ] = badop;       cpu6502.op_cycles[ 0x93 ] = 2;
	cpu6502.do_instruct[ 0x94 ] = sty_aa_x;    cpu6502.op_cycles[ 0x94 ] = 4;
	cpu6502.do_instruct[ 0x95 ] = sta_aa_x;    cpu6502.op_cycles[ 0x95 ] = 4;
	cpu6502.do_instruct[ 0x96 ] = stx_aa_y;    cpu6502.op_cycles[ 0x96 ] = 4;
	cpu6502.do_instruct[ 0x97 ] = badop;       cpu6502.op_cycles[ 0x97 ] = 2;
	cpu6502.do_instruct[ 0x98 ] = tya;         cpu6502.op_cycles[ 0x98 ] = 2;
	cpu6502.do_instruct[ 0x99 ] = sta_aaaa_y;  cpu6502.op_cycles[ 0x99 ] = 5;
	cpu6502.do_instruct[ 0x9a ] = txs;         cpu6502.op_cycles[ 0x9a ] = 2;
	cpu6502.do_instruct[ 0x9b ] = badop;       cpu6502.op_cycles[ 0x9b ] = 2;
	cpu6502.do_instruct[ 0x9c ] = badop;       cpu6502.op_cycles[ 0x9c ] = 2;
	cpu6502.do_instruct[ 0x9d ] = sta_aaaa_x;  cpu6502.op_cycles[ 0x9d ] = 5;
	cpu6502.do_instruct[ 0x9e ] = shx_aaaa_y;  cpu6502.op_cycles[ 0x9e ] = 7;
	cpu6502.do_instruct[ 0x9f ] = badop;       cpu6502.op_cycles[ 0x9f ] = 2;
	cpu6502.do_instruct[ 0xa0 ] = ldy_d_aa;    cpu6502.op_cycles[ 0xa0 ] = 2;
	cpu6502.do_instruct[ 0xa1 ] = lda_i_aa_x;  cpu6502.op_cycles[ 0xa1 ] = 6;
	cpu6502.do_instruct[ 0xa2 ] = ldx_d_aa;    cpu6502.op_cycles[ 0xa2 ] = 2;
	cpu6502.do_instruct[ 0xa3 ] = badop;       cpu6502.op_cycles[ 0xa3 ] = 2;
	cpu6502.do_instruct[ 0xa4 ] = ldy_aa;      cpu6502.op_cycles[ 0xa4 ] = 3;
	cpu6502.do_instruct[ 0xa5 ] = lda_aa;      cpu6502.op_cycles[ 0xa5 ] = 3;
	cpu6502.do_instruct[ 0xa6 ] = ldx_aa;      cpu6502.op_cycles[ 0xa6 ] = 3;
	cpu6502.do_instruct[ 0xa7 ] = badop;       cpu6502.op_cycles[ 0xa7 ] = 2;
	cpu6502.do_instruct[ 0xa8 ] = tay;         cpu6502.op_cycles[ 0xa8 ] = 2;
	cpu6502.do_instruct[ 0xa9 ] = lda_d_aa;    cpu6502.op_cycles[ 0xa9 ] = 2;
	cpu6502.do_instruct[ 0xaa ] = tax;         cpu6502.op_cycles[ 0xaa ] = 2;
	cpu6502.do_instruct[ 0xab ] = badop;       cpu6502.op_cycles[ 0xab ] = 2;
	cpu6502.do_instruct[ 0xac ] = ldy_aaaa;    cpu6502.op_cycles[ 0xac ] = 4;
	cpu6502.do_instruct[ 0xad ] = lda_aaaa;    cpu6502.op_cycles[ 0xad ] = 4;
	cpu6502.do_instruct[ 0xae ] = ldx_aaaa;    cpu6502.op_cycles[ 0xae ] = 4;
	cpu6502.do_instruct[ 0xaf ] = badop;       cpu6502.op_cycles[ 0xaf ] = 2;
	cpu6502.do_instruct[ 0xb0 ] = bcs;         cpu6502.op_cycles[ 0xb0 ] = 2;
	cpu6502.do_instruct[ 0xb1 ] = lda_i_aa_y;  cpu6502.op_cycles[ 0xb1 ] = 5;
	cpu6502.do_instruct[ 0xb2 ] = badop;       cpu6502.op_cycles[ 0xb2 ] = 2;
	cpu6502.do_instruct[ 0xb3 ] = badop;       cpu6502.op_cycles[ 0xb3 ] = 2;
	cpu6502.do_instruct[ 0xb4 ] = ldy_aa_x;    cpu6502.op_cycles[ 0xb4 ] = 4;
	cpu6502.do_instruct[ 0xb5 ] = lda_aa_x;    cpu6502.op_cycles[ 0xb5 ] = 4;
	cpu6502.do_instruct[ 0xb6 ] = ldx_aa_y;    cpu6502.op_cycles[ 0xb6 ] = 4;
	cpu6502.do_instruct[ 0xb7 ] = badop;       cpu6502.op_cycles[ 0xb7 ] = 2;
	cpu6502.do_instruct[ 0xb8 ] = clv;         cpu6502.op_cycles[ 0xb8 ] = 2;
	cpu6502.do_instruct[ 0xb9 ] = lda_aaaa_y;  cpu6502.op_cycles[ 0xb9 ] = 4;
	cpu6502.do_instruct[ 0xba ] = tsx;         cpu6502.op_cycles[ 0xba ] = 2;
	cpu6502.do_instruct[ 0xbb ] = badop;       cpu6502.op_cycles[ 0xbb ] = 2;
	cpu6502.do_instruct[ 0xbc ] = ldy_aaaa_x;  cpu6502.op_cycles[ 0xbc ] = 4;
	cpu6502.do_instruct[ 0xbd ] = lda_aaaa_x;  cpu6502.op_cycles[ 0xbd ] = 4;
	cpu6502.do_instruct[ 0xbe ] = ldx_aaaa_y;  cpu6502.op_cycles[ 0xbe ] = 4;
	cpu6502.do_instruct[ 0xbf ] = lax_aaaa_y;  cpu6502.op_cycles[ 0xbf ] = 4;
	cpu6502.do_instruct[ 0xc0 ] = cpy_d_aa;    cpu6502.op_cycles[ 0xc0 ] = 2;
	cpu6502.do_instruct[ 0xc1 ] = cmp_i_aa_x;  cpu6502.op_cycles[ 0xc1 ] = 6;
	cpu6502.do_instruct[ 0xc2 ] = nop_d_aa;    cpu6502.op_cycles[ 0xc2 ] = 2;
	cpu6502.do_instruct[ 0xc3 ] = badop;       cpu6502.op_cycles[ 0xc3 ] = 2;
	cpu6502.do_instruct[ 0xc4 ] = cpy_aa;      cpu6502.op_cycles[ 0xc4 ] = 3;
	cpu6502.do_instruct[ 0xc5 ] = cmp_aa;      cpu6502.op_cycles[ 0xc5 ] = 3;
	cpu6502.do_instruct[ 0xc6 ] = dec_aa;      cpu6502.op_cycles[ 0xc6 ] = 5;
	cpu6502.do_instruct[ 0xc7 ] = badop;       cpu6502.op_cycles[ 0xc7 ] = 2;
	cpu6502.do_instruct[ 0xc8 ] = iny;         cpu6502.op_cycles[ 0xc8 ] = 2;
	cpu6502.do_instruct[ 0xc9 ] = cmp_d_aa;    cpu6502.op_cycles[ 0xc9 ] = 2;
	cpu6502.do_instruct[ 0xca ] = dex;         cpu6502.op_cycles[ 0xca ] = 2;
	cpu6502.do_instruct[ 0xcb ] = badop;       cpu6502.op_cycles[ 0xcb ] = 2;
	cpu6502.do_instruct[ 0xcc ] = cpy_aaaa;    cpu6502.op_cycles[ 0xcc ] = 4;
	cpu6502.do_instruct[ 0xcd ] = cmp_aaaa;    cpu6502.op_cycles[ 0xcd ] = 4;
	cpu6502.do_instruct[ 0xce ] = dec_aaaa;    cpu6502.op_cycles[ 0xce ] = 6;
	cpu6502.do_instruct[ 0xcf ] = badop;       cpu6502.op_cycles[ 0xcf ] = 2;
	cpu6502.do_instruct[ 0xd0 ] = bne;         cpu6502.op_cycles[ 0xd0 ] = 2;
	cpu6502.do_instruct[ 0xd1 ] = cmp_i_aa_y;  cpu6502.op_cycles[ 0xd1 ] = 5;
	cpu6502.do_instruct[ 0xd2 ] = badop;       cpu6502.op_cycles[ 0xd2 ] = 2;
	cpu6502.do_instruct[ 0xd3 ] = badop;       cpu6502.op_cycles[ 0xd3 ] = 2;
	cpu6502.do_instruct[ 0xd4 ] = badop;       cpu6502.op_cycles[ 0xd4 ] = 2;
	cpu6502.do_instruct[ 0xd5 ] = cmp_aa_x;    cpu6502.op_cycles[ 0xd5 ] = 4;
	cpu6502.do_instruct[ 0xd6 ] = dec_aa_x;    cpu6502.op_cycles[ 0xd6 ] = 6;
	cpu6502.do_instruct[ 0xd7 ] = badop;       cpu6502.op_cycles[ 0xd7 ] = 2;
	cpu6502.do_instruct[ 0xd8 ] = cld;         cpu6502.op_cycles[ 0xd8 ] = 2;
	cpu6502.do_instruct[ 0xd9 ] = cmp_aaaa_y;  cpu6502.op_cycles[ 0xd9 ] = 4;
	cpu6502.do_instruct[ 0xda ] = badop;       cpu6502.op_cycles[ 0xda ] = 2;
	cpu6502.do_instruct[ 0xdb ] = badop;       cpu6502.op_cycles[ 0xdb ] = 2;
	cpu6502.do_instruct[ 0xdc ] = badop;       cpu6502.op_cycles[ 0xdc ] = 2;
	cpu6502.do_instruct[ 0xdd ] = cmp_aaaa_x;  cpu6502.op_cycles[ 0xdd ] = 4;
	cpu6502.do_instruct[ 0xde ] = dec_aaaa_x;  cpu6502.op_cycles[ 0xde ] = 7;
	cpu6502.do_instruct[ 0xdf ] = badop;       cpu6502.op_cycles[ 0xdf ] = 2;
	cpu6502.do_instruct[ 0xe0 ] = cpx_d_aa;    cpu6502.op_cycles[ 0xe0 ] = 2;
	cpu6502.do_instruct[ 0xe1 ] = sbc_i_aa_x;  cpu6502.op_cycles[ 0xe1 ] = 6;
	cpu6502.do_instruct[ 0xe2 ] = badop;       cpu6502.op_cycles[ 0xe2 ] = 2;
	cpu6502.do_instruct[ 0xe3 ] = badop;       cpu6502.op_cycles[ 0xe3 ] = 2;
	cpu6502.do_instruct[ 0xe4 ] = cpx_aa;      cpu6502.op_cycles[ 0xe4 ] = 3;
	cpu6502.do_instruct[ 0xe5 ] = sbc_aa;      cpu6502.op_cycles[ 0xe5 ] = 3;
	cpu6502.do_instruct[ 0xe6 ] = inc_aa;      cpu6502.op_cycles[ 0xe6 ] = 5;
	cpu6502.do_instruct[ 0xe7 ] = badop;       cpu6502.op_cycles[ 0xe7 ] = 2;
	cpu6502.do_instruct[ 0xe8 ] = inx;         cpu6502.op_cycles[ 0xe8 ] = 2;
	cpu6502.do_instruct[ 0xe9 ] = sbc_d_aa;    cpu6502.op_cycles[ 0xe9 ] = 2;
	cpu6502.do_instruct[ 0xea ] = nop;         cpu6502.op_cycles[ 0xea ] = 2;
	cpu6502.do_instruct[ 0xeb ] = badop;       cpu6502.op_cycles[ 0xeb ] = 2;
	cpu6502.do_instruct[ 0xec ] = cpx_aaaa;    cpu6502.op_cycles[ 0xec ] = 4;
	cpu6502.do_instruct[ 0xed ] = sbc_aaaa;    cpu6502.op_cycles[ 0xed ] = 4;
	cpu6502.do_instruct[ 0xee ] = inc_aaaa;    cpu6502.op_cycles[ 0xee ] = 6;
	cpu6502.do_instruct[ 0xef ] = badop;       cpu6502.op_cycles[ 0xef ] = 2;
	cpu6502.do_instruct[ 0xf0 ] = beq;         cpu6502.op_cycles[ 0xf0 ] = 2;
	cpu6502.do_instruct[ 0xf1 ] = sbc_i_aa_y;  cpu6502.op_cycles[ 0xf1 ] = 5;
	cpu6502.do_instruct[ 0xf2 ] = run_esc;     cpu6502.op_cycles[ 0xf2 ] = 2;
	cpu6502.do_instruct[ 0xf3 ] = badop;       cpu6502.op_cycles[ 0xf3 ] = 2;
	cpu6502.do_instruct[ 0xf4 ] = nop_aa_x;    cpu6502.op_cycles[ 0xf4 ] = 4;
	cpu6502.do_instruct[ 0xf5 ] = sbc_aa_x;    cpu6502.op_cycles[ 0xf5 ] = 4;
	cpu6502.do_instruct[ 0xf6 ] = inc_aa_x;    cpu6502.op_cycles[ 0xf6 ] = 6;
	cpu6502.do_instruct[ 0xf7 ] = badop;       cpu6502.op_cycles[ 0xf7 ] = 2;
	cpu6502.do_instruct[ 0xf8 ] = sed;         cpu6502.op_cycles[ 0xf8 ] = 2;
	cpu6502.do_instruct[ 0xf9 ] = sbc_aaaa_y;  cpu6502.op_cycles[ 0xf9 ] = 4;
	cpu6502.do_instruct[ 0xfa ] = badop;       cpu6502.op_cycles[ 0xfa ] = 2;
	cpu6502.do_instruct[ 0xfb ] = ins_aaaa_y;  cpu6502.op_cycles[ 0xfb ] = 7;
	cpu6502.do_instruct[ 0xfc ] = badop;       cpu6502.op_cycles[ 0xfc ] = 2;
	cpu6502.do_instruct[ 0xfd ] = sbc_aaaa_x;  cpu6502.op_cycles[ 0xfd ] = 4;
	cpu6502.do_instruct[ 0xfe ] = inc_aaaa_x;  cpu6502.op_cycles[ 0xfe ] = 7;
	cpu6502.do_instruct[ 0xff ] = ins_aaaa_x;  cpu6502.op_cycles[ 0xff ] = 7;

	strcpy ( cpu6502.op_name[ 0x00 ], "brk" );        
	strcpy ( cpu6502.op_name[ 0x01 ], "ora_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0x02 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x03 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x04 ], "nop_aa*" );      
	strcpy ( cpu6502.op_name[ 0x05 ], "ora_aa" );     
	strcpy ( cpu6502.op_name[ 0x06 ], "asl_aa" );     
	strcpy ( cpu6502.op_name[ 0x07 ], "aso_aa*" );     
	strcpy ( cpu6502.op_name[ 0x08 ], "php" );        
	strcpy ( cpu6502.op_name[ 0x09 ], "ora_d_aa" );   
	strcpy ( cpu6502.op_name[ 0x0a ], "asl_A" );      
	strcpy ( cpu6502.op_name[ 0x0b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x0c ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x0d ], "ora_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x0e ], "asl_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x0f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x10 ], "bpl" );        
	strcpy ( cpu6502.op_name[ 0x11 ], "ora_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0x12 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x13 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x14 ], "nop_aa_x*" );      
	strcpy ( cpu6502.op_name[ 0x15 ], "ora_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x16 ], "asl_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x17 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x18 ], "clc" );        
	strcpy ( cpu6502.op_name[ 0x19 ], "ora_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0x1a ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x1b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x1c ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x1d ], "ora_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x1e ], "asl_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x1f ], "aso_aaaa_x*" );      
	strcpy ( cpu6502.op_name[ 0x20 ], "jsr_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x21 ], "and_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0x22 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x23 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x24 ], "bit_aa" );     
	strcpy ( cpu6502.op_name[ 0x25 ], "and_aa" );     
	strcpy ( cpu6502.op_name[ 0x26 ], "rol_aa" );     
	strcpy ( cpu6502.op_name[ 0x27 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x28 ], "plp" );        
	strcpy ( cpu6502.op_name[ 0x29 ], "and_d_aa" );   
	strcpy ( cpu6502.op_name[ 0x2a ], "rol_A" );      
	strcpy ( cpu6502.op_name[ 0x2b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x2c ], "bit_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x2d ], "and_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x2e ], "rol_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x2f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x30 ], "bmi" );        
	strcpy ( cpu6502.op_name[ 0x31 ], "and_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0x32 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x33 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x34 ], "nop_aa_x*" );      
	strcpy ( cpu6502.op_name[ 0x35 ], "and_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x36 ], "rol_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x37 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x38 ], "sec" );        
	strcpy ( cpu6502.op_name[ 0x39 ], "and_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0x3a ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x3b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x3c ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x3d ], "and_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x3e ], "rol_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x3f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x40 ], "rti" );        
	strcpy ( cpu6502.op_name[ 0x41 ], "eor_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0x42 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x43 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x44 ], "nop_aa*" );      
	strcpy ( cpu6502.op_name[ 0x45 ], "eor_aa" );     
	strcpy ( cpu6502.op_name[ 0x46 ], "lsr_aa" );     
	strcpy ( cpu6502.op_name[ 0x47 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x48 ], "pha" );        
	strcpy ( cpu6502.op_name[ 0x49 ], "eor_d_aa" );   
	strcpy ( cpu6502.op_name[ 0x4a ], "lsr_A" );      
	strcpy ( cpu6502.op_name[ 0x4b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x4c ], "jmp_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x4d ], "eor_aaaa" );  
	strcpy ( cpu6502.op_name[ 0x4e ], "lsr_aaaa" );  
	strcpy ( cpu6502.op_name[ 0x4f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x50 ], "bvc" );        
	strcpy ( cpu6502.op_name[ 0x51 ], "eor_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0x52 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x53 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x54 ], "nop_aa_x*" );      
	strcpy ( cpu6502.op_name[ 0x55 ], "eor_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x56 ], "lsr_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x57 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x58 ], "cli" );        
	strcpy ( cpu6502.op_name[ 0x59 ], "eor_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0x5a ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x5b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x5c ], "nop_aaaa_x*" );      
	strcpy ( cpu6502.op_name[ 0x5d ], "eor_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x5e ], "lsr_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x5f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x60 ], "rts" );        
	strcpy ( cpu6502.op_name[ 0x61 ], "adc_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0x62 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x63 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x64 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x65 ], "adc_aa" );     
	strcpy ( cpu6502.op_name[ 0x66 ], "ror_aa" );     
	strcpy ( cpu6502.op_name[ 0x67 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x68 ], "pla" );        
	strcpy ( cpu6502.op_name[ 0x69 ], "adc_d_aa" );   
	strcpy ( cpu6502.op_name[ 0x6a ], "ror_A" );      
	strcpy ( cpu6502.op_name[ 0x6b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x6c ], "jmp_i_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x6d ], "adc_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x6e ], "ror_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x6f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x70 ], "bvs" );        
	strcpy ( cpu6502.op_name[ 0x71 ], "adc_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0x72 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x73 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x74 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x75 ], "adc_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x76 ], "ror_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x77 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x78 ], "sei" );        
	strcpy ( cpu6502.op_name[ 0x79 ], "adc_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0x7a ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x7b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x7c ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x7d ], "adc_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x7e ], "ror_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x7f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x80 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x81 ], "sta_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0x82 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x83 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x84 ], "sty_aa" );     
	strcpy ( cpu6502.op_name[ 0x85 ], "sta_aa" );     
	strcpy ( cpu6502.op_name[ 0x86 ], "stx_aa" );     
	strcpy ( cpu6502.op_name[ 0x87 ], "badop" );     
	strcpy ( cpu6502.op_name[ 0x88 ], "dey" );        
	strcpy ( cpu6502.op_name[ 0x89 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x8a ], "txa" );        
	strcpy ( cpu6502.op_name[ 0x8b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x8c ], "sty_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x8d ], "sta_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x8e ], "stx_aaaa" );   
	strcpy ( cpu6502.op_name[ 0x8f ], "sax_aaaa*" );      
	strcpy ( cpu6502.op_name[ 0x90 ], "bcc" );        
	strcpy ( cpu6502.op_name[ 0x91 ], "sta_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0x92 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x93 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x94 ], "sty_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x95 ], "sta_aa_x" );   
	strcpy ( cpu6502.op_name[ 0x96 ], "stx_aa_y" );   
	strcpy ( cpu6502.op_name[ 0x97 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x98 ], "tya" );        
	strcpy ( cpu6502.op_name[ 0x99 ], "sta_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0x9a ], "txs" );        
	strcpy ( cpu6502.op_name[ 0x9b ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x9c ], "badop" );      
	strcpy ( cpu6502.op_name[ 0x9d ], "sta_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0x9e ], "shx_aaaa_y*" );      
	strcpy ( cpu6502.op_name[ 0x9f ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xa0 ], "ldy_d_aa" );    
	strcpy ( cpu6502.op_name[ 0xa1 ], "lda_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0xa2 ], "ldx_d_aa" );   
	strcpy ( cpu6502.op_name[ 0xa3 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xa4 ], "ldy_aa" );     
	strcpy ( cpu6502.op_name[ 0xa5 ], "lda_aa" );     
	strcpy ( cpu6502.op_name[ 0xa6 ], "ldx_aa" );     
	strcpy ( cpu6502.op_name[ 0xa7 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xa8 ], "tay" );        
	strcpy ( cpu6502.op_name[ 0xa9 ], "lda_d_aa" );   
	strcpy ( cpu6502.op_name[ 0xaa ], "tax" );        
	strcpy ( cpu6502.op_name[ 0xab ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xac ], "ldy_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xad ], "lda_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xae ], "ldx_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xaf ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xb0 ], "bcs" );        
	strcpy ( cpu6502.op_name[ 0xb1 ], "lda_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0xb2 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xb3 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xb4 ], "ldy_aa_x" );   
	strcpy ( cpu6502.op_name[ 0xb5 ], "lda_aa_x" );   
	strcpy ( cpu6502.op_name[ 0xb6 ], "ldx_aa_y" );   
	strcpy ( cpu6502.op_name[ 0xb7 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xb8 ], "clv" );        
	strcpy ( cpu6502.op_name[ 0xb9 ], "lda_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0xba ], "tsx" );        
	strcpy ( cpu6502.op_name[ 0xbb ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xbc ], "ldy_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0xbd ], "lda_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0xbe ], "ldx_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0xbf ], "lax_aaaa_y*" );      
	strcpy ( cpu6502.op_name[ 0xc0 ], "cpy_d_aa" );   
	strcpy ( cpu6502.op_name[ 0xc1 ], "cmp_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0xc2 ], "nop_d_aa*" );      
	strcpy ( cpu6502.op_name[ 0xc3 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xc4 ], "cpy_aa" );     
	strcpy ( cpu6502.op_name[ 0xc5 ], "cmp_aa" );     
	strcpy ( cpu6502.op_name[ 0xc6 ], "dec_aa" );     
	strcpy ( cpu6502.op_name[ 0xc7 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xc8 ], "iny" );        
	strcpy ( cpu6502.op_name[ 0xc9 ], "cmp_d_aa" );   
	strcpy ( cpu6502.op_name[ 0xca ], "dex" );        
	strcpy ( cpu6502.op_name[ 0xcb ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xcc ], "cpy_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xcd ], "cmp_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xce ], "dec_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xcf ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xd0 ], "bne" );        
	strcpy ( cpu6502.op_name[ 0xd1 ], "cmp_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0xd2 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xd3 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xd4 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xd5 ], "cmp_aa_x" );   
	strcpy ( cpu6502.op_name[ 0xd6 ], "dec_aa_x" );   
	strcpy ( cpu6502.op_name[ 0xd7 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xd8 ], "cld" );        
	strcpy ( cpu6502.op_name[ 0xd9 ], "cmp_aaaa_y" ); 
	strcpy ( cpu6502.op_name[ 0xda ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xdb ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xdc ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xdd ], "cmp_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0xde ], "dec_aaaa_x" ); 
	strcpy ( cpu6502.op_name[ 0xdf ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xe0 ], "cpx_d_aa" );   
	strcpy ( cpu6502.op_name[ 0xe1 ], "sbc_i_aa_x" ); 
	strcpy ( cpu6502.op_name[ 0xe2 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xe3 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xe4 ], "cpx_aa" );     
	strcpy ( cpu6502.op_name[ 0xe5 ], "sbc_aa" );     
	strcpy ( cpu6502.op_name[ 0xe6 ], "inc_aa" );     
	strcpy ( cpu6502.op_name[ 0xe7 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xe8 ], "inx" );        
	strcpy ( cpu6502.op_name[ 0xe9 ], "sbc_d_aa" );   
	strcpy ( cpu6502.op_name[ 0xea ], "nop" );        
	strcpy ( cpu6502.op_name[ 0xeb ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xec ], "cpx_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xed ], "sbc_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xee ], "inc_aaaa" );   
	strcpy ( cpu6502.op_name[ 0xef ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xf0 ], "beq" );        
	strcpy ( cpu6502.op_name[ 0xf1 ], "sbc_i_aa_y" ); 
	strcpy ( cpu6502.op_name[ 0xf2 ], "run_esc" );      
	strcpy ( cpu6502.op_name[ 0xf3 ], "badop" );      
	strcpy ( cpu6502.op_name[ 0xf4 ], "nop_aa_x*" );      
	strcpy ( cpu6502.op_name[ 0xf5 ], "sbc_aa_x" );   
	strcpy ( cpu6502.op_name[ 0xf6 ], "inc_aa_x" );  
	strcpy ( cpu6502.op_name[ 0xf7 ], "badop" );     
	strcpy ( cpu6502.op_name[ 0xf8 ], "sed" );       
	strcpy ( cpu6502.op_name[ 0xf9 ], "sbc_aaaa_y" );
	strcpy ( cpu6502.op_name[ 0xfa ], "badop" );    
	strcpy ( cpu6502.op_name[ 0xfb ], "ins_aaaa_y*" );   
	strcpy ( cpu6502.op_name[ 0xfc ], "badop" );     
	strcpy ( cpu6502.op_name[ 0xfd ], "sbc_aaaa_x" );
	strcpy ( cpu6502.op_name[ 0xfe ], "inc_aaaa_x" );
	strcpy ( cpu6502.op_name[ 0xff ], "ins_aaaa_x*" );     
}
