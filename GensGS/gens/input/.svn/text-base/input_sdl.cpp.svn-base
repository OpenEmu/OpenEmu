/**
 * Gens: Input class - SDL
 */


#include "input_sdl.hpp"
#include "input_sdl_keys.h"
#include "gdk/gdkkeysyms.h"

#include "emulator/g_main.hpp"
#include "emulator/g_input.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.hpp"

// Needed to handle controller input configuration
#include "controller_config/controller_config_window.hpp"
#include "controller_config/controller_config_window_misc.hpp"

const struct KeyMap keyDefault[8] =
{
	// Player 1
	{GENS_KEY_RETURN, GENS_KEY_RSHIFT,
	GENS_KEY_a, GENS_KEY_s, GENS_KEY_d,
	GENS_KEY_z, GENS_KEY_x, GENS_KEY_c,
	GENS_KEY_UP, GENS_KEY_DOWN, GENS_KEY_LEFT, GENS_KEY_RIGHT},
	
	// Players 1B, 1C, 1D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	// Player 2
	{GENS_KEY_u, GENS_KEY_t,
	GENS_KEY_k, GENS_KEY_l, GENS_KEY_m,
	GENS_KEY_i, GENS_KEY_o, GENS_KEY_p,
	GENS_KEY_y, GENS_KEY_h, GENS_KEY_h, GENS_KEY_j},
	
	// Players 2B, 2C, 2D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


static int gdk_to_sdl_keyval(int gdk_key);


// Axis values.
static const unsigned char JoyAxisValues[2][6] =
{
	// axis value < -10,000
	{0x03, 0x01, 0x07, 0x05, 0x0B, 0x09},
	
	// axis value > 10,000
	{0x04, 0x02, 0x08, 0x06, 0x0C, 0x0A},
};


Input_SDL::Input_SDL()
{
	// Initialize m_keys and m_joyState.
	memset(m_keys, 0x00, sizeof(m_keys));
	memset(m_joyState, 0x00, sizeof(m_joyState));
	
	// Install the GTK+ key snooper.
	gtk_key_snooper_install(GDK_KeySnoop, this);
	
	// Initialize joysticks.
	m_numJoysticks = 0;
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		// Error initializing SDL.
		fprintf(stderr, "%s: Error initializing SDL's joystick handler: %s\n", __func__, SDL_GetError());
		return;
	}
	
	// If any joysticks are connected, set them up.
	// TODO: Increase number of joysticks from 6?
	if (SDL_NumJoysticks() > 0)
	{
		SDL_JoystickEventState(SDL_ENABLE);
		
		for (int i = 0; i < 6; i++)
		{
			m_joy[i] = SDL_JoystickOpen(i);
			if (m_joy[i])
				m_numJoysticks++;
		}
	}
}


Input_SDL::~Input_SDL()
{
	// If any joysticks were opened, close them.
	for (int i = 0; i < 6; i++)
	{
		if (SDL_JoystickOpened(i))
		{
			SDL_JoystickClose(m_joy[i]);
			m_joy[i] = NULL;
		}
	}
}


/**
 * GDK_KeySnoop(): Keysnooping callback event for GTK+/GDK.
 * @param grab_widget Widget this key was snooped from.
 * @param event Event information.
 * @param func_data User data.
 * @return TRUE to stop processing this event; FALSE to allow GTK+ to process this event.
 */
gint Input_SDL::GDK_KeySnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data)
{
	SDL_Event sdlev;
	
	// Only grab keys from the Gens window. (or controller config window)
	if (grab != gens_window && grab != controller_config_window)
		return FALSE;
	
	switch(event->type)
	{
		case GDK_KEY_PRESS:
			sdlev.type = SDL_KEYDOWN;
			sdlev.key.state = SDL_PRESSED;
			break;
		case GDK_KEY_RELEASE:
			sdlev.type = SDL_KEYUP;
			sdlev.key.state = SDL_RELEASED;
			break;
		default:
			fputs("Can't happen: keysnoop got a bad type\n", stderr);
			return 0;
	}
	
	// Convert this keypress from GDK to SDL.
	// TODO: Use GENS key defines instead.
	sdlev.key.keysym.sym = (SDLKey)gdk_to_sdl_keyval(event->keyval);
	if (sdlev.key.keysym.sym != -1)
		SDL_PushEvent(&sdlev);
	
	if (grab == controller_config_window && CC_Configuring)
	{
		// Configuring controllers. Don't allow GTK+ to handle this keypress.
		return TRUE;
	}
	
	return FALSE;
}


