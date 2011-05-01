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

#include "nes.h"
#include "x6502.h"
#include "fds.h"
#include "sound.h"
#include "cart.h"
#include "nsf.h"
#include "fds-sound.h"

static void RenderSoundHQ(void);

static DECLFW(FDSWaveWrite);
static DECLFR(FDSWaveRead);

static DECLFR(FDSSRead);
static DECLFW(FDSSWrite);

//static bool cwave_ip = TRUE;

static int32 env_divider;	// Main envelope clock divider.

static int32 sample_out_cache;	// Sample out cache, with volume stuff applied.

static uint32 cwave_freq;	// $4082 and lower 4 bits of $4083
static uint32 cwave_pos;
static uint8 cwave_control;	// $4083, lower 6 bits masked out.

static uint32 mod_freq;		// $4086 and lower 4 bits of $4087
static uint32 mod_pos;		// Should be named "mwave_pos", but "mod_pos" distinguishes it more.
static bool mod_disabled;	// Upper bit of $4087

static uint8 master_control;	// Waveform write enable and master volume($4089)
static uint8 env_speed;		// Master envelope speed controller($408A).

static uint8 env_control[2];	// Envelope control(4080/4084)
static uint8 volume[2];		// Current volumes.

static int32 mwave[0x20];      // Modulation waveform.  Stored in expanded(after LUT) form.
			       // Set to 0x10 if the original value is 0x4(reset sweep bias accumulator).
static uint8 cwave[0x40];      // Game-defined waveform(carrier)

static int32 env_counter[2];

static uint32 sweep_bias;

static int32 cov;		// Current out volume, 8 bits of after-decimal place precision.

static int32 FBC = 0;

// We need to call this function whenever volume[0] changes, and on changes in the lower 2 bits of master_control,
static INLINE void CalcCOV(void)
{
 int k = volume[0];

// printf("%d %d\n", volume[0], master_control & 0x3);

 if(k > 0x20)
  k = 0x20;

 cov = 256 * k * 6 / ((master_control & 0x3) + 2);
}

static DECLFR(FDSSRead)
{
 if(!fceuindbg)
  RenderSoundHQ();

 //printf("Read: %04x\n", A);
 switch(A & 0xF)
 {
  case 0x0: return(volume[0] | (X.DB&0xC0));
  case 0x2: return(volume[1] | (X.DB&0xC0));
 }
 return(X.DB);
}

bool reset_mode = 0;

static INLINE int GetEnvReload(uint8 value)
{
 int ret;

 ret = (value & 0x3F) + 1;

 return(ret);
}

static DECLFW(FDSSWrite)
{
 RenderSoundHQ();

 //printf("%04x %02x, %lld\n", A, V, timestampbase + timestamp);
 A -= 0x4080;
 switch(A)
 {
  default: //printf("%04x %02x\n", A, V);
	   break;

  case 0x0: if(V & 0x80)
	    {
	     volume[0] = V & 0x3F;
	     CalcCOV();
	    }
	    else if(env_control[0] & 0x80)
	     env_counter[0] = GetEnvReload(V);

	    env_control[0] = V;
	    break;

  case 0x2: cwave_freq &= 0xFF00;
	    cwave_freq |= V << 0;
	    break;

  case 0x3:
	    if(!(V & 0x80) && (cwave_control & 0x80))
	     cwave_pos = 0;

	    //if(!(V & 0x40) && (cwave_control & 0x40))
	    // env_divider = env_speed << 3;

	    cwave_freq &= 0x00FF;
	    cwave_freq |= (V & 0xF) << 8;
	    cwave_control = V & 0xC0;
	    break;

  case 0x4: if(V & 0x80)
	    {
	     volume[1] = V & 0x3F;
	    }
	    else if(env_control[1] & 0x80)
	     env_counter[1] = GetEnvReload(V);

	    env_control[1] = V;
	    break;

  case 0x5:	sweep_bias = (V & 0x7F) << 4;
		mod_pos = 0;
		//printf("Sweep Bias: %02x\n", V & 0x7F);
		break;

  case 0x6:	mod_freq &= 0xFF00;
		mod_freq |= V << 0;
		break;

  case 0x7:
		//if(!(V & 0x80) && mod_disabled)
		// mod_pos = 0;

		mod_freq &= 0x00FF;
		mod_freq |= (V & 0xF) << 8;
		mod_disabled = (bool)(V & 0x80);
		break;

  case 0x8:
		if(mod_disabled)
		{
		 static const int bias_tab[8] = { 0, 1, 2, 4, 0, -4, -2, -1 };

	   	 for(int i = 0; i < 31; i++)
	    	  mwave[i] = mwave[i + 1];

	   	 mwave[0x1F] = bias_tab[V & 0x7];

		 if((V & 0x7) == 0x4)
		  mwave[0x1F] = 0x10;
		}
		//else
		// puts("NYAR");
	   	break;

  case 0x9:	master_control = V;
		CalcCOV();
		break;

  case 0xA:	env_speed = V;
		break;
 }
}

