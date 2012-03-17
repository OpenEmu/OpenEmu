/**
 * Gens: Input class - SDL
 */

#ifndef GENS_INPUT_SDL_HPP
#define GENS_INPUT_SDL_HPP

#include "input.hpp"

#include <SDL/SDL.h>

// GTK stuff
#include <gtk/gtk.h>

class Input_SDL : public Input
{
	public:
		Input_SDL();
		~Input_SDL();
		
		// Update the input subsystem
		void update(void);
		
		// Check if the specified key is pressed.
		bool checkKeyPressed(unsigned int key);
		
		// Get a key. (Used for controller configuration.)
		unsigned int getKey(void);
		
	protected:
		static gint GDK_KeySnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data);
		
		// Functions required by the Input class.
		bool joyExists(int joyNum);
		
		// Check an SDL joystick axis.
		void checkJoystickAxis(SDL_Event *event);
		
		// Number of joysticks connected
		int m_numJoysticks;
		
		// SDL joystick structs
		SDL_Joystick *m_joy[6];
		
		// Key and joystick state.
		bool m_keys[1024];
		bool m_joyState[0x530];
};

#endif