/**
 * joyExists(): Check if the specified joystick exists.
 * @param joyNum Joystick number.
 * @return true if the joystick exists; false if it does not exist.
 */
bool Input_SDL::joyExists(int joyNum)
{
	if (joyNum < 0 || joyNum >= 6)
		return false;
	
	if (m_joy[joyNum])
		return true;
	
	// Joystick does not exist.
	return false;
}


/**
 * getKey(): Get a key. (Used for controller configuration.)
 * @return Key value.
 */
unsigned int Input_SDL::getKey(void)
{
	// TODO: Optimize this function.
	GdkEvent *event;
	SDL_Event sdl_event;
	SDL_Joystick *js[6];
	SDL_JoystickEventState(SDL_ENABLE);
	
	// Open all 6 joysticks.
	for (int i = 0; i < 6; i++)
	{
		js[i] = SDL_JoystickOpen(i);
	}
	
	// Update the UI.
	GensUI::update();
	
	while (true)
	{
		while (SDL_PollEvent (&sdl_event))
		{
			switch (sdl_event.type)
			{
				case SDL_JOYAXISMOTION:
					if (/*sdl_event.jaxis.axis < 0 ||*/ sdl_event.jaxis.axis >= 6)
						break;
					
					if (sdl_event.jaxis.value < -10000)
					{
						return (0x1000 + (0x100 * sdl_event.jaxis.which) +
							JoyAxisValues[0][sdl_event.jaxis.axis]);
					}
					else if (sdl_event.jaxis.value > 10000)
					{
						return (0x1000 + (0x100 * sdl_event.jaxis.which) +
							JoyAxisValues[1][sdl_event.jaxis.axis]);
					}
					else
					{
						// FIXME: WTF is this for?!
						return getKey();
					}
					break;
				
				case SDL_JOYBUTTONUP:
					return (0x1010 + (0x100 * sdl_event.jbutton.which) + sdl_event.jbutton.button);
					break;
				
				//case SDL_JOYHATMOTION:
				//	return (0xdeadbeef + (0x100 * sdl_event.jhat.which) + sdl_event.jhat.hat + sdl_event.jhat.value);
			}
		}
		
		// Check if a GDK key press occurred.
		event = gdk_event_get();
		if (event && event->type == GDK_KEY_PRESS)
			return gdk_to_sdl_keyval(event->key.keyval);
	}
}


/**
 * update(): Update the input subsystem.
 */
void Input_SDL::update(void)
{
	// Check for SDL events
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				close_gens();
				return;
			
			/* TODO: SDL_VIDEORESIZE should work in GL mode.
			case SDL_VIDEORESIZE:
				surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
				break;
			*/
			
			case SDL_KEYDOWN:
				m_keys[event.key.keysym.sym] = true;
				Input_KeyDown(event.key.keysym.sym);
				break;
				
			case SDL_KEYUP:
				m_keys[event.key.keysym.sym] = false;
				Input_KeyUp(event.key.keysym.sym);
				break;
			
			case SDL_JOYAXISMOTION:
				checkJoystickAxis(&event);
				break;
			
			case SDL_JOYBUTTONDOWN:
				m_joyState[0x10 + (0x100 * event.jbutton.which) + event.jbutton.button] = true;
				break;
			
			case SDL_JOYBUTTONUP:
				m_joyState[0x10 + (0x100 * event.jbutton.which) + event.jbutton.button] = false;
				break;
			
			case SDL_JOYHATMOTION:
				break;
			
			default:
				break;
		}
	}
}


