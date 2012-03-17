/**
 * Gens: Video Drawing class - SDL
 */

#ifndef GENS_V_DRAW_SDL_HPP
#define GENS_V_DRAW_SDL_HPP

#include <SDL/SDL.h>
#include "v_draw.hpp"

class VDraw_SDL : public VDraw
{
	public:
		VDraw_SDL();
		VDraw_SDL(VDraw *oldDraw);
		~VDraw_SDL();
		
		int Init_Video(void);
		void End_Video(void);
		
		// Initialize the graphics subsystem.
		int Init_Subsystem(void);
		
		// Shut down the graphics subsystem.
		int Shut_Down(void);
		
		// Clear the screen.
		void clearScreen(void);
		//void Clear_Primary_Screen(void);
		//void Clear_Back_Screen(void);
		
		// Update VSync value.
		void updateVSync(bool unused = false);
		
	protected:
		int Init_SDL_Renderer(int w, int h);
		
		// Flip the screen buffer. (Called by v_draw.)
		int flipInternal(void);
		void drawBorder(void);
		
		// Update the renderer.
		void updateRenderer(void);
		
		// SDL flags
		static const int SDL_Flags =
				SDL_DOUBLEBUF |
				SDL_HWSURFACE |
				SDL_HWPALETTE |
				SDL_ASYNCBLIT |
				SDL_HWACCEL;
		
		SDL_Surface *screen;
		char SDL_WindowID[24];
};

#endif
