/**
 * Gens: Video Drawing class - OSX
 */

#ifndef GENS_V_DRAW_OSX_HPP
#define GENS_V_DRAW_OSX_HPP

#include "v_draw.hpp"

class VDraw_OSX : public VDraw
	{
	public:
		VDraw_OSX();
		VDraw_OSX(VDraw *oldDraw);
		~VDraw_OSX();
		
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
		int Init_OSX_Renderer(int w, int h);
		
		// Flip the screen buffer. (Called by v_draw.)
		int flipInternal(void);
		void drawBorder(void);
		
		// Update the renderer.
		void updateRenderer(void);
		
	};

#endif
