#ifndef __PCE_INPUT_GAMEPAD_H
#define __PCE_INPUT_GAMEPAD_H

namespace MDFN_IEN_PCE
{

extern const InputDeviceInputInfoStruct PCE_GamepadIDII[0xD];
extern const InputDeviceInputInfoStruct PCE_GamepadIDII_DSR[0xD];

PCE_Input_Device *PCEINPUT_MakeGamepad(int which);

};

#endif
