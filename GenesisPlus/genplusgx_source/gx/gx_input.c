/****************************************************************************
 *  gx_input.c
 *
 *  Genesis Plus GX input support
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

#include "shared.h"
#include "font.h"
#include "gui.h"
#include "cheats.h"

#ifdef HW_RVL
#include <ogc/usbmouse.h>
#endif

/* Analog sticks sensitivity */
#define ANALOG_SENSITIVITY 30

/* Delay before held keys triggering */
/* higher is the value, less responsive is the key update */
#define HELD_DELAY 30

/* Direction & selection update speed when a key is being held */
/* lower is the value, faster is the key update */
#define HELD_SPEED 4


/* Configurable keys */
#define KEY_BUTTONA 0
#define KEY_BUTTONB 1
#define KEY_BUTTONC 2
#define KEY_START   3
#define KEY_BUTTONX 4
#define KEY_BUTTONY 5
#define KEY_BUTTONZ 6
#define KEY_MODE    7
#define KEY_MENU    8

#ifdef HW_RVL

#define PAD_UP    0   
#define PAD_DOWN  1
#define PAD_LEFT  2
#define PAD_RIGHT 3

/* default directions mapping  */
static u32 wpad_dirmap[3][4] =
{
  {WPAD_BUTTON_RIGHT, WPAD_BUTTON_LEFT, WPAD_BUTTON_UP, WPAD_BUTTON_DOWN},                                /* WIIMOTE */
  {WPAD_BUTTON_UP, WPAD_BUTTON_DOWN, WPAD_BUTTON_LEFT, WPAD_BUTTON_RIGHT},                                /* WIIMOTE + NUNCHUK */
  {WPAD_CLASSIC_BUTTON_UP, WPAD_CLASSIC_BUTTON_DOWN, WPAD_CLASSIC_BUTTON_LEFT, WPAD_CLASSIC_BUTTON_RIGHT} /* CLASSIC */
};

#define WPAD_BUTTONS_HELD (WPAD_BUTTON_UP | WPAD_BUTTON_DOWN | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT | \
                           WPAD_BUTTON_MINUS | WPAD_BUTTON_PLUS | WPAD_BUTTON_A | WPAD_BUTTON_2 | \
                           WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_RIGHT | \
                           WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R | WPAD_CLASSIC_BUTTON_A)

#endif

#define PAD_BUTTONS_HELD  (PAD_BUTTON_UP | PAD_BUTTON_DOWN | PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | \
                           PAD_TRIGGER_L | PAD_TRIGGER_R | PAD_BUTTON_A)

static char keyname[MAX_KEYS][16];

static int held_cnt = 0;
static int inputs_disabled = 0;

/***************************************************************************************/
/*   Gamecube PAD support                                                              */
/***************************************************************************************/
static void pad_config(int chan, int first_key, int last_key)
{
  u16 p = 0;
  char msg[64];

  /* disable background PAD scanning */
  inputs_disabled = 1;

  /* Check if PAD is connected */
  VIDEO_WaitVSync();
  if (!(PAD_ScanPads() & (1<<chan)))
  {
    /* restore inputs update callback */
    sprintf(msg, "PAD #%d is not connected !", chan+1);
    GUI_WaitPrompt("Error",msg);

    /* re-enable background PAD scanning and exit */
    inputs_disabled = 0;
    return;
  }

  /* Configure each keys */
  do
  {
    /* ignore unused keys */
    if (strcmp(keyname[first_key], "N.A"))
    {
      /* remove any pending keys */
      while (PAD_ButtonsHeld(chan))
      {
        VIDEO_WaitVSync();
        PAD_ScanPads();
      }

      /* configurable button */
      sprintf(msg,"Press key for %s\n(D-PAD to return)",keyname[first_key]);
      GUI_MsgBoxUpdate(0,msg);

      /* wait for user input */
      p = 0;
      while (!p)
      {
        VIDEO_WaitVSync();
        PAD_ScanPads();
        p = PAD_ButtonsDown(chan);
      }

      /* find pressed key */
      if (p & PAD_BUTTON_A) p = PAD_BUTTON_A;
      else if (p & PAD_BUTTON_B) p = PAD_BUTTON_B;
      else if (p & PAD_BUTTON_X) p = PAD_BUTTON_X;
      else if (p & PAD_BUTTON_Y) p = PAD_BUTTON_Y;
      else if (p & PAD_TRIGGER_R) p = PAD_TRIGGER_R;
      else if (p & PAD_TRIGGER_L) p = PAD_TRIGGER_L;
      else if (p & PAD_TRIGGER_Z) p = PAD_TRIGGER_Z;
      else if (p & PAD_BUTTON_START) p = PAD_BUTTON_START;
      else first_key = MAX_KEYS;

      /* update key mapping */
      if (first_key < MAX_KEYS)
      {
        config.pad_keymap[chan][first_key] = p;
      }
    }
  }
  while (first_key++ < last_key);

  /* remove any pending keys */
  while (PAD_ButtonsHeld(chan))
  {
    VIDEO_WaitVSync();
    PAD_ScanPads();
  }

  /* Configurable menu key */
  GUI_MsgBoxUpdate(0,"Press key(s) for MENU");

  /* reset key combo */
  config.pad_keymap[chan][KEY_MENU] = 0;

  /* wait for user input */
  p = 0;
  while (!p)
  {
    /* update PAD status */
    VIDEO_WaitVSync();
    PAD_ScanPads();
    p = PAD_ButtonsHeld(chan);
  }

  /* register keys until none are pressed anymore */
  while (p)
  {
    /* update key combo */
    config.pad_keymap[chan][KEY_MENU] |= p;

    /* update PAD status */
    VIDEO_WaitVSync();
    PAD_ScanPads();
    p = PAD_ButtonsHeld(chan);
  }

  /* re-enable background PAD scanning and exit */
  inputs_disabled = 0;
}

