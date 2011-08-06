#ifndef __SMS_CART_H
#define __SMS_CART_H

namespace MDFN_IEN_SMS
{

bool SMS_CartInit(const uint8 *data, uint32 size);

void SMS_CartWrite(uint16 A, uint8 V);
uint8 SMS_CartRead(uint16 A);
void SMS_CartReset(void);
void SMS_CartClose(void);
int SMS_CartStateAction(StateMem *sm, int load, int data_only);

}
#endif
