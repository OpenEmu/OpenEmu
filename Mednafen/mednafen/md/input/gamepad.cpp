/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald
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

#include "../shared.h"
#include "gamepad.h"

/*--------------------------------------------------------------------------*/
/* Master System 2-button gamepad                                           */
/*--------------------------------------------------------------------------*/
class Gamepad2 : public MD_Input_Device
{
        public:
        Gamepad2();
        virtual ~Gamepad2();
        virtual void Write(uint8 data);
        virtual uint8 Read();
        virtual void Update(const void *data);

        private:
        int select;
        uint8 buttons;
};

/*--------------------------------------------------------------------------*/
/* Genesis 3-button gamepad                                                 */
/*--------------------------------------------------------------------------*/
class Gamepad3 : public MD_Input_Device
{
        public:
        Gamepad3();
        virtual ~Gamepad3();
        virtual void Write(uint8 data);
        virtual uint8 Read();
        virtual void Update(const void *data);
	virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);

	private:
	int select;
	uint8 buttons;
};

/*--------------------------------------------------------------------------*/
/* Fighting Pad 6B                                                          */
/*--------------------------------------------------------------------------*/
class Gamepad6 : public MD_Input_Device
{
        public:
        Gamepad6();
        virtual ~Gamepad6();
        virtual void Write(uint8 data);
        virtual uint8 Read();
        virtual void Update(const void *data);

        private:
	int count;
	int select;
	int timeout;
	uint16 buttons;
};

const InputDeviceInputInfoStruct Gamepad2IDII[7] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "a", "A", 5, IDIT_BUTTON_CAN_RAPID, NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "start", "Start", 4, IDIT_BUTTON, NULL },
};

Gamepad2::Gamepad2()
{

}

Gamepad2::~Gamepad2()
{

}

void Gamepad2::Write(uint8 data)
{

}

uint8 Gamepad2::Read()
{
    uint8 temp = 0x3F;
    if(buttons & INPUT_UP)    temp &= ~0x01;
    if(buttons & INPUT_DOWN)  temp &= ~0x02;
    if(buttons & INPUT_LEFT)  temp &= ~0x04;
    if(buttons & INPUT_RIGHT) temp &= ~0x08;
    if(buttons & INPUT_B)     temp &= ~0x10;
    if(buttons & INPUT_C)     temp &= ~0x20;
    return temp;
}

void Gamepad2::Update(const void *data)
{
 buttons = *(uint8 *)data;
}

const InputDeviceInputInfoStruct GamepadIDII[8] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "a", "A", 5, IDIT_BUTTON_CAN_RAPID, NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "c", "C", 7, IDIT_BUTTON_CAN_RAPID, NULL },
 { "start", "Start", 4, IDIT_BUTTON, NULL },
// { "x", "B", 7, IDIT_BUTTON_CAN_RAPID, NULL },
// { "y", "B", 7, IDIT_BUTTON_CAN_RAPID, NULL },
// { "z", "B", 7, IDIT_BUTTON_CAN_RAPID, NULL },
};

Gamepad3::Gamepad3()
{

}

Gamepad3::~Gamepad3()
{

}

void Gamepad3::Write(uint8 data)
{
    select = (data >> 6) & 1;
}

uint8 Gamepad3::Read()
{
    uint8 temp = 0x3F;

    if(select)
    {
        temp = 0x3F;
        if(buttons & INPUT_UP)    temp &= ~0x01;
        if(buttons & INPUT_DOWN)  temp &= ~0x02;
        if(buttons & INPUT_LEFT)  temp &= ~0x04;
        if(buttons & INPUT_RIGHT) temp &= ~0x08;
        if(buttons & INPUT_B)     temp &= ~0x10;
        if(buttons & INPUT_C)     temp &= ~0x20;

	//printf("Read: %d, %02x\n", select, temp);

        return temp | 0x40;
    }
    else
    {
        temp = 0x33;
        if(buttons & INPUT_UP)    temp &= ~0x01;
        if(buttons & INPUT_DOWN)  temp &= ~0x02;
        if(buttons & INPUT_A)     temp &= ~0x10;
        if(buttons & INPUT_START) temp &= ~0x20;

	//printf("Read: %d, %02x\n", select, temp);
        return temp;
    }
}

