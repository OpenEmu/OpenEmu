#ifndef __MDFN_PSX_INPUT_DUALANALOG_H
#define __MDFN_PSX_INPUT_DUALANALOG_H

namespace MDFN_IEN_PSX
{

InputDevice *Device_DualAnalog_Create(bool joystick_mode);
extern InputDeviceInputInfoStruct Device_DualAnalog_IDII[24];
extern InputDeviceInputInfoStruct Device_AnalogJoy_IDII[24];
}
#endif
