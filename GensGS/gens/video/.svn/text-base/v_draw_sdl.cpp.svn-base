/**
 * Gens: Video Drawing class - SDL
 */ 

#include "v_draw_sdl.hpp"

#include <string.h>

#include <gdk/gdkx.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "ui/gtk/gtk-misc.h"
#include "gens/gens_window.hpp"

// CPU flags
#include "gens_core/misc/cpuflags.h"


VDraw_SDL::VDraw_SDL()
{
}

VDraw_SDL::VDraw_SDL(VDraw *oldDraw)
	: VDraw(oldDraw)
{
}

VDraw_SDL::~VDraw_SDL()
{
}

/**
 * Init_Video(): Initialize the video subsystem.
 * @return 1 on success.
 */
int VDraw_SDL::Init_Video(void)
{
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	const int scale = PluginMgr::getPluginFromID_Render(rendMode)->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (m_FullScreen)
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
		
		unsetenv("SDL_WINDOWID");
		strcpy(SDL_WindowID, "");
	}
	else
	{
		// Show the embedded SDL window.
		GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
		gtk_widget_set_size_request(sdlsock, w, h);
		gtk_widget_realize(sdlsock);
		gtk_widget_show(sdlsock);
		
		// Wait for GTK+ to catch up.
		// TODO: If gtk_main_iteration_do() returns TRUE, exit the program.
		while (gtk_events_pending())
			gtk_main_iteration_do(FALSE);
		
		// Get the Window ID of the SDL socket.
		sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(sdlsock->window)));
		setenv("SDL_WINDOWID", SDL_WindowID, 1);
	}
	
	// Initialize SDL.
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		// Error initializing SDL.
		Init_Fail("Couldn't initialize embedded SDL.");
		return 0;
	}
	
	// Initialize the renderer.
	int x = Init_SDL_Renderer(w, h);
	
	// Disable the cursor in fullscreen mode.
	SDL_ShowCursor(m_FullScreen ? SDL_DISABLE : SDL_ENABLE);
	
	// Return the status code from Init_SDL_Renderer().
	return x;
}


/**
 * Init_SDL_Renderer(): Initialize the SDL renderer.
 * @param w Width of the screen.
 * @param h Height of the screen.
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL::Init_SDL_Renderer(int w, int h)
{
	screen = SDL_SetVideoMode(w, h, bppOut, SDL_Flags | (m_FullScreen ? SDL_FULLSCREEN : 0));
	
	if (!screen)
	{
		// Error initializing SDL.
		fprintf(stderr, "Error creating SDL primary surface: %s\n", SDL_GetError());
		exit(0);
	}
	
	return 1;
}


/**
 * End_Video(): Closes the SDL renderer.
 */
void VDraw_SDL::End_Video(void)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/**
 * clearScreen(): Clears the screen.
 */
void VDraw_SDL::clearScreen(void)
{
	// Clear the screen.
	SDL_LockSurface(screen);
	memset(screen->pixels, 0x00, screen->pitch * screen->h);
	SDL_UnlockSurface(screen);
	
	// Reset the border color to make sure it's redrawn.
	m_BorderColor_16B = ~MD_Palette[0];
	m_BorderColor_32B = ~MD_Palette32[0];
}


