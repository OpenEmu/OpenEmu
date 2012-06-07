#include <mednafen/hw_sound/ym2413/emu2413.h>

class MCGenjin_CS_Device_YM2413 : public MCGenjin_CS_Device
{
 public:

 MCGenjin_CS_Device_YM2413(Blip_Buffer *bb)
 {
  zec = EMU2413_new(3579545);
  fm_last_value = 0;

  sbuf = bb;
 }

 virtual ~MCGenjin_CS_Device_YM2413()
 {
  EMU2413_delete(zec);
 }

 virtual void Power(void)
 {
  EMU2413_reset(zec);
  last_ts = 0;
  fm_div = 0;
 }

 virtual int StateAction(StateMem *sm, int load, int data_only, const char *sname)
 {
  SFORMAT StateRegs[] = 
  {
   SFEND
  };
  int ret = 1;

  ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, sname);

  return ret;
 }

 virtual uint8 Read(int32 timestamp, uint32 A)
 {
  return 0xFF;
 }

 virtual void Write(int32 timestamp, uint32 A, uint8 V)
 {
  Update(timestamp / 3);
  EMU2413_writeIO(zec, A & 1, V);
 }

 virtual void EndFrame(int32 timestamp)
 {
  Update(timestamp / 3);
  last_ts = 0;
  sbuf->set_modified();
 }

 private:

 void Update(int32 timestamp)
 {
  int32 cycles = timestamp - last_ts;
 
  fm_div -= cycles;
  while(fm_div <= 0)
  {
   int32 new_value;

   new_value = EMU2413_calc(zec) >> 1;

   if(new_value > 32767) 
    new_value = 32767;

   if(new_value < -32768)
    new_value = -32768;

   fmsynth.offset(timestamp + fm_div, new_value - fm_last_value, sbuf);

   fm_last_value = new_value;
   fm_div += 72;
  }

  last_ts = timestamp;
 }

 EMU2413 *zec;
 Blip_Buffer *sbuf;

 typedef Blip_Synth<blip_good_quality, 8192> FMSynth_t;
 FMSynth_t fmsynth;

 int32 fm_div;
 int32 fm_last_value;

 int32 last_ts;
};

