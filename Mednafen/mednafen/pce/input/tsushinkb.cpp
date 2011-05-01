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

#include "../pce.h"
#include "../input.h"
#include "tsushinkb.h"

namespace MDFN_IEN_PCE
{

class PCE_Input_TsushinKB : public PCE_Input_Device
{
 public:

 PCE_Input_TsushinKB();

 virtual void Power(int32 timestamp);
 virtual void Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR);
 virtual uint8 Read(int32 timestamp);
 virtual void Update(const void *data);
 virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);


 private:
 bool SEL, CLR;
 uint8 TsuKBState[16];
 uint8 TsuKBLatch[16 + 2 + 1];
 uint32 TsuKBIndex;
 bool last_capslock;
};

void PCE_Input_TsushinKB::Power(int32 timestamp)
{
 SEL = CLR = 0;
 memset(TsuKBState, 0, sizeof(TsuKBState));
 memset(TsuKBLatch, 0, sizeof(TsuKBLatch));
 TsuKBIndex = 0;
 last_capslock = 0;
}

PCE_Input_TsushinKB::PCE_Input_TsushinKB()
{
 Power(0);
}

void PCE_Input_TsushinKB::Update(const void *data)
{
 uint8 *data_ptr = (uint8 *)data;
 bool capslock = TsuKBState[0xE] & 0x10;
 bool new_capslock = data_ptr[0xE] & 0x10;

 if(!last_capslock && new_capslock)
  capslock ^= 1;

 for(int i = 0; i < 16; i++)
 {
  TsuKBState[i] = data_ptr[i];
 }

 TsuKBState[0xE] = (TsuKBState[0xE] & ~0x10) | (capslock ? 0x10 : 0x00);

 last_capslock = new_capslock;
}

uint8 PCE_Input_TsushinKB::Read(int32 timestamp)
{
 uint8 ret;

 ret = ((TsuKBLatch[TsuKBIndex] >> (SEL * 4)) & 0xF);

 return(ret);
}

void PCE_Input_TsushinKB::Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR)
{
 SEL = new_SEL;
 CLR = new_CLR;

 //printf("Write: %d %d %d %d\n", old_SEL, new_SEL, old_CLR, new_CLR);

 if(!old_CLR && new_CLR)
 {
  TsuKBLatch[0] = 0x02;

  for(int i = 0; i < 16; i++)
   TsuKBLatch[i + 1] = TsuKBState[i] ^ 0xFF;

  TsuKBLatch[17] = 0x02;
  TsuKBIndex = 0;
  //puts("Latched");
 }
 else if(!old_SEL && new_SEL)
 {
  TsuKBIndex = (TsuKBIndex + 1) % 18;
  if(!TsuKBIndex)
  {
   for(int i = 0; i < 16; i++)
    TsuKBLatch[i + 1] = TsuKBState[i] ^ 0xFF;
  }
 }
}

int PCE_Input_TsushinKB::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(SEL),
  SFVAR(CLR),
  SFARRAY(TsuKBState, sizeof(TsuKBState)),
  SFARRAY(TsuKBLatch, sizeof(TsuKBLatch)),
  SFVAR(TsuKBIndex),
  SFVAR(last_capslock),
  SFEND
 };
 int ret =  MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);
 
 return(ret);
}

