#ifndef __PCFX_INPUT_GAMEPAD_H
#define __PCFX_INPUT_GAMEPAD_H

extern const InputDeviceInputInfoStruct PCFX_GamepadIDII[0xF];
extern const InputDeviceInputInfoStruct PCFX_GamepadIDII_DSR[0xF];

PCFX_Input_Device *PCFXINPUT_MakeGamepad(int which);

#endif