/**
 * checkJoystickAxis: Check the SDL_Event for a joystick axis event.
 * @param event Pointer to SDL_Event.
 */
void Input_SDL::checkJoystickAxis(SDL_Event *event)
{
	if (event->jaxis.axis >= 6)
	{
		// Gens doesn't support more than 6 axes.
		// TODO: Fix this sometime.
		return;
	}
	
	if (event->jaxis.value < -10000)
	{
		m_joyState[(0x100 * event->jaxis.which) +
			   JoyAxisValues[0][event->jaxis.axis]] = true;
		m_joyState[(0x100 * event->jaxis.which) +
			   JoyAxisValues[1][event->jaxis.axis]] = false;
	}
	else if (event->jaxis.value > 10000)
	{
		m_joyState[(0x100 * event->jaxis.which) +
			   JoyAxisValues[0][event->jaxis.axis]] = false;
		m_joyState[(0x100 * event->jaxis.which) +
			   JoyAxisValues[1][event->jaxis.axis]] = true;
	}
	else
	{
		m_joyState[(0x100 * event->jaxis.which) +
			   JoyAxisValues[0][event->jaxis.axis]] = false;
		m_joyState[(0x100 * event->jaxis.which) +
			   JoyAxisValues[1][event->jaxis.axis]] = false;
	}
}


/**
 * checkKeyPressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return True if the key is pressed.
 */
bool Input_SDL::checkKeyPressed(unsigned int key)
{
	// If the key value is <1024, it's a keyboard key.
	if (key < 1024)
		return m_keys[key];
	
	// Joystick "key" check.
	
	// Determine which joystick we're looking for.
	int joyNum = ((key >> 8) & 0xF);
	
	// Check that this joystick exists.
	if (!joyExists(joyNum))
		return false;
	
	// Joystick exists. Check the state.
	if (key & 0x80)
	{
		// Joystick POV
		// TODO: This doesn't seem to be implemented in Gens/Linux for some reason...
#if 0
		switch (key & 0xF)
		{
			case 1:
					//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 0) return(1); break;
					//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 0) return(1); break;
			case 2:
					//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
					//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
			case 3:
					//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
					//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
			case 4:
					//if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
					//if (joystate[0x100*Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
			default:
			break;
		}
#endif
	}
	else if (key & 0x70)
	{
		// Joystick buttons
		if (m_joyState[0x10 + (0x100 * joyNum) + ((key & 0xFF) - 0x10)])
			return true;
	}
	else
	{
		// Joystick axes
		if (((key & 0xF) >= 1) && ((key & 0xF) <= 12))
		{
			if (m_joyState[(0x100 * joyNum) + (key & 0xF)])
				return true;
		}
	}
	
	// Key is not pressed.
	return false;
}


/**
 * gdk_to_sdl_keyval(): Converts a GDK key value to a Gens key value.
 * @param gdk_key GDK key value.
 * @return Gens key value.
 */