const InputDeviceInputInfoStruct PCE_TsushinKBIDII[0x80] = 
{
 // 0 - DONE!
 { "kp_0", "Keypad 0", 0, IDIT_BUTTON },
 { "kp_1", "Keypad 1", 0, IDIT_BUTTON },
 { "kp_2", "Keypad 2", 0, IDIT_BUTTON },
 { "kp_3", "Keypad 3", 0, IDIT_BUTTON },
 { "kp_4", "Keypad 4", 0, IDIT_BUTTON },
 { "kp_5", "Keypad 5", 0, IDIT_BUTTON },
 { "kp_6", "Keypad 6", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 1 - DONE!
 { "kp_8", "Keypad 8", 0, IDIT_BUTTON },
 { "kp_9", "Keypad 9", 0, IDIT_BUTTON },
 { "kp_multiply", "Keypad *", 0, IDIT_BUTTON },
 { "kp_plus", "Keypad +", 0, IDIT_BUTTON },
 { "kp_equals", "Keypad =", 0, IDIT_BUTTON },
 { "kp_comma", "Keypad ,", 0, IDIT_BUTTON },
 { "kp_period", "Keypad .", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 2 - DONE!
 { "at", "@", 0, IDIT_BUTTON },
 { "a", "a", 0, IDIT_BUTTON },
 { "b", "b", 0, IDIT_BUTTON },
 { "c", "c", 0, IDIT_BUTTON },
 { "d", "d", 0, IDIT_BUTTON },
 { "e", "e", 0, IDIT_BUTTON },
 { "f", "f", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 3 - DONE!
 { "h", "h", 0, IDIT_BUTTON },
 { "i", "i", 0, IDIT_BUTTON },
 { "j", "j", 0, IDIT_BUTTON },
 { "k", "k", 0, IDIT_BUTTON },
 { "l", "l", 0, IDIT_BUTTON },
 { "m", "m", 0, IDIT_BUTTON },
 { "n", "n", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },
 
// 4 - DONE!
 { "p", "p", 0, IDIT_BUTTON },
 { "q", "q", 0, IDIT_BUTTON },
 { "r", "r", 0, IDIT_BUTTON },
 { "s", "s", 0, IDIT_BUTTON },
 { "t", "t", 0, IDIT_BUTTON },
 { "u", "u", 0, IDIT_BUTTON },
 { "v", "v", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 5 - DONE!
 { "x", "x", 0, IDIT_BUTTON },
 { "y", "y", 0, IDIT_BUTTON },
 { "z", "z", 0, IDIT_BUTTON },
 { "left_bracket", "[", 0, IDIT_BUTTON },
 { "yen", "Yen", 0, IDIT_BUTTON },
 { "right_bracket", "]", 0, IDIT_BUTTON },
 { "caret", "^", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 6 - DONE!
 { "0", "0", 0, IDIT_BUTTON },
 { "1", "1", 0, IDIT_BUTTON },
 { "2", "2", 0, IDIT_BUTTON },
 { "3", "3", 0, IDIT_BUTTON },
 { "4", "4", 0, IDIT_BUTTON },
 { "5", "5", 0, IDIT_BUTTON },
 { "6", "6", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 7 - DONE!
 { "8", "8", 0, IDIT_BUTTON },
 { "9", "9", 0, IDIT_BUTTON },
 { "colon", ":", 0, IDIT_BUTTON },
 { "semicolon", ";", 0, IDIT_BUTTON },
 { "comma", ",", 0, IDIT_BUTTON },
 { "period", ".", 0, IDIT_BUTTON },
 { "slash", "/", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 8 - DONE enough
 { "clear", "clear", 0, IDIT_BUTTON },
 { "up", "up", 0, IDIT_BUTTON },
 { "right", "right", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },	// Alternate backspace key on PC-88 keyboard??? { "unk80", "unknown", 0, IDIT_BUTTON },
 { "grph", "GRPH", 0, IDIT_BUTTON },
 { "kana", "カナ", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },	// Alternate shift key on PC-88 keyboard??? { "unk83", "unknown", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// 9 - DONE!
 { "stop", "STOP", 0, IDIT_BUTTON },	// Break / STOP
 { "f1", "F1", 0, IDIT_BUTTON },
 { "f2", "F2", 0, IDIT_BUTTON },
 { "f3", "F3", 0, IDIT_BUTTON },
 { "f4", "F4", 0, IDIT_BUTTON },
 { "f5", "F5", 0, IDIT_BUTTON },
 { "space", "space", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// A - DONE!
 { "tab", "Tab", 0, IDIT_BUTTON },		// Tab
 { "down", "down", 0, IDIT_BUTTON },
 { "left", "left", 0, IDIT_BUTTON },
 { "help", "Help", 0, IDIT_BUTTON },	// -624
 { "copy", "Copy", 0, IDIT_BUTTON },	// -623
 { "kp_minus", "Keypad Minus", 0, IDIT_BUTTON },
 { "kp_divide", "Keypad Divide", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

 // B - DONE(most likely)
 { "roll_down", "ROLL DOWN", 0, IDIT_BUTTON },
 { "roll_up", "ROLL UP", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownB2", "unknown", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownB3", "unknown", 0, IDIT_BUTTON },
 { "o", "o", 0, IDIT_BUTTON },
 { "underscore", "Underscore", 0, IDIT_BUTTON },
 { "g", "g", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// C - DONE!
 { "f6", "f6", 0, IDIT_BUTTON },
 { "f7", "f7", 0, IDIT_BUTTON },
 { "f8", "f8", 0, IDIT_BUTTON },
 { "f9", "f9", 0, IDIT_BUTTON },
 { "f10", "F10", 0, IDIT_BUTTON },
 { "backspace", "backspace", 0, IDIT_BUTTON },
 { "insert", "insert", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// D - DONE!
 { "convert", "変換", 0, IDIT_BUTTON },		 // (-620) Begin marking entered text, and disable marking after pressing the 
					     	 // end-of-block key.
 { "nonconvert", "決定", 0, IDIT_BUTTON }, 	 // (-619) End text marking block
 { "pc", "PC", 0, IDIT_BUTTON }, 		 // (-617) Selects between Rgana and Rkana.  SHIFT+this key switches between
					     	 // latin and kana/gana mode?
 { "width", "変換", 0, IDIT_BUTTON }, 	     	 // (-618) Chooses font width?
 { "ctrl", "CTRL/Control", 0, IDIT_BUTTON }, 	 // CTRL
 { "kp_7", "Keypad 7", 0, IDIT_BUTTON },
 { "w", "w", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// E - DONE!
 { "return", "return", 0, IDIT_BUTTON }, // enter
 { "kp_enter", "Keypad Enter", 0, IDIT_BUTTON }, // enter
 { "left_shift", "Left Shift", 0, IDIT_BUTTON }, // Left Shift
 { "right_shift", "Right Shift", 0, IDIT_BUTTON }, // Right Shift
 { "caps_lock", "Caps Lock", 0, IDIT_BUTTON }, // Caps Lock(mechanically-locking...)
 { "delete", "Delete", 0, IDIT_BUTTON },
 { "escape", "Escape", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },

// F - DONE(most likely)
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownF0", "unknown", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownF1", "unknown", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownF2", "unknown", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownF3", "unknown", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },		// { "unknownF4", "unknown", 0, IDIT_BUTTON },
 { "minus", "Minus", 0, IDIT_BUTTON },
 { "7", "7", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },
};

PCE_Input_Device *PCEINPUT_MakeTsushinKB(void)
{
 return(new PCE_Input_TsushinKB());
}

};
