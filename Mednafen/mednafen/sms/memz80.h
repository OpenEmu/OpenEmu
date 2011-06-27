
#ifndef _MEMZ80_H_
#define _MEMZ80_H_

namespace MDFN_IEN_SMS
{

/* Global data */
extern uint8 data_bus_pullup;
extern uint8 data_bus_pulldown;

/* Function prototypes */
uint8 z80_read_unmapped(void);
void gg_port_w(uint16 port, uint8 data);
uint8 gg_port_r(uint16 port);
void ggms_port_w(uint16 port, uint8 data);
uint8 ggms_port_r(uint16 port);
void sms_port_w(uint16 port, uint8 data);
uint8 sms_port_r(uint16 port);
void smsj_port_w(uint16 port, uint8 data);
uint8 smsj_port_r(uint16 port);
void md_port_w(uint16 port, uint8 data);
uint8 md_port_r(uint16 port);

}

#endif /* _MEMZ80_H_ */
