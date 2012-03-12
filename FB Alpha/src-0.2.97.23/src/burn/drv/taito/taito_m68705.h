#include "m6805_intf.h"

struct m68705_interface {
	void (*portA_out)(UINT8 *data);
	void (*portB_out)(UINT8 *data);
	void (*portC_out)(UINT8 *data);
	void (*ddrA_out)(UINT8 *data);
	void (*ddrB_out)(UINT8 *data);
	void (*ddrC_out)(UINT8 *data);
	void (*portA_in)();
	void (*portB_in)();
	void (*portC_in)();
};

extern UINT8 portA_in;
extern UINT8 portA_out;
extern UINT8 ddrA;
extern UINT8 portB_in;
extern UINT8 portB_out;
extern UINT8 ddrB;
extern UINT8 portC_in;
extern UINT8 portC_out;
extern UINT8 ddrC;

extern UINT8 from_main;
extern UINT8 from_mcu;
extern INT32 mcu_sent;
extern INT32 main_sent;

void m67805_taito_reset();
void m67805_taito_init(UINT8 *rom, UINT8 *ram, m68705_interface *interface);
void m67805_taito_exit();
INT32 m68705_taito_scan(INT32 nAction);

void standard_taito_mcu_write(INT32 data);
INT32 standard_taito_mcu_read();
void standard_m68705_portB_out(UINT8 *data);
void standard_m68705_portC_in();

extern m68705_interface standard_m68705_interface;
