#ifndef __MDFN_PSX_INPUT_DUALSHOCK_H
#define __MDFN_PSX_INPUT_DUALSHOCK_H

#include <string>

namespace MDFN_IEN_PSX
{
InputDevice *Device_DualShock_Create(const std::string &name);
extern InputDeviceInputInfoStruct Device_DualShock_IDII[26];
}
#endif