static void ClockEnv(void)
{
 for(int x = 0; x < 2; x++)
 {
  if((env_control[x] & 0x80) || (cwave_control & 0x40))
   continue;
  
  env_counter[x]--;

  if(env_counter[x] <= 0)
  {
   if(env_control[x] & 0x40)	// Fade in
   {
    if(volume[x] < 0x20)
     volume[x]++;
   }
   else				// Fade out
   {
    if(volume[x] > 0)
     volume[x]--;
   }
   if(!x)
    CalcCOV();

   env_counter[x] = GetEnvReload(env_control[x]);
  }
 }
}

static DECLFR(FDSWaveRead)
{
 if(master_control & 0x80)
  return(cwave[A&0x3f]|(X.DB&0xC0));
 else
 {
  //puts("MEOWARG");
  return(X.DB);
 }
}

static DECLFW(FDSWaveWrite)
{
 if(master_control & 0x80)
 {
  const int index = A & 0x3F;

  //printf("%04x %02x, %lld\n", A, V, timestampbase + timestamp);

  cwave[index] = V & 0x3F;
 }
}

static uint32 prev_mod_pos = 0;
static int32 temp = 0;
int mod_div = 0;

static INLINE void ClockMod(void)
{
 if(!mod_disabled)
 {
  prev_mod_pos = mod_pos;

  mod_pos += mod_freq;

  if((mod_pos & (0x3F << 12)) != (prev_mod_pos & (0x3F << 12)))
  {
   const int32 mw = mwave[((mod_pos >> 17) & 0x1F)];

   sweep_bias = (sweep_bias + mw) & 0x7FF;
   //printf("%d %d\n", mod_pos >> 17, sweep_bias);
   if(mw == 0x10)
    sweep_bias = 0;
  }

  temp = sign_x_to_s32(11, sweep_bias) * ((volume[1] > 0x20) ? 0x20 : volume[1]);

  // >> 4 or / 16?  / 16 sounds better in Zelda...
  if(temp & 0x0F0)
  {
   temp /= 256;
   if(sweep_bias & 0x400)
    temp--;
   else
    temp += 2;
  }
  else
   temp /= 256;

  if(temp >= 194)
  {
   //printf("Oops: %d\n", temp);
   temp -= 258;
  }
  if(temp < -64)
  {
   //printf("Oops2: %d\n", temp);
   temp += 256;
  }
 }
}

static INLINE void ClockCarrier(void)
{
 int32 cur_cwave_freq;

 if(!mod_disabled)
 {
  cur_cwave_freq = (int32)(cwave_freq << 6) + (int32)cwave_freq * temp;

  if(cur_cwave_freq < 0) 
   cur_cwave_freq = 0;
 }
 else
  cur_cwave_freq = cwave_freq << 6;

 cwave_pos += cur_cwave_freq;
}

static INLINE void FDSDoSound(void)
{
 uint32 prev_cwave_pos = cwave_pos;

 ClockMod();

 if(!(master_control & 0x80) && !(cwave_control & 0x80))
  ClockCarrier();

 if(env_speed)
 {
  env_divider--;
  if(env_divider <= 0)
  {
   env_divider = env_speed << 3;

   ClockEnv();
  }
 }

 #if 0
 if(cwave_ip)
 {
  int k = volume[0];

  if(k > 0x20)
   k = 0x20;

  //printf("%d, %d, %lld\n", prev_cwave_pos >> 22, cwave_pos >> 22, timestampbase + timestamp);

  int32 cur, delta, iped;

  cur = (cwave[(cwave_pos >> 22) & 0x3F] - 0x20) * k * 6;
  delta = ((cwave[((cwave_pos >> 22) + 1) & 0x3F] - 0x20) * k * 6) - cur;

  iped = cur + ((delta * ((cwave_pos >> 14) & 0xFF)) >> 8);

  sample_out_cache = iped / ((master_control & 0x3) + 2);
 }
 else 
 #endif

 if((cwave_pos ^ prev_cwave_pos) & (1 << 22))
 //if(!(master_control & 0x80) && !(cwave_control & 0x80))
 {
  //printf("%d, %d, %lld\n", prev_cwave_pos >> 22, cwave_pos >> 22, timestampbase + timestamp);

  sample_out_cache = ((cwave[(cwave_pos >> 22) & 0x3F] - 0x20) * cov) >> 8;
 }
}

