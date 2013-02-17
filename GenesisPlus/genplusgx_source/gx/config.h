/****************************************************************************
 *  config.c
 *
 *  Genesis Plus GX configuration file support
 *
 *  Copyright Eke-Eke (2007-2012)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define CONFIG_VERSION "GENPLUS-GX 1.7.2"

/****************************************************************************
 * Config Option 
 *
 ****************************************************************************/
typedef struct 
{
  char version[16];
  uint8 hq_fm;
  uint8 filter;
  uint8 psgBoostNoise;
  uint8 dac_bits;
  uint8 ym2413;
  int16 psg_preamp;
  int16 fm_preamp;
  int16 lp_range;
  int16 low_freq;
  int16 high_freq;
  int16 lg;
  int16 mg;
  int16 hg;
  uint8 system;
  uint8 region_detect;
  uint8 master_clock;
  uint8 vdp_mode;
  uint8 force_dtack;
  uint8 addr_error;
  uint8 tmss;
  uint8 bios;
  uint8 lock_on;
  uint8 hot_swap;
  uint8 invert_mouse;
  uint8 gun_cursor[2];
  uint8 overscan;
  uint8 gg_extra;
  uint8 ntsc;
  uint8 vsync;
  uint8 render;
  uint8 tv_mode;
  uint8 bilinear;
  uint8 aspect;
  int16 xshift;
  int16 yshift;
  int16 xscale;
  int16 yscale;
#ifdef HW_RVL
  uint32 trap;
  float gamma;
#else
  uint8 v_prog;
#endif
  t_input_config input[MAX_INPUTS];
  uint16 pad_keymap[4][MAX_KEYS+1];
#ifdef HW_RVL
  uint32 wpad_keymap[4*3][MAX_KEYS];
#endif
  uint8 autoload;
  uint8 autocheat;
  uint8 s_auto;
  uint8 s_default;
  uint8 s_device;
  uint8 l_device;
  uint8 bg_overlay;
  uint8 cd_leds;
  int16 screen_w;
  float bgm_volume;
  float sfx_volume;
  char lastdir[FILETYPE_MAX][TYPE_RECENT][MAXPATHLEN];
} t_config;

/* Global data */
t_config config;


extern void config_save(void);
extern void config_default(void);


#endif /* _CONFIG_H_ */