void Gamepad3::Update(const void *data)
{
 buttons = *(uint8 *)data;
}

int Gamepad3::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(buttons),
  SFVAR(select),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);

 if(load)
 {

 }
 return(ret);
}


const InputDeviceInputInfoStruct Gamepad6IDII[11] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "a", "A", 5, IDIT_BUTTON_CAN_RAPID, NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "c", "C", 7, IDIT_BUTTON_CAN_RAPID, NULL },
 { "start", "Start", 4, IDIT_BUTTON, NULL },
 { "x", "X", 8, IDIT_BUTTON_CAN_RAPID, NULL },
 { "y", "Y", 9, IDIT_BUTTON_CAN_RAPID, NULL },
 { "z", "Z", 10, IDIT_BUTTON_CAN_RAPID, NULL },
};

Gamepad6::Gamepad6()
{

}

Gamepad6::~Gamepad6()
{

}

void Gamepad6::Write(uint8 data)
{
    data = (data >> 6) & 1;

    /* Ignore cases where TH isn't changed */
    if(data == select)
        return;

    /* Update state of TH */
    select = data;

    /* Reset the timeout counter */
    timeout = 0;

    /* Bump the state counter */
    ++count;
}

uint8 Gamepad6::Read()
{
    uint8 temp = 0x3F;

    if(count <= 4 || count >= 8)
    {
        if(select)
        {
            temp = 0x3F;
            if(buttons & INPUT_UP)    temp &= ~0x01;
            if(buttons & INPUT_DOWN)  temp &= ~0x02;
            if(buttons & INPUT_LEFT)  temp &= ~0x04;
            if(buttons & INPUT_RIGHT) temp &= ~0x08;
            if(buttons & INPUT_B)     temp &= ~0x10;
            if(buttons & INPUT_C)     temp &= ~0x20;
            return temp | 0x40;
        }
        else
        {
            temp = 0x33;
            if(buttons & INPUT_UP)    temp &= ~0x01;
            if(buttons & INPUT_DOWN)  temp &= ~0x02;
            if(buttons & INPUT_A)     temp &= ~0x10;
            if(buttons & INPUT_START) temp &= ~0x20;
            return temp;
        }
    }
    else
    switch(count)
    {
        case 5:
            temp = 0x30;
            if(buttons & INPUT_A) temp &= ~0x10;
            if(buttons & INPUT_START) temp &= ~0x20;
            return temp;

        case 6:
            temp = 0x3F;
            if(buttons & INPUT_Z)     temp &= ~0x01;
            if(buttons & INPUT_Y)     temp &= ~0x02;
            if(buttons & INPUT_X)     temp &= ~0x04;
            if(buttons & INPUT_MODE)  temp &= ~0x08;
            if(buttons & INPUT_B)     temp &= ~0x10;
            if(buttons & INPUT_C)     temp &= ~0x20;
            return temp | 0x40;

        case 7:
            temp = 0x3F;
            if(buttons & INPUT_A)     temp &= ~0x10;
            if(buttons & INPUT_START) temp &= ~0x20;
            return temp;
    }

    return -1;
}

void Gamepad6::Update(const void *data)
{
 buttons = ((uint8 *)data)[0] | (((uint8 *)data)[1] << 8);
}


#if 0
/* Update the timeout counter for any Fighting Pad 6B devices */
void gen_io_update(void)
{
    int i;
    for(i = 0; i < PORT_MAX; i++)
    {
        if(port[i].device == DEVICE_MD6B)
        {
            device_md6b_t *p = &device_md6b[i];

            /* After 24 scanlines, reset the state counter */
            if(++timeout > 24)
                count = 0;
        }
    }
}
#endif

MD_Input_Device *MDInput_MakeMS2B(void)
{
 return new Gamepad2;
}

MD_Input_Device *MDInput_MakeMD3B(void)
{
 return new Gamepad3;
}

MD_Input_Device *MDInput_MakeMD6B(void)
{
 return new Gamepad6;
}

