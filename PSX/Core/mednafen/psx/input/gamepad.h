#ifndef __MDFN_PSX_INPUT_GAMEPAD_H
#define __MDFN_PSX_INPUT_GAMEPAD_H

namespace MDFN_IEN_PSX
{

InputDevice *Device_Gamepad_Create(void);
extern InputDeviceInputInfoStruct Device_Gamepad_IDII[16];
extern InputDeviceInputInfoStruct Device_Dancepad_IDII[16];

}
#endif
