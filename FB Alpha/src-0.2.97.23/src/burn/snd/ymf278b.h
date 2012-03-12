#ifndef __YMF278B_H__
#define __YMF278B_H__

#ifdef FBA															/* !!! FBA */
#define MAX_YMF278B	(1)
#else																/* !!! FBA */
#define MAX_YMF278B	(2)
#endif																/* !!! FBA */

#define YMF278B_STD_CLOCK (33868800)						/* standard clock for OPL4 */

struct YMF278B_interface {
	int num;        										/* Number of chips */
	int clock[MAX_YMF278B];									/* clock input, normally 33.8688 MHz */
	int region[MAX_YMF278B];								/* memory region of sample ROMs */
	int mixing_level[MAX_YMF278B];							/* volume */
	void (*irq_callback[MAX_YMF278B])(int num, int state);	/* irq callback */
};

#ifdef FBA															/* !!! FBA */
void ymf278b_pcm_update(int num, INT16 **outputs, int length);
int ymf278b_timer_over(int num, int timer);
int ymf278b_start(INT8 num, UINT8 *rom, void (*irq_cb)(int, int), void (*timer_cb)(int, int, double), int clock, int rate);
#else																/* !!! FBA */
int  YMF278B_sh_start( const struct MachineSound *msound );
#endif																/* !!! FBA */
void YMF278B_sh_stop(void);

READ8_HANDLER( YMF278B_status_port_0_r );
READ8_HANDLER( YMF278B_data_port_0_r );
WRITE8_HANDLER( YMF278B_control_port_0_A_w );
WRITE8_HANDLER( YMF278B_data_port_0_A_w );
WRITE8_HANDLER( YMF278B_control_port_0_B_w );
WRITE8_HANDLER( YMF278B_data_port_0_B_w );
WRITE8_HANDLER( YMF278B_control_port_0_C_w );
WRITE8_HANDLER( YMF278B_data_port_0_C_w );

#ifndef FBA															/* !!! FBA */
READ8_HANDLER( YMF278B_status_port_1_r );
READ8_HANDLER( YMF278B_data_port_1_r );
WRITE8_HANDLER( YMF278B_control_port_1_A_w );
WRITE8_HANDLER( YMF278B_data_port_1_A_w );
WRITE8_HANDLER( YMF278B_control_port_1_B_w );
WRITE8_HANDLER( YMF278B_data_port_1_B_w );
WRITE8_HANDLER( YMF278B_control_port_1_C_w );
WRITE8_HANDLER( YMF278B_data_port_1_C_w );
#endif																/* !!! FBA */

#endif
