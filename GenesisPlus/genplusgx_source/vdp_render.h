/***************************************************************************************
 *  Genesis Plus
 *  Video Display Processor (Modes 0, 1, 2, 3, 4 & 5 rendering)
 *
 *  Support for SG-1000, Master System (315-5124 & 315-5246), Game Gear & Mega Drive VDP
 *
 *  Copyright (C) 1998-2007  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
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

#ifndef _RENDER_H_
#define _RENDER_H_

/* Global variables */
extern uint8 object_count;
extern uint16 spr_col;

/* Function prototypes */
extern void render_init(void);
extern void render_reset(void);
extern void render_line(int line);
extern void blank_line(int line, int offset, int width);
extern void remap_line(int line);
extern void window_clip(unsigned int data, unsigned int sw);
extern void render_bg_m0(int line, int width);
extern void render_bg_m1(int line, int width);
extern void render_bg_m1x(int line, int width);
extern void render_bg_m2(int line, int width);
extern void render_bg_m3(int line, int width);
extern void render_bg_m3x(int line, int width);
extern void render_bg_inv(int line, int width);
extern void render_bg_m4(int line, int width);
extern void render_bg_m5(int line, int width);
extern void render_bg_m5_vs(int line, int width);
extern void render_bg_m5_im2(int line, int width);
extern void render_bg_m5_im2_vs(int line, int width);
extern void render_obj_tms(int max_width);
extern void render_obj_m4(int max_width);
extern void render_obj_m5(int max_width);
extern void render_obj_m5_ste(int max_width);
extern void render_obj_m5_im2(int max_width);
extern void render_obj_m5_im2_ste(int max_width);
extern void parse_satb_tms(int line);
extern void parse_satb_m4(int line);
extern void parse_satb_m5(int line);
extern void update_bg_pattern_cache_m4(int index);
extern void update_bg_pattern_cache_m5(int index);
extern void color_update_m4(int index, unsigned int data);
extern void color_update_m5(int index, unsigned int data);

/* Function pointers */
extern void (*render_bg)(int line, int width);
extern void (*render_obj)(int max_width);
extern void (*parse_satb)(int line);
extern void (*update_bg_pattern_cache)(int index);

#endif /* _RENDER_H_ */

