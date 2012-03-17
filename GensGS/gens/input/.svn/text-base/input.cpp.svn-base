/**
 * Gens: Input base class.
 */


#include <string.h>

#include "input.hpp"
#include "emulator/g_input.hpp"
#include "gens_core/io/io.h"


Input::Input()
{
}


Input::~Input()
{
}


#define CHECK_BUTTON(player, ctrl, button, mask)		\
	if (checkKeyPressed(m_keyMap[player].button)) 		\
		Controller_ ## ctrl ## _Buttons &= ~mask;	\
	else Controller_ ## ctrl ## _Buttons |= mask;

#define CHECK_DIR(player, ctrl)						\
	if (checkKeyPressed(m_keyMap[player].Up)) 			\
	{					   			\
		Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_UP;	\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_DOWN;	\
	}								\
	else								\
	{								\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_UP;	\
		CHECK_BUTTON(player, ctrl, Down, CONTROLLER_DOWN)	\
	}								\
	if (checkKeyPressed(m_keyMap[player].Left))			\
	{					   			\
		Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_LEFT;	\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_RIGHT;	\
	}								\
	else								\
	{								\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_LEFT;	\
		CHECK_BUTTON(player, ctrl, Right, CONTROLLER_RIGHT)	\
	}

#define CHECK_ALL_BUTTONS(player, ctrl)					\
	CHECK_BUTTON(player, ctrl, Start, CONTROLLER_START)		\
	CHECK_BUTTON(player, ctrl, A, CONTROLLER_A)			\
	CHECK_BUTTON(player, ctrl, B, CONTROLLER_B)			\
	CHECK_BUTTON(player, ctrl, C, CONTROLLER_C)			\
									\
	if (Controller_ ## ctrl ## _Type & 1)				\
	{								\
		CHECK_BUTTON(player, ctrl, Mode, CONTROLLER_MODE)	\
		CHECK_BUTTON(player, ctrl, X, CONTROLLER_X)		\
		CHECK_BUTTON(player, ctrl, Y, CONTROLLER_Y)		\
		CHECK_BUTTON(player, ctrl, Z, CONTROLLER_Z)		\
	}

#define CHECK_PLAYER_PAD(player, ctrl)					\
	CHECK_DIR(player, ctrl)						\
	CHECK_ALL_BUTTONS(player, ctrl)

/**
 * updateControllers(): Update the controller bitfields.
 */
void Input::updateControllers(void)
{
	CHECK_PLAYER_PAD(0, 1);
	CHECK_PLAYER_PAD(1, 2);
	
	if (Controller_1_Type & 0x10)
	{
		// TEAMPLAYER PORT 1
		CHECK_PLAYER_PAD(2, 1B);
		CHECK_PLAYER_PAD(3, 1C);
		CHECK_PLAYER_PAD(4, 1D);
	}
	
	if (Controller_2_Type & 0x10)
	{
		// TEAMPLAYER PORT 2
		CHECK_PLAYER_PAD(5, 2B);
		CHECK_PLAYER_PAD(6, 2C);
		CHECK_PLAYER_PAD(7, 2D);
	}
}
