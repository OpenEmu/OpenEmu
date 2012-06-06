/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "psx.h"
#include "frontio.h"
#include "../FileWrapper.h"

#include "input/gamepad.h"
#include "input/dualanalog.h"
#include "input/mouse.h"
#include "input/memcard.h"

#include "input/multitap.h"

#define PSX_FIODBGINFO(format, ...) { /* printf(format " -- timestamp=%d -- PAD temp\n", ## __VA_ARGS__, timestamp); */  }

namespace MDFN_IEN_PSX
{

InputDevice::InputDevice()
{
}

InputDevice::~InputDevice()
{
}

void InputDevice::Power(void)
{
}

void InputDevice::Update(const pscpu_timestamp_t timestamp)
{

}

void InputDevice::ResetTS(void)
{

}


void InputDevice::UpdateInput(const void *data)
{
}


void InputDevice::SetDTR(bool new_dtr)
{

}

bool InputDevice::GetDSR(void)
{
 return(0);
}

bool InputDevice::Clock(bool TxD, int32 &dsr_pulse_delay)
{
 dsr_pulse_delay = 0;

 return(1);
}

uint32 InputDevice::GetNVSize(void)
{
 return(0);
}

void InputDevice::ReadNV(uint8 *buffer, uint32 offset, uint32 count)
{

}

void InputDevice::WriteNV(const uint8 *buffer, uint32 offset, uint32 count)
{

}

uint64 InputDevice::GetNVDirtyCount(void)
{
 return(0);
}

void InputDevice::ResetNVDirtyCount(void)
{

}

static INLINE unsigned EP_to_MP(unsigned ep)
{
#if 0
 return((bool)(ep & 0x4));
#else
 return(ep == 1 || ep >= 5);
#endif
}

static INLINE unsigned EP_to_SP(unsigned ep)
{
#if 0
 return(ep & 0x3);
#else
 if(ep < 2)
  return(0);
 else if(ep < 5)
  return(ep - 2 + 1);
 else
  return(ep - 5 + 1);
#endif
}

void FrontIO::MapDevicesToPorts(void)
{
 for(unsigned i = 0; i < 2; i++)
 {
  if(emulate_multitap[i])
  {
   Ports[i] = DevicesTap[i];
   MCPorts[i] = DummyDevice;
  }
  else
  {
   Ports[i] = Devices[i];
   MCPorts[i] = emulate_memcards[i] ? DevicesMC[i] : DummyDevice;
  }
 }

 for(unsigned i = 0; i < 8; i++)
 {
  unsigned mp = EP_to_MP(i);
  
  if(emulate_multitap[mp])
   DevicesTap[mp]->SetSubDevice(EP_to_SP(i), Devices[i], emulate_memcards[i] ? DevicesMC[i] : DummyDevice);
  else
   DevicesTap[mp]->SetSubDevice(EP_to_SP(i), DummyDevice, DummyDevice);
 }
}

FrontIO::FrontIO(bool emulate_memcards_[8], bool emulate_multitap_[2])
{
 memcpy(emulate_memcards, emulate_memcards_, sizeof(emulate_memcards));
 memcpy(emulate_multitap, emulate_multitap_, sizeof(emulate_multitap));

 DummyDevice = new InputDevice;

 for(int i = 0; i < 8; i++)
 {
  DeviceData[i] = NULL;
  Devices[i] = new InputDevice;
  DevicesMC[i] = Device_Memcard_Create();
 }

 for(unsigned i = 0; i < 2; i++)
 {
  DevicesTap[i] = new InputDevice_Multitap;
 }

 MapDevicesToPorts();
}

FrontIO::~FrontIO()
{
 for(int i = 0; i < 8; i++)
 {
  if(Devices[i])
  {
   delete Devices[i];
   Devices[i] = NULL;
  }
  if(DevicesMC[i])
  {
   delete DevicesMC[i];
   DevicesMC[i] = NULL;
  }
 }

 for(unsigned i = 0; i < 2; i++)
 {
  if(DevicesTap[i])
  {
   delete DevicesTap[i];
   DevicesTap[i] = NULL;
  }
 }
}

int32 FrontIO::CalcNextEvent(int32 next_event)
{
 if(ClockDivider > 0 && ClockDivider < next_event)
  next_event = ClockDivider;

 for(int i = 0; i < 4; i++)
  if(dsr_pulse_delay[i] > 0 && next_event > dsr_pulse_delay[i])
   next_event = dsr_pulse_delay[i];

 return(next_event);
}

void FrontIO::CheckStartStopPending(pscpu_timestamp_t timestamp, bool skip_event_set)
{
 //const bool prior_ReceiveInProgress = ReceiveInProgress;
 //const bool prior_TransmitInProgress = TransmitInProgress;
 bool trigger_condition = false;

 trigger_condition = (ReceivePending && (Control & 0x4)) || (TransmitPending && (Control & 0x1));

 if(trigger_condition)
 {
  if(ReceivePending)
  {
   ReceivePending = false;
   ReceiveInProgress = true;
   ReceiveBufferAvail = false;
   ReceiveBuffer = 0;
   ReceiveBitCounter = 0;
  }

  if(TransmitPending)
  {
   TransmitPending = false;
   TransmitInProgress = true;
   TransmitBitCounter = 0;
  }

  ClockDivider = 0x44;
 }

 if(!(Control & 0x5))
 {
  ReceiveInProgress = false;
  TransmitInProgress = false;
 }

 if(!ReceiveInProgress && !TransmitInProgress)
  ClockDivider = 0;

 if(!(skip_event_set))
  PSX_SetEventNT(PSX_EVENT_FIO, timestamp + CalcNextEvent(0x10000000));
}

// DSR IRQ bit setting appears(from indirect tests on real PS1) to be level-sensitive, not edge-sensitive
INLINE void FrontIO::DoDSRIRQ(void)
{
 if(Control & 0x1000)
 {
  PSX_FIODBGINFO("[DSR] IRQ");
  istatus = true;
  IRQ_Assert(IRQ_SIO, true);
 }
}


void FrontIO::Write(pscpu_timestamp_t timestamp, uint32 A, uint32 V)
{
 assert(!(A & 0x1));

 PSX_FIODBGINFO("[FIO] Write: %08x %08x", A, V);

 Update(timestamp);

 switch(A & 0xF)
 {
  case 0x0:
	TransmitBuffer = V;
	TransmitPending = true;
	TransmitInProgress = false;
	break;

  case 0x8:
	Mode = V & 0x013F;
	break;

  case 0xa:
	if(ClockDivider > 0 && ((V & 0x2000) != (Control & 0x2000)) && ((Control & 0x2) == (V & 0x2))  )
	 fprintf(stderr, "FIO device selection changed during comm %04x->%04x", Control, V);

	Control = V & 0x3F2F;

	if(V & 0x10)
        {
	 istatus = false;
	 IRQ_Assert(IRQ_SIO, false);
	}

	if(V & 0x40)	// Reset
	{
	 istatus = false;
	 IRQ_Assert(IRQ_SIO, false);

	 ClockDivider = 0;
	 ReceivePending = false;
	 TransmitPending = false;

	 ReceiveInProgress = false;
	 TransmitInProgress = false;

	 ReceiveBufferAvail = false;

	 TransmitBuffer = 0;
	 ReceiveBuffer = 0;

	 ReceiveBitCounter = 0;
	 TransmitBitCounter = 0;

	 Mode = 0;
	 Control = 0;
	 Baudrate = 0;
	}

	Ports[0]->SetDTR((Control & 0x2) && !(Control & 0x2000));
        MCPorts[0]->SetDTR((Control & 0x2) && !(Control & 0x2000));
	Ports[1]->SetDTR((Control & 0x2) && (Control & 0x2000));
        MCPorts[1]->SetDTR((Control & 0x2) && (Control & 0x2000));

#if 1
if(!((Control & 0x2) && !(Control & 0x2000)))
{
 dsr_pulse_delay[0] = 0;
 dsr_pulse_delay[2] = 0;
 dsr_active_until_ts[0] = -1;
 dsr_active_until_ts[2] = -1;
}

if(!((Control & 0x2) && (Control & 0x2000)))
{
 dsr_pulse_delay[1] = 0;
 dsr_pulse_delay[3] = 0;
 dsr_active_until_ts[1] = -1;
 dsr_active_until_ts[3] = -1;
}

#endif
	// TODO: Uncomment out in the future once our CPU emulation is a bit more accurate with timing, to prevent causing problems with games
	// that may clear the IRQ in an unsafe pattern that only works because its execution was slow enough to allow DSR to go inactive.  (Whether or not
	// such games even exist though is unknown!)
	//if(timestamp < dsr_active_until_ts[0] || timestamp < dsr_active_until_ts[1] || timestamp < dsr_active_until_ts[2] || timestamp < dsr_active_until_ts[3])
	// DoDSRIRQ();

	break;

  case 0xe:
	Baudrate = V;
	//MDFN_DispMessage("%02x\n", V);
	break;
 }

 CheckStartStopPending(timestamp, false);
}


uint32 FrontIO::Read(pscpu_timestamp_t timestamp, uint32 A)
{
 uint32 ret = 0;

 assert(!(A & 0x1));

 Update(timestamp);

 switch(A & 0xF)
 {
  case 0x0:
	ret = ReceiveBuffer | (ReceiveBuffer << 8) | (ReceiveBuffer << 16) | (ReceiveBuffer << 24);
	ReceiveBufferAvail = false;
	ReceivePending = true;
	ReceiveInProgress = false;
	CheckStartStopPending(timestamp, false);
	break;

  case 0x4:
	ret = 0;

	if(!TransmitPending && !TransmitInProgress)
	 ret |= 0x1;

	if(ReceiveBufferAvail)
	 ret |= 0x2;

	if(timestamp < dsr_active_until_ts[0] || timestamp < dsr_active_until_ts[1] || timestamp < dsr_active_until_ts[2] || timestamp < dsr_active_until_ts[3])
	 ret |= 0x80;

	if(istatus)
	 ret |= 0x200;

	break;

  case 0x8:
	ret = Mode;
	break;

  case 0xa:
	ret = Control;
	break;

  case 0xe:
	ret = Baudrate;
	break;
 }

 if((A & 0xF) != 0x4)
  PSX_FIODBGINFO("[FIO] Read: %08x %08x", A, ret);

 return(ret);
}

pscpu_timestamp_t FrontIO::Update(pscpu_timestamp_t timestamp)
{
 int32 clocks = timestamp - lastts;
 bool need_start_stop_check = false;

 for(int i = 0; i < 4; i++)
  if(dsr_pulse_delay[i] > 0)
  {
   dsr_pulse_delay[i] -= clocks;
   if(dsr_pulse_delay[i] <= 0)
   {
    dsr_active_until_ts[i] = timestamp + 32 + dsr_pulse_delay[i];
    DoDSRIRQ();
   }
  }


 if(ClockDivider > 0)
 {
  ClockDivider -= clocks;

  while(ClockDivider <= 0)
  {
   if(ReceiveInProgress || TransmitInProgress)
   {
    bool rxd = 0, txd = 0;
    const uint32 BCMask = 0x07;

    if(TransmitInProgress)
    {
     txd = (TransmitBuffer >> TransmitBitCounter) & 1;
     TransmitBitCounter = (TransmitBitCounter + 1) & BCMask;
     if(!TransmitBitCounter)
     {
      need_start_stop_check = true;
      PSX_FIODBGINFO("[FIO] Data transmitted: %08x", TransmitBuffer);
      TransmitInProgress = false;

      if(Control & 0x400)
      {
       istatus = true;
       IRQ_Assert(IRQ_SIO, true);
      }
     }
    }

    rxd = Ports[0]->Clock(txd, dsr_pulse_delay[0]) & Ports[1]->Clock(txd, dsr_pulse_delay[1]) &
	  MCPorts[0]->Clock(txd, dsr_pulse_delay[2]) & MCPorts[1]->Clock(txd, dsr_pulse_delay[3]);

    if(ReceiveInProgress)
    {
     ReceiveBuffer &= ~(1 << ReceiveBitCounter);
     ReceiveBuffer |= rxd << ReceiveBitCounter;

     ReceiveBitCounter = (ReceiveBitCounter + 1) & BCMask;

     if(!ReceiveBitCounter)
     {
      need_start_stop_check = true;
      PSX_FIODBGINFO("[FIO] Data received: %08x", ReceiveBuffer);

      ReceiveInProgress = false;
      ReceiveBufferAvail = true;

      if(Control & 0x800)
      {
       istatus = true;
       IRQ_Assert(IRQ_SIO, true);
      }
     }
    }
    ClockDivider += 0x44; //88; //99;
   }
   else
    break;
  }
 }


 lastts = timestamp;


 if(need_start_stop_check)
 {
  CheckStartStopPending(timestamp, true);
 }

 return(timestamp + CalcNextEvent(0x10000000));
}

void FrontIO::ResetTS(void)
{
 for(int i = 0; i < 8; i++)
 {
  Devices[i]->Update(lastts);	// Maybe eventually call Update() from FrontIO::Update() and remove this(but would hurt speed)?
  Devices[i]->ResetTS();

  DevicesMC[i]->Update(lastts);	// Maybe eventually call Update() from FrontIO::Update() and remove this(but would hurt speed)?
  DevicesMC[i]->ResetTS();
 }

 for(int i = 0; i < 2; i++)
 {
  DevicesTap[i]->Update(lastts);
  DevicesTap[i]->ResetTS();
 }

 for(int i = 0; i < 4; i++)
 {
  if(dsr_active_until_ts[i] >= 0)
  {
   dsr_active_until_ts[i] -= lastts;
   //printf("SPOOONY: %d %d\n", i, dsr_active_until_ts[i]);
  }
 }
 lastts = 0;
}


void FrontIO::Power(void)
{
 for(int i = 0; i < 4; i++)
 {
  dsr_pulse_delay[i] = 0;
  dsr_active_until_ts[i] = -1;
 }

 lastts = 0;

 //
 //

 ClockDivider = 0;

 ReceivePending = false;
 TransmitPending = false;

 ReceiveInProgress = false;
 TransmitInProgress = false;

 ReceiveBufferAvail = false;

 TransmitBuffer = 0;
 ReceiveBuffer = 0;

 ReceiveBitCounter = 0;
 TransmitBitCounter = 0;

 Mode = 0;
 Control = 0;
 Baudrate = 0;

 for(int i = 0; i < 8; i++)
 {
  Devices[i]->Power();
  DevicesMC[i]->Power();
 }

 istatus = false;
}

void FrontIO::UpdateInput(void)
{
 for(int i = 0; i < 8; i++)
  Devices[i]->UpdateInput(DeviceData[i]);
}

void FrontIO::SetInput(unsigned int port, const char *type, void *ptr)
{
 delete Devices[port];
 Devices[port] = NULL;

 if(!strcmp(type, "gamepad"))
  Devices[port] = Device_Gamepad_Create();
 else if(!strcmp(type, "dualanalog"))
  Devices[port] = Device_DualAnalog_Create(false);
 else if(!strcmp(type, "analogjoy"))
  Devices[port] = Device_DualAnalog_Create(true);
 else if(!strcmp(type, "mouse"))
  Devices[port] = Device_Mouse_Create();
 else
  Devices[port] = new InputDevice;

 DeviceData[port] = ptr;

 MapDevicesToPorts();
}

uint64 FrontIO::GetMemcardDirtyCount(unsigned int which)
{
 assert(which < 8);

 return(DevicesMC[which]->GetNVDirtyCount());
}

void FrontIO::LoadMemcard(unsigned int which, const char *path)
{
 assert(which < 8);

 try
 {
  if(DevicesMC[which]->GetNVSize())
  {
   FileWrapper mf(path, FileWrapper::MODE_READ);
   std::vector<uint8> tmpbuf;

   tmpbuf.resize(DevicesMC[which]->GetNVSize());

   if(mf.size() != (int64)tmpbuf.size())
    throw(MDFN_Error(0, _("Memory card file \"%s\" is an incorrect size(%d bytes).  The correct size is %d bytes."), path, (int)mf.size(), (int)tmpbuf.size()));

   mf.read(&tmpbuf[0], tmpbuf.size());

   DevicesMC[which]->WriteNV(&tmpbuf[0], 0, tmpbuf.size());
   DevicesMC[which]->ResetNVDirtyCount();		// There's no need to rewrite the file if it's the same data.
  }
 }
 catch(MDFN_Error &e)
 {
  if(e.GetErrno() != ENOENT)
   throw(e);
 }
}

void FrontIO::SaveMemcard(unsigned int which, const char *path)
{
 assert(which < 8);

 if(DevicesMC[which]->GetNVSize() && DevicesMC[which]->GetNVDirtyCount())
 {
  FileWrapper mf(path, FileWrapper::MODE_WRITE);	// TODO: MODE_WRITE_ATOMIC_OVERWRITE
  std::vector<uint8> tmpbuf;

  tmpbuf.resize(DevicesMC[which]->GetNVSize());

  DevicesMC[which]->ReadNV(&tmpbuf[0], 0, tmpbuf.size());
  mf.write(&tmpbuf[0], tmpbuf.size());

  mf.close();	// Call before resetting the NV dirty count!

  DevicesMC[which]->ResetNVDirtyCount();
 }
}



static InputDeviceInfoStruct InputDeviceInfoPSXPort[] =
{
 // None
 {
  "none",
  "none",
  NULL,
  0,
  NULL 
 },

 // Gamepad(SCPH-1080)
 {
  "gamepad",
  "Digital Gamepad",
  NULL,
  sizeof(Device_Gamepad_IDII) / sizeof(InputDeviceInputInfoStruct),
  Device_Gamepad_IDII,
 },

 // Dual Analog Gamepad(SCPH-1180), forced to analog mode.
 {
  "dualanalog",
  "Dual Analog",
  NULL,
  sizeof(Device_DualAnalog_IDII) / sizeof(InputDeviceInputInfoStruct),
  Device_DualAnalog_IDII,
 },


 // Analog joystick(SCPH-1110), forced to analog mode - emulated through a tweak to dual analog gamepad emulation.
 {
  "analogjoy",
  "Analog Joystick",
  NULL,
  sizeof(Device_AnalogJoy_IDII) / sizeof(InputDeviceInputInfoStruct),
  Device_AnalogJoy_IDII,
 },

 {
  "mouse",
  "Mouse",
  NULL,
  sizeof(Device_Mouse_IDII) / sizeof(InputDeviceInputInfoStruct),
  Device_Mouse_IDII,
 },

};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "port1", "Port 1/1A", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port2", "Port 2/2A", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port3", "Port 1B", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port4", "Port 1C", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port5", "Port 1D", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port6", "Port 2B", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port7", "Port 2C", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
 { 0, "port8", "Port 2D", sizeof(InputDeviceInfoPSXPort) / sizeof(InputDeviceInfoStruct), InputDeviceInfoPSXPort, "gamepad" },
};

InputInfoStruct FIO_InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


}