static void pad_update(s8 chan, u8 i)
{
  /* PAD status */
  u16 p = PAD_ButtonsHeld(chan);
  s8 x  = PAD_StickX (chan);
  s8 y  = PAD_StickY (chan);

  /* Retrieve current key mapping */
  u16 *pad_keymap = config.pad_keymap[chan];

  /* User configurable menu combo */
  if ((p & pad_keymap[KEY_MENU]) == pad_keymap[KEY_MENU])
  {
    ConfigRequested = 1;
    return;
  }

  /* Emulated device */
  switch (input.dev[i])
  {
    case DEVICE_PAD6B:
    {
      /* Extra buttons */
      if (p & pad_keymap[KEY_BUTTONX]) input.pad[i] |= INPUT_X;
      if (p & pad_keymap[KEY_BUTTONY]) input.pad[i] |= INPUT_Y;
      if (p & pad_keymap[KEY_BUTTONZ]) input.pad[i] |= INPUT_Z;
      if (p & pad_keymap[KEY_MODE])    input.pad[i] |= INPUT_MODE;

      /* default inputs are checked below */
   }

    case DEVICE_PAD3B:
    {
      /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* D-PAD */
      if ((p & PAD_BUTTON_UP) || (y > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_UP;
      else if ((p & PAD_BUTTON_DOWN) || (y < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_DOWN;
      if ((p & PAD_BUTTON_LEFT) || (x < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_LEFT;
      else if ((p & PAD_BUTTON_RIGHT) || (x > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_RIGHT;

      /* Buttons */
      if (p & pad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_A;
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_B;
      if (p & pad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_C;
      if (p & pad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_XE_A1P:
    {
      /* Left Stick analog position [0-255] */
      input.analog[i][0] = (x + 128);
      input.analog[i][1] = y ? (127 - y) : (128 - y);

      /* Right Stick analog position [0-255] */
      x = PAD_SubStickX(chan);
      y = PAD_SubStickY(chan);

      /* Emulated stick is unidirectional but can be rotated */
      if (abs(x) > abs(y))
      {
       input.analog[i+1][0] = (x + 128);
      }
      else 
      {
       input.analog[i+1][0] = (y + 128);
      }

      /* Buttons */
      if (p & pad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_XE_A;
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_XE_B;
      if (p & pad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_XE_C;
      if (p & pad_keymap[KEY_START])   input.pad[i] |= INPUT_XE_START;
      if (p & pad_keymap[KEY_BUTTONX]) input.pad[i] |= INPUT_XE_D;
      if (p & pad_keymap[KEY_BUTTONY]) input.pad[i] |= INPUT_XE_E1;
      if (p & pad_keymap[KEY_BUTTONZ]) input.pad[i] |= INPUT_XE_E2;
      if (p & pad_keymap[KEY_MODE])    input.pad[i] |= INPUT_XE_SELECT;

      break;
    }

    case DEVICE_SPORTSPAD:
    {
      /* Y analog position [0-255] */
      input.analog[i][1] = y ? (127 - y) : (128 - y);

      /* default inputs are checked below */
   }

    case DEVICE_PADDLE:
    {
      /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* X analog position [0-255] */
      input.analog[i][0] = (x + 128);

      /* Buttons */
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_BUTTON1;
      if (p & pad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_BUTTON2;
      if (p & pad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_PAD2B:
    {
      /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* D-PAD */
      if ((p & PAD_BUTTON_UP) || (y > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_UP;
      else if ((p & PAD_BUTTON_DOWN) || (y < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_DOWN;
      if ((p & PAD_BUTTON_LEFT) || (x < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_LEFT;
      else if ((p & PAD_BUTTON_RIGHT) || (x > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_RIGHT;

      /* Buttons */
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_BUTTON1;
      if (p & pad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_BUTTON2;
      if (p & pad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_LIGHTGUN:
    {
       /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* Gun screen position (x,y) */
      input.analog[i][0] += x / ANALOG_SENSITIVITY;
      input.analog[i][1] -= y / ANALOG_SENSITIVITY;

      /* Limits */
      if (input.analog[i][0] < 0) input.analog[i][0] = 0;
      else if (input.analog[i][0] > bitmap.viewport.w) input.analog[i][0] = bitmap.viewport.w;
      if (input.analog[i][1] < 0) input.analog[i][1] = 0;
      else if (input.analog[i][1] > bitmap.viewport.h) input.analog[i][1] = bitmap.viewport.h;

      /* Buttons */
      if (p & pad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_A;
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_B;
      if (p & pad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_C;
      if (p & pad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_MOUSE:
    {
      /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* Mouse relative movement (-255,255) */
      input.analog[i][0] =  (x / ANALOG_SENSITIVITY) * 2;
      input.analog[i][1] =  (y / ANALOG_SENSITIVITY) * 2;

      /* Y-Axis inversion */
      if (config.invert_mouse)
      {
        input.analog[i][1] = -input.analog[i][1];
      }

      /* Buttons */
      if (p & pad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_MOUSE_CENTER;
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_MOUSE_LEFT;
      if (p & pad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_MOUSE_RIGHT;
      if (p & pad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_PICO:
    {
      /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* D-PAD */
      if (p & PAD_BUTTON_UP) input.pad[0] |= INPUT_UP;
      else if (p & PAD_BUTTON_DOWN) input.pad[0] |= INPUT_DOWN;
      if (p & PAD_BUTTON_LEFT) input.pad[0] |= INPUT_LEFT;
      else if (p & PAD_BUTTON_RIGHT) input.pad[0] |= INPUT_RIGHT;

      /* PEN screen position (x,y) */
      input.analog[0][0] += x / ANALOG_SENSITIVITY;
      input.analog[0][1] -= y / ANALOG_SENSITIVITY;

      /* Limits */
      if (input.analog[0][0] < 0x17c) input.analog[0][0] = 0x17c;
      else if (input.analog[0][0] > 0x3c) input.analog[0][0] = 0x3c;
      if (input.analog[0][1] < 0x1fc) input.analog[0][1] = 0x1fc;
      else if (input.analog[0][1] > 0x3f3) input.analog[0][1] = 0x3f3;

      /* PEN button */
      if (p & pad_keymap[KEY_BUTTONA]) input.pad[0] |= INPUT_PICO_RED;

      /* RED button */
      if (p & pad_keymap[KEY_BUTTONB]) input.pad[0] |= INPUT_PICO_PEN;

      /* PAGE index increment */
      if (p & pad_keymap[KEY_BUTTONC]) pico_current = (pico_current + 1) & 7;

      break;
    }

    case DEVICE_TEREBI:
    {
      /* Default menu key (right analog stick) */
      if (PAD_SubStickX(chan) > ANALOG_SENSITIVITY)
      {
        ConfigRequested = 1;
        return;
      }

      /* PEN screen position (x,y) */
      input.analog[0][0] += x / ANALOG_SENSITIVITY;
      input.analog[0][1] -= y / ANALOG_SENSITIVITY;

      /* Limits */
      if (input.analog[0][0] < 0) input.analog[0][0] = 0;
      else if (input.analog[0][0] > 250) input.analog[0][0] = 250;
      if (input.analog[0][1] < 0) input.analog[0][1] = 0;
      else if (input.analog[0][1] > 250) input.analog[0][1] = 250;

      /* PEN button */
      if (p & pad_keymap[KEY_BUTTONA]) input.pad[0] |= INPUT_BUTTON1;

      break;
    }

    case DEVICE_ACTIVATOR:
    {
      /* Left & right analog stick angle [0-360] */
      float ang;

      /* Left stick values */
      if ((abs(x) > ANALOG_SENSITIVITY) || (abs(y) > ANALOG_SENSITIVITY))
      {
        /* Calculate angle (in degree) */
        ang = 90.0 - (atan((float)y / (float)x) * 180.0 / M_PI);
	      if (x < 0) ang += 180.0;

        /* 8 bottom sensors = 8 areas */
        if ((ang > 22.5) && (ang <= 67.5)) input.pad[i] |= INPUT_ACTIVATOR_2L;
        else if ((ang > 67.5) && (ang <= 112.5)) input.pad[i] |= INPUT_ACTIVATOR_3L;
        else if ((ang > 112.5) && (ang <= 157.5)) input.pad[i] |= INPUT_ACTIVATOR_4L;
        else if ((ang > 157.5) && (ang <= 202.5)) input.pad[i] |= INPUT_ACTIVATOR_5L;
        else if ((ang > 202.5) && (ang <= 247.5)) input.pad[i] |= INPUT_ACTIVATOR_6L;
        else if ((ang > 247.5) && (ang <= 292.5)) input.pad[i] |= INPUT_ACTIVATOR_7L;
        else if ((ang > 292.5) && (ang <= 337.5)) input.pad[i] |= INPUT_ACTIVATOR_8L;
        else input.pad[i] |= INPUT_ACTIVATOR_1L;
      }

      /* Right stick values */
      x = PAD_SubStickX(chan);
      y = PAD_SubStickY(chan);

      if ((abs(x) > ANALOG_SENSITIVITY) || (abs(y) > ANALOG_SENSITIVITY))
      {
        /* Calculate angle (in degree) */
        ang = 90.0 - (atan((float)y / (float)x) * 180.0 / M_PI);
	      if (x < 0) ang += 180.0;

        /* 8 top sensors = 8 areas */
        if ((ang > 22.5) && (ang <= 67.5)) input.pad[i] |= INPUT_ACTIVATOR_2U;
        else if ((ang > 67.5) && (ang <= 112.5)) input.pad[i] |= INPUT_ACTIVATOR_3U;
        else if ((ang > 112.5) && (ang <= 157.5)) input.pad[i] |= INPUT_ACTIVATOR_4U;
        else if ((ang > 157.5) && (ang <= 202.5)) input.pad[i] |= INPUT_ACTIVATOR_5U;
        else if ((ang > 202.5) && (ang <= 247.5)) input.pad[i] |= INPUT_ACTIVATOR_6U;
        else if ((ang > 247.5) && (ang <= 292.5)) input.pad[i] |= INPUT_ACTIVATOR_7U;
        else if ((ang > 292.5) && (ang <= 337.5)) input.pad[i] |= INPUT_ACTIVATOR_8U;
        else input.pad[i] |= INPUT_ACTIVATOR_1U;
      }

      break;
    }
  }
}

/***************************************************************************************/
/*   Wii WPAD support                                                                  */
/***************************************************************************************/
#ifdef HW_RVL

static int wpad_StickX(WPADData *data, u8 right)
{
  struct joystick_t* js = NULL;

  switch (data->exp.type)
  {
    case WPAD_EXP_NUNCHUK:
      js = right ? NULL : &data->exp.nunchuk.js;
      break;

    case WPAD_EXP_CLASSIC:
      js = right ? &data->exp.classic.rjs : &data->exp.classic.ljs;
      break;

    default:
      break;
  }

  if (js)
  {
    /* raw X value */
    int x = js->pos.x;
    
    /* value returned is sometime above calibrated limits */
    if (x > js->max.x) return 127;
    if (x < js->min.x) return -128;
    
    /* adjust against center position */
    x -= js->center.x;

	  /* return interpolated range [-128;127] */
    if (x > 0)
    {
      return (int)(127.0 * ((float)x / (float)(js->max.x - js->center.x)));
    }
    {
      return (int)(128.0 * ((float)x / (float)(js->center.x - js->min.x)));
    }
  }

  return 0;
}

static int wpad_StickY(WPADData *data, u8 right)
{
  struct joystick_t* js = NULL;

  switch (data->exp.type)
  {
    case WPAD_EXP_NUNCHUK:
      js = right ? NULL : &data->exp.nunchuk.js;
      break;

    case WPAD_EXP_CLASSIC:
      js = right ? &data->exp.classic.rjs : &data->exp.classic.ljs;
      break;

    default:
      break;
  }

  if (js)
  {
    /* raw Y value */
    int y = js->pos.y;
    
    /* value returned is sometime above calibrated limits */
    if (y > js->max.y) return 127;
    if (y < js->min.y) return -128;
    
    /* adjust against center position */
    y -= js->center.y;

	  /* return interpolated range [-128;127] */
    if (y > 0)
    {
      return (int)(127.0 * ((float)y / (float)(js->max.y - js->center.y)));
    }
    {
      return (int)(128.0 * ((float)y / (float)(js->center.y - js->min.y)));
    }
  }

  return 0;
}

static void wpad_config(u8 exp, int chan, int first_key, int last_key)
{
  char msg[64];
  u32 p = 255;

  /* Disable background PAD scanning */
  inputs_disabled = 1;

  /* Check if device is connected */
  WPAD_Probe(chan, &p);
  if (((exp > WPAD_EXP_NONE) && (p != exp)) || (p == 255))
  {
    /* device not detected */
    if (exp == WPAD_EXP_NONE)     sprintf(msg, "WIIMOTE #%d is not connected !", chan+1);
    if (exp == WPAD_EXP_NUNCHUK)  sprintf(msg, "NUNCHUK #%d is not connected !", chan+1);
    if (exp == WPAD_EXP_CLASSIC)  sprintf(msg, "CLASSIC #%d is not connected !", chan+1);
    GUI_WaitPrompt("Error",msg);

    /* re-enable background PAD scanning and exit */
    inputs_disabled = 0;
    return;
  }

  /* Configure each keys */
  do
  {
    /* ignore unused keys */
    if (strcmp(keyname[first_key], "N.A"))
    {
      /* remove any pending buttons */
      while (WPAD_ButtonsHeld(chan))
      {
        VIDEO_WaitVSync();
        WPAD_ScanPads();
      }

      /* configurable button */
      sprintf(msg,"Press key for %s\n(HOME to return)",keyname[first_key]);
      GUI_MsgBoxUpdate(0,msg);

      /* wait for user input */
      p = 0;
      while (!p)
      {
        VIDEO_WaitVSync();
        WPAD_ScanPads();
        p = WPAD_ButtonsDown(chan);
      }

      /* detect pressed key */
      switch (exp)
      {
        /* Wiimote (TODO: add motion sensing !) */
        case WPAD_EXP_NONE:
        {
          if (p & WPAD_BUTTON_2) p = WPAD_BUTTON_2;
          else if (p & WPAD_BUTTON_1) p = WPAD_BUTTON_1;
          else if (p & WPAD_BUTTON_B) p = WPAD_BUTTON_B;
          else if (p & WPAD_BUTTON_A) p = WPAD_BUTTON_A;
          else if (p & WPAD_BUTTON_PLUS) p = WPAD_BUTTON_PLUS;
          else if (p & WPAD_BUTTON_MINUS) p = WPAD_BUTTON_MINUS;
          else first_key = MAX_KEYS;
          break;
        }

        /* Wiimote + Nunchuk (TODO: add motion sensing !) */
        case WPAD_EXP_NUNCHUK:
        {
          if (p & WPAD_BUTTON_2) p = WPAD_BUTTON_2;
          else if (p & WPAD_BUTTON_1) p = WPAD_BUTTON_1;
          else if (p & WPAD_BUTTON_B) p = WPAD_BUTTON_B;
          else if (p & WPAD_BUTTON_A) p = WPAD_BUTTON_A;
          else if (p & WPAD_BUTTON_PLUS) p = WPAD_BUTTON_PLUS;
          else if (p & WPAD_BUTTON_MINUS) p= WPAD_BUTTON_MINUS;
          else if (p & WPAD_NUNCHUK_BUTTON_Z) p = WPAD_NUNCHUK_BUTTON_Z;
          else if (p & WPAD_NUNCHUK_BUTTON_C) p = WPAD_NUNCHUK_BUTTON_C;
          else first_key = MAX_KEYS;
          break;
        }

        /* Classic Controller */
        case WPAD_EXP_CLASSIC:
        {
          if (p & WPAD_CLASSIC_BUTTON_X) p = WPAD_CLASSIC_BUTTON_X;
          else if (p & WPAD_CLASSIC_BUTTON_A) p = WPAD_CLASSIC_BUTTON_A;
          else if (p & WPAD_CLASSIC_BUTTON_Y) p = WPAD_CLASSIC_BUTTON_Y;
          else if (p & WPAD_CLASSIC_BUTTON_B) p = WPAD_CLASSIC_BUTTON_B;
          else if (p & WPAD_CLASSIC_BUTTON_ZL) p = WPAD_CLASSIC_BUTTON_ZL;
          else if (p & WPAD_CLASSIC_BUTTON_ZR) p = WPAD_CLASSIC_BUTTON_ZR;
          else if (p & WPAD_CLASSIC_BUTTON_PLUS) p = WPAD_CLASSIC_BUTTON_PLUS;
          else if (p & WPAD_CLASSIC_BUTTON_MINUS) p = WPAD_CLASSIC_BUTTON_MINUS;
          else if (p & WPAD_CLASSIC_BUTTON_FULL_L) p = WPAD_CLASSIC_BUTTON_FULL_L;
          else if (p & WPAD_CLASSIC_BUTTON_FULL_R) p = WPAD_CLASSIC_BUTTON_FULL_R;
          else first_key = MAX_KEYS;
          break;
        }

        default:
        {
          first_key = MAX_KEYS;
          break;
        }
      }

      /* update key mapping */
      if (first_key < MAX_KEYS)
      {
        config.wpad_keymap[exp + (chan * 3)][first_key] = p;
      }
    }
  }
  while (first_key++ < last_key);

  /* remove any pending buttons */
  while (WPAD_ButtonsHeld(chan))
  {
    VIDEO_WaitVSync();
    WPAD_ScanPads();
  }

  /* re-enable background PAD scanning and exit */
  inputs_disabled = 0;
}

static void wpad_update(s8 chan, u8 i, u32 exp)
{
  /* WPAD data */
  WPADData *data = WPAD_Data(chan);

  /* WPAD status */
  u32 p = data->btns_h;

  /* Analog sticks */
  s8 x = 0;
  s8 y = 0;

  if (exp != WPAD_EXP_NONE)
  {
    x = wpad_StickX(data,0);
    y = wpad_StickY(data,0);
  }

  /* Retrieve current key mapping */
  u32 *wpad_keymap = config.wpad_keymap[exp + (chan * 3)];

  /* Emulated device */
  switch (input.dev[i])
  {
    case DEVICE_PAD6B:
    {
      /* Extra buttons */
      if (p & wpad_keymap[KEY_BUTTONX]) input.pad[i] |= INPUT_X;
      if (p & wpad_keymap[KEY_BUTTONY]) input.pad[i] |= INPUT_Y;
      if (p & wpad_keymap[KEY_BUTTONZ]) input.pad[i] |= INPUT_Z;
      if (p & wpad_keymap[KEY_MODE])    input.pad[i] |= INPUT_MODE;
    }

    case DEVICE_PAD3B:
    {
      /* D- PAD */
      if ((p & wpad_dirmap[exp][PAD_UP]) || (y > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_UP;
      else if ((p & wpad_dirmap[exp][PAD_DOWN]) || (y < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_DOWN;
      if ((p & wpad_dirmap[exp][PAD_LEFT]) || (x < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_LEFT;
      else if ((p & wpad_dirmap[exp][PAD_RIGHT]) || (x > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_RIGHT;

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_A;
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_B;
      if (p & wpad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_C;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_XE_A1P:
    {
      /* Left Stick analog position [0-255] */
      input.analog[i][0] = (x + 128);
      input.analog[i][1] = y ? (127 - y) : (128 - y);

      /* Right Stick analog position [0-255] */
      if (exp == WPAD_EXP_CLASSIC)
      {
        /* Classic Controller right stick */
        x = wpad_StickX(data,1);
        y = wpad_StickY(data,1);

        /* Emulated stick is unidirectional but can be rotated */
        if (abs(x) > abs(y))
        {
         input.analog[i+1][0] = (x + 128);
        }
        else 
        {
         input.analog[i+1][0] = (y + 128);
        }
      }
      else
      {
        /* Wiimote D-PAD */
        if ((p & wpad_dirmap[exp][PAD_DOWN]) || (p & wpad_dirmap[exp][PAD_LEFT])) input.analog[i+1][0]-=2;
        else if ((p & wpad_dirmap[exp][PAD_UP]) || (p & wpad_dirmap[exp][PAD_RIGHT])) input.analog[i+1][0]+=2;
 
        /* Limits */
        if (input.analog[i+1][0] < 0) input.analog[i+1][0] = 0;
        else if (input.analog[i+1][0] > 255) input.analog[i+1][0] = 255;
      }

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_XE_A;
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_XE_B;
      if (p & wpad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_XE_C;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_XE_START;
      if (p & wpad_keymap[KEY_BUTTONX]) input.pad[i] |= INPUT_XE_D;
      if (p & wpad_keymap[KEY_BUTTONY]) input.pad[i] |= INPUT_XE_E1;
      if (p & wpad_keymap[KEY_BUTTONZ]) input.pad[i] |= INPUT_XE_E2;
      if (p & wpad_keymap[KEY_MODE])    input.pad[i] |= INPUT_XE_SELECT;

      break;
    }

    case DEVICE_SPORTSPAD:
    {
      /* X analog position [0-255] */
      if (p & wpad_dirmap[exp][PAD_LEFT]) input.analog[i][0]-=2;
      else if (p & wpad_dirmap[exp][PAD_RIGHT]) input.analog[i][0]+=2;
      else input.analog[i][0] = (x + 128);

      /* Y analog position [0-255] */
      if (p & wpad_dirmap[exp][PAD_UP]) input.analog[i][1]-=2;
      else if (p & wpad_dirmap[exp][PAD_DOWN]) input.analog[i][1]+=2;
      else input.analog[i][1] = y ? (127 - y) : (128 - y);

      /* Limits */
      if (input.analog[i][0] < 0) input.analog[i][0] = 0;
      else if (input.analog[i][0] > 255) input.analog[i][0] = 255;
      if (input.analog[i][1] < 0) input.analog[i][1] = 0;
      else if (input.analog[i][1] > 255) input.analog[i][1] = 255;

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_BUTTON1;
      if (p & wpad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_BUTTON2;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_PADDLE:
    {
      /* X analog position [0-255] */
      if (exp == WPAD_EXP_NONE)
      {
        /* Wiimote D-PAD */
        if (p & wpad_dirmap[exp][PAD_LEFT]) input.analog[i][0]-=2;
        else if (p & wpad_dirmap[exp][PAD_RIGHT]) input.analog[i][0]+=2;

        /* Limits */
        if (input.analog[i][0] < 0) input.analog[i][0] = 0;
        else if (input.analog[i][0] > 255) input.analog[i][0] = 255;
      }
      else
      {
        /* Left analog stick */
        input.analog[i][0] = (x + 128);
      }

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_BUTTON1;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_PAD2B:
    {
      /* D-PAD */
      if ((p & wpad_dirmap[exp][PAD_UP]) || (y > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_UP;
      else if ((p & wpad_dirmap[exp][PAD_DOWN]) || (y < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_DOWN;
      if ((p & wpad_dirmap[exp][PAD_LEFT]) || (x < -ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_LEFT;
      else if ((p & wpad_dirmap[exp][PAD_RIGHT]) || (x > ANALOG_SENSITIVITY)) input.pad[i] |= INPUT_RIGHT;

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_BUTTON1;
      if (p & wpad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_BUTTON2;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_LIGHTGUN:
    {
      /* Gun screen position (x,y) */
      if (exp != WPAD_EXP_CLASSIC)
      {
        /* Wiimote IR */
        struct ir_t ir;
        WPAD_IR(chan, &ir);

        if (ir.valid)
        {
          /* screen position */
          input.analog[i][0] = (ir.x * bitmap.viewport.w) / 640;
          input.analog[i][1] = (ir.y * bitmap.viewport.h) / 480;
        }
        else
        {
          /* lightgun should point outside screen area */
          input.analog[i][0] = 512;
          input.analog[i][1] = 512;
        }
      }
      else
      {
        /* Classic Controller analog stick */
        input.analog[i][0] += x / ANALOG_SENSITIVITY;
        input.analog[i][1] -= y / ANALOG_SENSITIVITY;

        /* Limits */
        if (input.analog[i][0] < 0) input.analog[i][0] = 0;
        else if (input.analog[i][0] > bitmap.viewport.w) input.analog[i][0] = bitmap.viewport.w;
        if (input.analog[i][1] < 0) input.analog[i][1] = 0;
        else if (input.analog[i][1] > bitmap.viewport.h) input.analog[i][1] = bitmap.viewport.h;
      }

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_A;
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_B;
      if (p & wpad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_C;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_MOUSE:
    {
      /* Mouse relative movement (-255,255) */
      input.analog[i][0] = (x / ANALOG_SENSITIVITY) * 2;
      input.analog[i][1] = (y / ANALOG_SENSITIVITY) * 2;

      /* Wiimote IR (buggy) */
      if (exp != WPAD_EXP_CLASSIC)
      {
        struct ir_t ir;
        WPAD_IR(chan, &ir);

        /* Only if Wiimote is pointed to screen */
        if(ir.smooth_valid)
        {
          input.analog[i][0] = (int)((ir.sx - 512) / 2 / ANALOG_SENSITIVITY);
          input.analog[i][1] = (int)((ir.sy - 384) * 2 / 3 / ANALOG_SENSITIVITY);
        }
      }

      /* USB mouse support */
      if (MOUSE_IsConnected())
      {
        /* read mouse data */
        mouse_event event;
        MOUSE_GetEvent(&event);
        MOUSE_FlushEvents();

        /* mouse position (-127;+127) -> (-255;+255) */
        input.analog[i][0] = event.rx * 2;
        input.analog[i][1] = event.ry * 2;

        /* mouse buttons */
        if (event.button & 1) input.pad[i] |= INPUT_MOUSE_RIGHT;
        if (event.button & 2) input.pad[i] |= INPUT_MOUSE_CENTER;
        if (event.button & 4) input.pad[i] |= INPUT_MOUSE_LEFT;
      }

      /* Y-Axis inversion */
      if (config.invert_mouse)
      {
        input.analog[i][1] = -input.analog[i][1];
      }

      /* Buttons */
      if (p & wpad_keymap[KEY_BUTTONA]) input.pad[i] |= INPUT_MOUSE_CENTER;
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[i] |= INPUT_MOUSE_LEFT;
      if (p & wpad_keymap[KEY_BUTTONC]) input.pad[i] |= INPUT_MOUSE_RIGHT;
      if (p & wpad_keymap[KEY_START])   input.pad[i] |= INPUT_START;

      break;
    }

    case DEVICE_PICO:
    {
      /* D-PAD */
      if (p & PAD_BUTTON_UP) input.pad[i] |= INPUT_UP;
      else if (p & PAD_BUTTON_DOWN) input.pad[i] |= INPUT_DOWN;
      if (p & PAD_BUTTON_LEFT) input.pad[i] |= INPUT_LEFT;
      else if (p & PAD_BUTTON_RIGHT) input.pad[i] |= INPUT_RIGHT;

      /* PEN screen position (x,y) */
      input.analog[0][0] += x / ANALOG_SENSITIVITY;
      input.analog[0][1] -= y / ANALOG_SENSITIVITY;

      /* Limits */
      if (input.analog[0][0] < 0x17c) input.analog[0][0] = 0x17c;
      else if (input.analog[0][0] > 0x3c) input.analog[0][0] = 0x3c;
      if (input.analog[0][1] < 0x1fc) input.analog[0][1] = 0x1fc;
      else if (input.analog[0][1] > 0x3f3) input.analog[0][1] = 0x3f3;

      /* Wiimote IR */
      if (exp != WPAD_EXP_CLASSIC)
      {
        struct ir_t ir;
        WPAD_IR(chan, &ir);
        if (ir.valid)
        {
          input.analog[0][0] = 0x3c  + (ir.x * (0x17c - 0x3c  + 1)) / 640;
          input.analog[0][1] = 0x1fc + (ir.y * (0x3f3 - 0x1fc + 1)) / 480;
        }
      }

      /* PEN button */
      if (p & wpad_keymap[KEY_BUTTONA]) input.pad[0] |= INPUT_PICO_PEN;

      /* RED button */
      if (p & wpad_keymap[KEY_BUTTONB]) input.pad[0] |= INPUT_PICO_RED;

      /* PAGE index increment */
      if (p & wpad_keymap[KEY_BUTTONC]) pico_current = (pico_current + 1) & 7;

      break;
    }

    case DEVICE_TEREBI:
    {
      /* PEN screen position (x,y) */
      input.analog[0][0] += x / ANALOG_SENSITIVITY;
      input.analog[0][1] -= y / ANALOG_SENSITIVITY;

      /* Limits */
      if (input.analog[0][0] < 0) input.analog[0][0] = 0;
      else if (input.analog[0][0] > 250) input.analog[0][0] = 250;
      if (input.analog[0][1] < 0) input.analog[0][1] = 0;
      else if (input.analog[0][1] > 250) input.analog[0][1] = 250;

      /* Wiimote IR */
      if (exp != WPAD_EXP_CLASSIC)
      {
        struct ir_t ir;
        WPAD_IR(chan, &ir);
        if (ir.valid)
        {
          input.analog[0][0] = (ir.x * 250) / 640;
          input.analog[0][1] = (ir.y * 250) / 480;
        }
      }

      /* PEN button */
      if (p & wpad_keymap[KEY_BUTTONA]) input.pad[0] |= INPUT_BUTTON1;

      break;
    }

    case DEVICE_ACTIVATOR:
    {
      /* Classic Controller only */
      if (exp == WPAD_EXP_CLASSIC)
      {
        /* Left stick */
        float mag = data->exp.classic.ljs.mag;
        float ang = data->exp.classic.ljs.ang;

        if (mag > 0.5)
        {
          /* 8 bottom sensors = 8 areas */
          if ((ang > 22.5) && (ang <= 67.5)) input.pad[i] |= INPUT_ACTIVATOR_2L;
          else if ((ang > 67.5) && (ang <= 112.5)) input.pad[i] |= INPUT_ACTIVATOR_3L;
          else if ((ang > 112.5) && (ang <= 157.5)) input.pad[i] |= INPUT_ACTIVATOR_4L;
          else if ((ang > 157.5) && (ang <= 202.5)) input.pad[i] |= INPUT_ACTIVATOR_5L;
          else if ((ang > 202.5) && (ang <= 247.5)) input.pad[i] |= INPUT_ACTIVATOR_6L;
          else if ((ang > 247.5) && (ang <= 292.5)) input.pad[i] |= INPUT_ACTIVATOR_7L;
          else if ((ang > 292.5) && (ang <= 337.5)) input.pad[i] |= INPUT_ACTIVATOR_8L;
          else input.pad[i] |= INPUT_ACTIVATOR_1L;
        }

        /* Right stick */
        mag = data->exp.classic.rjs.mag;
        ang = data->exp.classic.rjs.ang;

        if (mag > 0.5)
        {
          /* 8 top sensors = 8 areas */
          if ((ang > 22.5) && (ang <= 67.5)) input.pad[i] |= INPUT_ACTIVATOR_2U;
          else if ((ang > 67.5) && (ang <= 112.5)) input.pad[i] |= INPUT_ACTIVATOR_3U;
          else if ((ang > 112.5) && (ang <= 157.5)) input.pad[i] |= INPUT_ACTIVATOR_4U;
          else if ((ang > 157.5) && (ang <= 202.5)) input.pad[i] |= INPUT_ACTIVATOR_5U;
          else if ((ang > 202.5) && (ang <= 247.5)) input.pad[i] |= INPUT_ACTIVATOR_6U;
          else if ((ang > 247.5) && (ang <= 292.5)) input.pad[i] |= INPUT_ACTIVATOR_7U;
          else if ((ang > 292.5) && (ang <= 337.5)) input.pad[i] |= INPUT_ACTIVATOR_8U;
          else input.pad[i] |= INPUT_ACTIVATOR_1U;
        }
      }

      break;
    }
  }
}
#endif


/***************************************************************************************/
/*   GX Input interface                                                                */
/***************************************************************************************/
void gx_input_Init(void)
{
  PAD_Init();
#ifdef HW_RVL
  WPAD_Init();
  WPAD_SetIdleTimeout(60);
  WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
  WPAD_SetVRes(WPAD_CHAN_ALL,640,480);
#endif
}

int gx_input_FindDevices(void)
{
  int i;
#ifdef HW_RVL
  u32 wpad;
#endif
  int found = 0;
  int player = 0;

  VIDEO_WaitVSync();
  u32 pad = PAD_ScanPads();

  for (i=0; i<MAX_DEVICES; i++)
  {
    /* check emulated peripheral */
    if (input.dev[i] != NO_DEVICE)
    {
      /* test input device */
      switch (config.input[player].device)
      {
        case 0: /* Gamecube Controller */
        {
          if (pad & (1 << config.input[player].port))
          {
            found++;
          }
          break;
        }

 #ifdef HW_RVL
        case 1:  /* Wiimote */
        {
          wpad = 255;
          WPAD_Probe(config.input[player].port, &wpad);
          if (wpad != 255)
          {
            found++;
          }
          break;
        }

        case 2: /* Expansion Controller */
        case 3:
        {
          wpad = 255;
          WPAD_Probe(config.input[player].port, &wpad);
          if (wpad == (config.input[player].device - 1))
          {
            found++;
          }
          break;
        }
 #endif

        default:
        {
          break;
        }
      }

      /* next configured player */
      player ++;
    }
  }

  /* return number of connected devices */
  return found;
}


void gx_input_SetDefault(void)
{
  int i,j;
  u32 exp;

  /* set default key mapping for each type of devices */
  for (i=0; i<4; i++)
  {
    config.pad_keymap[i][KEY_BUTTONA] = PAD_BUTTON_B;
    config.pad_keymap[i][KEY_BUTTONB] = PAD_BUTTON_A;
    config.pad_keymap[i][KEY_BUTTONC] = PAD_BUTTON_X;
    config.pad_keymap[i][KEY_START]   = PAD_BUTTON_START;
    config.pad_keymap[i][KEY_BUTTONX] = PAD_TRIGGER_L;
    config.pad_keymap[i][KEY_BUTTONY] = PAD_BUTTON_Y;
    config.pad_keymap[i][KEY_BUTTONZ] = PAD_TRIGGER_R;
    config.pad_keymap[i][KEY_MODE]    = PAD_TRIGGER_Z;
    config.pad_keymap[i][KEY_MENU]    = PAD_TRIGGER_Z | PAD_BUTTON_RIGHT;
  }

#ifdef HW_RVL
  for (i=0; i<4; i++)
  {
    /* Wiimote (horizontal) */
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_BUTTONA] = WPAD_BUTTON_A;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_BUTTONB] = WPAD_BUTTON_1;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_BUTTONC] = WPAD_BUTTON_2;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_START]   = WPAD_BUTTON_PLUS;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_BUTTONX] = 0;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_BUTTONY] = 0;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_BUTTONZ] = 0;
    config.wpad_keymap[i*3 + WPAD_EXP_NONE][KEY_MODE]    = 0;

    /* Wiimote + Nunchuk */
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_BUTTONA] = WPAD_NUNCHUK_BUTTON_Z;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_BUTTONB] = WPAD_BUTTON_B;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_BUTTONC] = WPAD_BUTTON_A;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_START]   = WPAD_BUTTON_PLUS;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_BUTTONX] = WPAD_NUNCHUK_BUTTON_C;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_BUTTONY] = WPAD_BUTTON_1;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_BUTTONZ] = WPAD_BUTTON_2;
    config.wpad_keymap[i*3 + WPAD_EXP_NUNCHUK][KEY_MODE]    = WPAD_BUTTON_MINUS;

    /* Classic Controller */
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_BUTTONA] = WPAD_CLASSIC_BUTTON_Y;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_BUTTONB] = WPAD_CLASSIC_BUTTON_B;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_BUTTONC] = WPAD_CLASSIC_BUTTON_A;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_START]   = WPAD_CLASSIC_BUTTON_PLUS;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_BUTTONX] = WPAD_CLASSIC_BUTTON_ZL;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_BUTTONY] = WPAD_CLASSIC_BUTTON_ZR;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_BUTTONZ] = WPAD_CLASSIC_BUTTON_X;
    config.wpad_keymap[i*3 + WPAD_EXP_CLASSIC][KEY_MODE]    = WPAD_CLASSIC_BUTTON_MINUS;
  }
#endif

  /* Default player inputs */
  for (i=0; i<MAX_INPUTS; i++)
  {
    config.input[i].device  = -1;
    config.input[i].port    = i%4;
    config.input[i].padtype = 0;
  }

#ifdef HW_RVL
  /* autodetect connected Wii Controllers */
  for (i=0; i<4; i++)
  {
    exp = 255;
    WPAD_Probe(i, &exp);

    if (exp == WPAD_EXP_CLASSIC)
    {
      /* use Classic Controller */
      config.input[i].device = 3;
      config.input[i].port = i;
    }
    else if (exp == WPAD_EXP_NUNCHUK)
    {
      /* use Wiimote + Nunchuk */
      config.input[i].device = 2;
      config.input[i].port = i;
    }
    else if (exp != 255)
    {
      /* use Wiimote by default */
      config.input[i].device = 1;
      config.input[i].port = i;
    }
    else
    {
      /* look for unused Wiimotes */
      for (j=0; j<i; j++)
      {
        /* Classic Controller is assigned, which means Wiimote is free to use */
        if (config.input[j].device == (WPAD_EXP_CLASSIC + 1))
        {
          /* assign wiimote  */
          config.input[i].device = 1;
          config.input[i].port = j;
        }
      }
    }
  }
#endif

  /* Autodetect Gamecube Controllers */
  VIDEO_WaitVSync();
  exp = PAD_ScanPads();
  for (i=0; i<4; i++)
  {
    /* check if Gamecube Controller is connected */
    if (exp & (1 << i))
    {
      for (j=0; j<MAX_INPUTS; j++)
      {
        /* look for the first unassigned player */
        if (config.input[j].device == -1)
        {
          config.input[j].device  = 0;
          config.input[j].port    = i;
          j = MAX_INPUTS;
        }
      }
    }
  }
}

void gx_input_Config(u8 chan, u8 device, u8 type)
{
  int first_key, last_key;

  /* emulated device */
  switch (type)
  {
    case DEVICE_PADDLE:
    case DEVICE_PAD2B:
    case DEVICE_SPORTSPAD:
    {
      first_key = KEY_BUTTONB;
      last_key = KEY_START;
      sprintf(keyname[KEY_BUTTONB],"Button 1");
      sprintf(keyname[KEY_BUTTONC],"Button 2");
      sprintf(keyname[KEY_START],"PAUSE Button");
      break;
    }

    case DEVICE_XE_A1P:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_MODE;
      sprintf(keyname[KEY_BUTTONA],"Button A");
      sprintf(keyname[KEY_BUTTONB],"Button B");
      sprintf(keyname[KEY_BUTTONC],"Button C");
      sprintf(keyname[KEY_START],"START Button");
      sprintf(keyname[KEY_BUTTONX],"Button D");
      sprintf(keyname[KEY_BUTTONY],"Button E1");
      sprintf(keyname[KEY_BUTTONZ],"Button E2");
      sprintf(keyname[KEY_MODE],"SELECT Button");
      break;
    }

    case DEVICE_MOUSE:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_START;
      sprintf(keyname[KEY_BUTTONA],"Middle Button");
      sprintf(keyname[KEY_BUTTONB],"Left Button");
      sprintf(keyname[KEY_BUTTONC],"Right Button");
      sprintf(keyname[KEY_START],"START Button");
      break;
    }

    case DEVICE_PAD3B:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_START;
      sprintf(keyname[KEY_BUTTONA],"Button A");
      sprintf(keyname[KEY_BUTTONB],"Button B");
      sprintf(keyname[KEY_BUTTONC],"Button C");
      sprintf(keyname[KEY_START],"START Button");
      break;
    }

    case DEVICE_PAD6B:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_MODE;
      sprintf(keyname[KEY_BUTTONA],"Button A");
      sprintf(keyname[KEY_BUTTONB],"Button B");
      sprintf(keyname[KEY_BUTTONC],"Button C");
      sprintf(keyname[KEY_START],"START Button");
      sprintf(keyname[KEY_BUTTONX],"Button X");
      sprintf(keyname[KEY_BUTTONY],"Button Y");
      sprintf(keyname[KEY_BUTTONZ],"Button Z");
      sprintf(keyname[KEY_MODE],"MODE Button");
      break;
    }

    case DEVICE_LIGHTGUN:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_START;
      if (input.system[1] == SYSTEM_MENACER)
      {
        sprintf(keyname[KEY_BUTTONA],"TRIGGER Button");
        sprintf(keyname[KEY_BUTTONB],"Button B");
        sprintf(keyname[KEY_BUTTONC],"Button C");
        sprintf(keyname[KEY_START],"START Button");
      }
      else if (input.system[1] == SYSTEM_JUSTIFIER)
      {
        sprintf(keyname[KEY_BUTTONA],"TRIGGER Button");
        sprintf(keyname[KEY_BUTTONB],"N.A");
        sprintf(keyname[KEY_BUTTONC],"N.A");
        sprintf(keyname[KEY_START],"START Button");
      }
      else
      {
        sprintf(keyname[KEY_BUTTONA],"TRIGGER Button");
        sprintf(keyname[KEY_BUTTONB],"N.A");
        sprintf(keyname[KEY_BUTTONC],"N.A");
        sprintf(keyname[KEY_START],"PAUSE Button");
      }
      break;
    }

    case DEVICE_PICO:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_BUTTONB;
      sprintf(keyname[KEY_BUTTONA],"PEN Button");
      sprintf(keyname[KEY_BUTTONB],"RED Button");
      break;
    }

    case DEVICE_TEREBI:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_BUTTONA;
      sprintf(keyname[KEY_BUTTONA],"PEN Button");
      break;
    }

    default:
    {
      first_key = KEY_BUTTONA;
      last_key = KEY_BUTTONA;
      sprintf(keyname[KEY_BUTTONA],"N.A");
      GUI_WaitPrompt("Info","Activator is not configurable !");
      break;
    }
  }

  /* Input device */
  switch (device)
  {
    case 0:
    {
      pad_config(chan, first_key, last_key);
      break;
    }

    default:
    {
#ifdef HW_RVL
      wpad_config(device - 1, chan, first_key, last_key);
#endif
      break;
    }
  }
}

void gx_input_UpdateEmu(void)
{
  /* Update GC controllers status */
  PAD_ScanPads();

#ifdef HW_RVL
  /* Update Wii controllers status */
  WPAD_ScanPads();

  /* Hard-coded menu key */
  u32 p = WPAD_ButtonsHeld(0);
  if ((p & WPAD_BUTTON_HOME) || (p & WPAD_CLASSIC_BUTTON_HOME))
  {
    ConfigRequested = 1;
    return;
  }
#endif

  int i, player = 0;
  for (i=0; i<MAX_DEVICES; i++)
  {
    /* update inputs */
    if (input.dev[i] != NO_DEVICE)
    {
      /* clear key status */
      input.pad[i] = 0;

      if (config.input[player].device == 0)
      {
        pad_update(config.input[player].port, i);
      }
#ifdef HW_RVL
      else if (config.input[player].device > 0)
      {
        wpad_update(config.input[player].port, i, config.input[player].device - 1);
      }
#endif

      /* increment player index */
      player ++;
    }
  }

  /* Update RAM patches */
  RAMCheatUpdate();
}

/* Menu inputs update function */
void gx_input_UpdateMenu(void)
{
  /* Check if inputs update are disabled */
  if (inputs_disabled) return;

  /* PAD status update */
  PAD_ScanPads();

  /* PAD pressed keys */
  s16 pp = PAD_ButtonsDown(0);

  /* PAD held keys (direction/selection) */
  s16 hp = PAD_ButtonsHeld(0) & PAD_BUTTONS_HELD;

  /* PAD analog sticks (handled as PAD held direction keys) */
  s8 x  = PAD_StickX(0);
  s8 y  = PAD_StickY(0);
  if (x > ANALOG_SENSITIVITY)       hp |= PAD_BUTTON_RIGHT;
  else if (x < -ANALOG_SENSITIVITY) hp |= PAD_BUTTON_LEFT;
  else if (y > ANALOG_SENSITIVITY)  hp |= PAD_BUTTON_UP;
  else if (y < -ANALOG_SENSITIVITY) hp |= PAD_BUTTON_DOWN;

#ifdef HW_RVL
  /* WPAD status update */
  WPAD_ScanPads();
  WPADData *data = WPAD_Data(0);

  /* WPAD pressed keys */
  u32 pw = data->btns_d;

  /* WPAD held keys (direction/selection) */
  u32 hw = data->btns_h & WPAD_BUTTONS_HELD;

  /* WPAD analog sticks (handled as PAD held direction keys) */
  x = wpad_StickX(data, 0);
  y = wpad_StickY(data, 0);
  if (x > ANALOG_SENSITIVITY)       hp |= PAD_BUTTON_RIGHT;
  else if (x < -ANALOG_SENSITIVITY) hp |= PAD_BUTTON_LEFT;
  else if (y > ANALOG_SENSITIVITY)  hp |= PAD_BUTTON_UP;
  else if (y < -ANALOG_SENSITIVITY) hp |= PAD_BUTTON_DOWN;
#endif

  /* check if any direction/selection key is being held or just being pressed/released */
#ifdef HW_RVL
  if (pp||pw) held_cnt = 0;
  else if (hp||hw) held_cnt++;
  else held_cnt = 0;
#else
  if (pp) held_cnt = 0;
  else if (hp) held_cnt++;
  else held_cnt = 0;
#endif

  /* initial delay (prevents triggering to start immediately) */
  if (held_cnt > HELD_DELAY)
  {
    /* key triggering */
    pp |= hp;
#ifdef HW_RVL
    pw |= hw;
#endif

    /* delay until next triggering (adjusts direction/selection update speed) */
    held_cnt -= HELD_SPEED;
  }

#ifdef HW_RVL
  /* Wiimote direction keys */
  WPAD_IR(0, &m_input.ir);
  if (m_input.ir.valid)
  {
    /* Wiimote is handled vertically */
    if (pw & WPAD_BUTTON_UP)         pp |= PAD_BUTTON_UP;
    else if (pw & WPAD_BUTTON_DOWN)  pp |= PAD_BUTTON_DOWN;
    else if (pw & WPAD_BUTTON_LEFT)  pp |= PAD_BUTTON_LEFT;
    else if (pw & WPAD_BUTTON_RIGHT) pp |= PAD_BUTTON_RIGHT;
  }
  else
  {
    /* Wiimote is handled horizontally */
    if (pw & WPAD_BUTTON_UP)         pp |= PAD_BUTTON_LEFT;
    else if (pw & WPAD_BUTTON_DOWN)  pp |= PAD_BUTTON_RIGHT;
    else if (pw & WPAD_BUTTON_LEFT)  pp |= PAD_BUTTON_DOWN;
    else if (pw & WPAD_BUTTON_RIGHT) pp |= PAD_BUTTON_UP;
  }

  /* Classic Controller direction keys */
  if (pw & WPAD_CLASSIC_BUTTON_UP)          pp |= PAD_BUTTON_UP;
  else if (pw & WPAD_CLASSIC_BUTTON_DOWN)   pp |= PAD_BUTTON_DOWN;
  else if (pw & WPAD_CLASSIC_BUTTON_LEFT)   pp |= PAD_BUTTON_LEFT;
  else if (pw & WPAD_CLASSIC_BUTTON_RIGHT)  pp |= PAD_BUTTON_RIGHT;

  /* WPAD buttons */
  if (pw & WPAD_BUTTON_A)               pp |= PAD_BUTTON_A;
  if (pw & WPAD_BUTTON_B)               pp |= PAD_BUTTON_B;
  if (pw & WPAD_BUTTON_2)               pp |= PAD_BUTTON_A;
  if (pw & WPAD_BUTTON_1)               pp |= PAD_BUTTON_B;
  if (pw & WPAD_BUTTON_HOME)            pp |= PAD_TRIGGER_Z;
  if (pw & WPAD_BUTTON_PLUS)            pp |= PAD_TRIGGER_L;
  if (pw & WPAD_BUTTON_MINUS)           pp |= PAD_TRIGGER_R;
  if (pw & WPAD_CLASSIC_BUTTON_FULL_L)  pp |= PAD_TRIGGER_L;
  if (pw & WPAD_CLASSIC_BUTTON_FULL_R)  pp |= PAD_TRIGGER_R;
  if (pw & WPAD_CLASSIC_BUTTON_A)       pp |= PAD_BUTTON_A;
  if (pw & WPAD_CLASSIC_BUTTON_B)       pp |= PAD_BUTTON_B;
  if (pw & WPAD_CLASSIC_BUTTON_HOME)    pp |= PAD_TRIGGER_Z;
#endif

  /* Update menu inputs */
  m_input.keys = pp;
}
