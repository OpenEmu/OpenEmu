#ifndef __MDFN_DRIVERS_JOYSTICK_H
#define __MDFN_DRIVERS_JOYSTICK_H

int DTestButtonJoy(ButtConfig &bc);
int InitJoysticks(void);
int KillJoysticks(void);
uint32 *GetJSOr(void);
uint64 GetJoystickUniqueID(int num);

void JoyClearBC(ButtConfig &bc);

#endif
