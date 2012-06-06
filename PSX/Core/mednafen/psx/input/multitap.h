#ifndef __MDFN_PSX_INPUT_MULTITAP_H
#define __MDFN_PSX_INPUT_MULTITAP_H

namespace MDFN_IEN_PSX
{

class InputDevice_Multitap : public InputDevice
{
 public:

 InputDevice_Multitap();
 virtual ~InputDevice_Multitap();
 virtual void Power(void);

 void SetSubDevice(unsigned int sub_index, InputDevice *device, InputDevice *mc_device);

 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool GetDSR(void);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 private:

 InputDevice *pad_devices[4];
 InputDevice *mc_devices[4];

 bool dtr;

 int selected_device;
 bool full_mode_setting;

 bool full_mode;
 bool mc_mode;

 uint8 command;
 uint8 receive_buffer;
 uint8 bit_counter;
 uint8 byte_counter;
};

}

#endif