static int gdk_to_sdl_keyval(int gdk_key)
{
	switch (gdk_key)
	{
		case GDK_BackSpace:
			return GENS_KEY_BACKSPACE;
		case GDK_Tab:
			return GENS_KEY_TAB;
		case GDK_Clear:
			return GENS_KEY_CLEAR;
		case GDK_Return:
			return GENS_KEY_RETURN;
		case GDK_Pause:
			return GENS_KEY_PAUSE;
		case GDK_Escape:
			return GENS_KEY_ESCAPE;
		case GDK_KP_Space:
			return GENS_KEY_SPACE;
		case GDK_exclamdown:
			return GENS_KEY_EXCLAIM;
		case GDK_quotedbl:
			return GENS_KEY_QUOTEDBL;
		case GDK_numbersign:
			return GENS_KEY_HASH;
		case GDK_dollar:
			return GENS_KEY_DOLLAR;
		case GDK_ampersand:
			return GENS_KEY_AMPERSAND;
		case GDK_quoteright:
			return GENS_KEY_QUOTE;
		case GDK_parenleft:
			return GENS_KEY_LEFTPAREN;
		case GDK_parenright:
			return GENS_KEY_RIGHTPAREN;
		case GDK_asterisk:
			return GENS_KEY_ASTERISK;
		case GDK_plus:
			return GENS_KEY_PLUS;
		case GDK_comma:
			return GENS_KEY_COMMA;
		case GDK_minus:
			return GENS_KEY_MINUS;
		case GDK_period:
			return GENS_KEY_PERIOD;
		case GDK_slash:
			return GENS_KEY_SLASH;
		case GDK_0:
			return GENS_KEY_0;
		case GDK_1:
			return GENS_KEY_1;
		case GDK_2:
			return GENS_KEY_2;
		case GDK_3:
			return GENS_KEY_3;
		case GDK_4:
			return GENS_KEY_4;
		case GDK_5:
			return GENS_KEY_5;
		case GDK_6:
			return GENS_KEY_6;
		case GDK_7:
			return GENS_KEY_7;
		case GDK_8:
			return GENS_KEY_8;
		case GDK_9:
			return GENS_KEY_9;
		case GDK_colon:
			return GENS_KEY_COLON;
		case GDK_semicolon:
			return GENS_KEY_SEMICOLON;
		case GDK_less:
			return GENS_KEY_LESS;
		case GDK_equal:
			return GENS_KEY_EQUALS;
		case GDK_greater:
			return GENS_KEY_GREATER;
		case GDK_question:
			return GENS_KEY_QUESTION;
		case GDK_at:
			return GENS_KEY_AT;
		case GDK_bracketleft:
			return GENS_KEY_LEFTBRACKET;
		case GDK_backslash:
			return GENS_KEY_BACKSLASH;
		case GDK_bracketright:
			return GENS_KEY_RIGHTBRACKET;
		case GDK_asciicircum:
			return GENS_KEY_CARET;
		case GDK_underscore:
			return GENS_KEY_UNDERSCORE;
		case GDK_quoteleft:
			return GENS_KEY_BACKQUOTE;
		case GDK_a:
			return GENS_KEY_a;
		case GDK_b:
			return GENS_KEY_b;
		case GDK_c:
			return GENS_KEY_c;
		case GDK_d:
			return GENS_KEY_d;
		case GDK_e:
			return GENS_KEY_e;
		case GDK_f:
			return GENS_KEY_f;
		case GDK_g:
			return GENS_KEY_g;
		case GDK_h:
			return GENS_KEY_h;
		case GDK_i:
			return GENS_KEY_i;
		case GDK_j:
			return GENS_KEY_j;
		case GDK_k:
			return GENS_KEY_k;
		case GDK_l:
			return GENS_KEY_l;
		case GDK_m:
			return GENS_KEY_m;
		case GDK_n:
			return GENS_KEY_n;
		case GDK_o:
			return GENS_KEY_o;
		case GDK_p:
			return GENS_KEY_p;
		case GDK_q:
			return GENS_KEY_q;
		case GDK_r:
			return GENS_KEY_r;
		case GDK_s:
			return GENS_KEY_s;
		case GDK_t:
			return GENS_KEY_t;
		case GDK_u:
			return GENS_KEY_u;
		case GDK_v:
			return GENS_KEY_v;
		case GDK_w:
			return GENS_KEY_w;
		case GDK_x:
			return GENS_KEY_x;
		case GDK_y:
			return GENS_KEY_y;
		case GDK_z:
			return GENS_KEY_z;
		case GDK_Delete:
			return GENS_KEY_DELETE;
		case GDK_KP_0:
			return GENS_KEY_NUM_0;
		case GDK_KP_1:
			return GENS_KEY_NUM_1;
		case GDK_KP_2:
			return GENS_KEY_NUM_2;
		case GDK_KP_3:
			return GENS_KEY_NUM_3;
		case GDK_KP_4:
			return GENS_KEY_NUM_4;
		case GDK_KP_5:
			return GENS_KEY_NUM_5;
		case GDK_KP_6:
			return GENS_KEY_NUM_6;
		case GDK_KP_7:
			return GENS_KEY_NUM_7;
		case GDK_KP_8:
			return GENS_KEY_NUM_8;
		case GDK_KP_9:
			return GENS_KEY_NUM_9;
		case GDK_KP_Decimal:
			return GENS_KEY_NUM_PERIOD;
		case GDK_KP_Divide:
			return GENS_KEY_NUM_DIVIDE;
		case GDK_KP_Multiply:
			return GENS_KEY_NUM_MULTIPLY;
		case GDK_KP_Subtract:
			return GENS_KEY_NUM_MINUS;
		case GDK_KP_Add:
			return GENS_KEY_NUM_PLUS;
		case GDK_KP_Enter:
			return GENS_KEY_NUM_ENTER;
		case GDK_KP_Equal:
			return GENS_KEY_NUM_EQUALS;
		case GDK_Up:
			return GENS_KEY_UP;
		case GDK_Down:
			return GENS_KEY_DOWN;
		case GDK_Right:
			return GENS_KEY_RIGHT;
		case GDK_Left:
			return GENS_KEY_LEFT;
		case GDK_Insert:
			return GENS_KEY_INSERT;
		case GDK_Home:
			return GENS_KEY_HOME;
		case GDK_End:
			return GENS_KEY_END;
		case GDK_Page_Up:
			return GENS_KEY_PAGEUP;
		case GDK_Page_Down:
			return GENS_KEY_PAGEDOWN;
		case GDK_F1:
			return GENS_KEY_F1;
		case GDK_F2:
			return GENS_KEY_F2;
		case GDK_F3:
			return GENS_KEY_F3;
		case GDK_F4:
			return GENS_KEY_F4;
		case GDK_F5:
			return GENS_KEY_F5;
		case GDK_F6:
			return GENS_KEY_F6;
		case GDK_F7:
			return GENS_KEY_F7;
		case GDK_F8:
			return GENS_KEY_F8;
		case GDK_F9:
			return GENS_KEY_F9;
		case GDK_F10:
			return GENS_KEY_F10;
		case GDK_F11:
			return GENS_KEY_F11;
		case GDK_F12:
			return GENS_KEY_F12;
		case GDK_F13:
			return GENS_KEY_F13;
		case GDK_F14:
			return GENS_KEY_F14;
		case GDK_F15:
			return GENS_KEY_F15;
		case GDK_Num_Lock:
			return GENS_KEY_NUMLOCK;
		case GDK_Caps_Lock:
			return GENS_KEY_CAPSLOCK;
		case GDK_Scroll_Lock:
			return GENS_KEY_SCROLLOCK;
		case GDK_Shift_R:
			return GENS_KEY_RSHIFT;
		case GDK_Shift_L:
			return GENS_KEY_LSHIFT;
		case GDK_Control_R:
			return GENS_KEY_RCTRL;
		case GDK_Control_L:
			return GENS_KEY_LCTRL;
		case GDK_Alt_R:
			return GENS_KEY_RALT;
		case GDK_Alt_L:
			return GENS_KEY_LALT;
		case GDK_Meta_R:
			return GENS_KEY_RMETA;
		case GDK_Meta_L:
			return GENS_KEY_LMETA;
		case GDK_Super_L:
			return GENS_KEY_LSUPER;
		case GDK_Super_R:
			return GENS_KEY_RSUPER;
		case GDK_Mode_switch:
			return GENS_KEY_MODE;
		//case GDK_:
		//	return    GENS_KEY_COMPOSE;
		case GDK_Help:
			return GENS_KEY_HELP;
		case GDK_Print:
			return GENS_KEY_PRINT;
		case GDK_Sys_Req:
			return GENS_KEY_SYSREQ;
		case GDK_Break:
			return GENS_KEY_BREAK;
		case GDK_Menu:
			return GENS_KEY_MENU;
		//case GDK_:
		//	return    GENS_KEY_POWER;
		case GDK_EuroSign:
			return GENS_KEY_EURO;
		//case GDK_Undo:
		//	return        GENS_KEY_UNDO;
		default:
			//fprintf(stderr, "unknown gdk key\n");
			return -1;
	}
}