#define NZEROS 2
#define NPOLES 2
#define GAIN   5.172298875e+03

static float xv[NZEROS+1], yv[NPOLES+1];

static INLINE double filterloop(double input)
{
        xv[0] = xv[1];
	xv[1] = xv[2]; 
        xv[2] = input / GAIN;
        yv[0] = yv[1];
	yv[1] = yv[2]; 

        yv[2] =   (xv[0] + xv[2]) + 2 * xv[1]
                     + ( -0.9610604561 * yv[0]) + (  1.9602871056 * yv[1]);
        return(yv[2]);
}

static void RenderSoundHQ(void)
{
 //MDFN_DispMessage("%02x\n", volume[1]);
 for(uint32 x = FBC; x < SOUNDTS; x++)
 {
  FDSDoSound();

  WaveHiEx[x] += sample_out_cache; //filterloop(sample_out_cache);
 }

 FBC = SOUNDTS;
}

static void HQSync(int32 ts)
{
 FBC=ts;
}

int NSFFDS_Init(EXPSOUND *ep, bool MultiChip)
{
 //memset(&fdso,0,sizeof(fdso));

 ep->HiSync=HQSync;
 ep->HiFill=RenderSoundHQ;

 SetReadHandler(0x4040,0x407f,FDSWaveRead);
 NSFECSetWriteHandler(0x4040,0x407f,FDSWaveWrite);
 NSFECSetWriteHandler(0x4080,0x408A,FDSSWrite);
 SetReadHandler(0x4090,0x4092,FDSSRead);

 return(1);
}


bool FDSSound_Init(void)
{
 EXPSOUND TmpExpSound;
 memset(&TmpExpSound, 0, sizeof(TmpExpSound));

 SetReadHandler(0x4040, 0x407f, FDSWaveRead);
 SetWriteHandler(0x4040, 0x407f, FDSWaveWrite);
 SetWriteHandler(0x4080, 0x408A, FDSSWrite);
 SetReadHandler(0x4090, 0x4092, FDSSRead);

 TmpExpSound.HiSync=HQSync;
 TmpExpSound.HiFill=RenderSoundHQ;

 GameExpSound.push_back(TmpExpSound);

 return(1);
}


void FDSSound_Power(void)
{
 env_divider = 0;
 sample_out_cache = 0;
 
 cwave_freq = 0;
 cwave_pos = 0;
 cwave_control = 0;
 
 mod_freq = 0;
 mod_pos = 0;
 mod_disabled = 0;
 
 master_control = 0;
 env_speed = 0;

 memset(&env_control, 0, sizeof(env_control));
 memset(&volume, 0, sizeof(volume));
 memset(cwave, 0, sizeof(cwave));
 memset(mwave, 0, sizeof(mwave));
 memset(env_counter, 0, sizeof(env_counter));
 
 sweep_bias = 0;
}


int FDSSound_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(env_divider),

  SFVAR(sample_out_cache),

  SFVAR(cwave_freq),
  SFVAR(cwave_pos),
  SFVAR(cwave_control),
  
  SFVAR(mod_freq),
  SFVAR(mod_pos),
  SFVAR(mod_disabled),
  
  SFVAR(master_control),
  SFVAR(env_speed),

  SFARRAY(env_control, 2),
  SFARRAY(volume, 2),
  SFARRAY32(mwave, 0x20),
  SFARRAY(cwave, 0x40),
  
  SFARRAY32(env_counter, 2),
  
  SFVAR(sweep_bias),
  
  SFEND
 };

 if(!load)
 {

 }

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "FDSS");

 if(load)
 {
  for(int i = 0; i < 0x40; i++)
   cwave[i] &= 0x3F;
 }
 return(ret);
}