/**
 * Flip_internal(): Flip the screen buffer. (Called by v_draw.)
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL::flipInternal(void)
{
	SDL_LockSurface(screen);
	
	// Draw the border.
	drawBorder();
	
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// Start of the SDL framebuffer.
	const int pitch = screen->pitch;
	const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	const int HBorder = m_HBorder * (bytespp / 2);		// Left border width, in pixels.
	
	const int startPos = ((pitch * VBorder) + HBorder) * m_scale;	// Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(screen->pixels))[startPos]);
	
	// Set up the render information.
	if (m_rInfo.bpp != bppMD)
	{
		// bpp has changed. Reinitialize the screen pointers.
		m_rInfo.bpp = bppMD;
		m_rInfo.cpuFlags = CPU_Flags;
	}
	
	m_rInfo.destScreen = (void*)start;
	m_rInfo.width = 320 - m_HBorder;
	m_rInfo.height = VDP_Num_Vis_Lines;
	m_rInfo.destPitch = pitch;
	
	if (bppMD == 16 && bppOut != 16)
	{
		// MDP_RENDER_FLAG_SRC16DST32.
		// Render as 16-bit to an internal surface.
		
		// Make sure the internal surface is initialized.
		if (m_tmp16img_scale != m_scale)
		{
			if (m_tmp16img)
				free(m_tmp16img);
			
			m_tmp16img_scale = m_scale;
			m_tmp16img_pitch = 320 * m_scale * 2;
			m_tmp16img = static_cast<uint16_t*>(malloc(m_tmp16img_pitch * 240 * m_scale));
		}
		
		m_rInfo.destScreen = (void*)m_tmp16img;
		m_rInfo.destPitch = m_tmp16img_pitch;
		if (m_FullScreen)
			m_BlitFS(&m_rInfo);
		else
			m_BlitW(&m_rInfo);
		
		Render_16to32((uint32_t*)start, m_tmp16img,
			      m_rInfo.width * m_scale, m_rInfo.height * m_scale,
			      pitch, m_tmp16img_pitch);
	}
	else
	{
		if (m_FullScreen)
			m_BlitFS(&m_rInfo);
		else
			m_BlitW(&m_rInfo);
	}
	
	// Draw the message and/or FPS.
	if (m_MsgVisible)
	{
		// Message is visible.
		drawText(screen->pixels, screen->w, screen->w, screen->h, m_MsgText.c_str(), m_MsgStyle);
	}
	else if (m_FPSEnabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		drawText(screen->pixels, screen->w, screen->w, screen->h, m_MsgText.c_str(), m_FPSStyle);
	}
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
	
	// TODO: Return appropriate error code.
	return 1;
}


/**
 * drawBorder(): Draw the border color.
 * Called from flipInternal().
 */
void VDraw_SDL::drawBorder(void)
{
	// TODO: Consolidate this function by using a macro.
	
	SDL_Rect border;
	
	if (!Video.borderColorEmulation)
	{
		// Border color emulation is disabled.
		// Don't do anything if the border color is currently black.
		if (m_BorderColor_16B == 0 && m_BorderColor_32B == 0)
			return;
	}
	
	unsigned short newBorderColor_16B = MD_Palette[0];
	unsigned int newBorderColor_32B = MD_Palette32[0];
	
	if (!Video.borderColorEmulation || (Game == NULL) || (Debug > 0))
	{
		// Either no game is loaded or the debugger is enabled.
		// Make sure the border color is black.
		newBorderColor_16B = 0;
		newBorderColor_32B = 0;
	}
	
	if ((bppOut == 15 || bppOut == 16) && (m_BorderColor_16B != newBorderColor_16B))
	{
		m_BorderColor_16B = newBorderColor_16B;
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = screen->w;
			border.h = ((240 - VDP_Num_Vis_Lines) >> 1) * m_scale;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
			border.y = screen->h - border.h;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
		}
		if (m_HBorder > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = screen->h;
			border.w = (m_HBorder >> 1) * m_scale;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
			border.x = screen->w - border.w;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
		}
	}
	else if ((bppOut == 32) && (m_BorderColor_32B != newBorderColor_32B))
	{
		m_BorderColor_32B = newBorderColor_32B;
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = screen->w;
			border.h = ((240 - VDP_Num_Vis_Lines) >> 1) * m_scale;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
			border.y = screen->h - border.h;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
		}
		if (m_HBorder > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = screen->h;
			border.w = (m_HBorder >> 1) * m_scale;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
			border.x = screen->w - border.w;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
		}
	}
}


/**
 * Init_Subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int VDraw_SDL::Init_Subsystem(void)
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
	{
		fprintf (stderr, SDL_GetError());
		return -1;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, SDL_GetError());
		return -1;
	}
	
	/* Take it back down now that we know it works. */
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	return 0;
}


/**
 * Shut_Down(): Shut down the graphics subsystem.
 */
int VDraw_SDL::Shut_Down(void)
{
	SDL_Quit();
	return 1;
}


/**
 * updateRenderer(): Update the renderer.
 */
void VDraw_SDL::updateRenderer(void)
{
	// Check if a resolution switch is needed.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	const int scale = PluginMgr::getPluginFromID_Render(rendMode)->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (screen->w == w && screen->h == h)
	{
		// No resolution switch is necessary. Simply clear the screen.
		clearScreen();
		return;
	}
	
	// Resolution switch is needed.
	End_Video();
	Init_Video();
	
	// Clear the screen.
	clearScreen();
	
	// Adjust stretch parameters.
	stretchAdjustInternal();
}


/**
 * updateVSync(): Update VSync value.
 * @param unused Unused in this function.
 */
void VDraw_SDL::updateVSync(bool unused)
{
	GENS_UNUSED_PARAMETER(unused);
	
	// Unfortunately, plain old SDL doesn't support VSync. :(
	return;
}
