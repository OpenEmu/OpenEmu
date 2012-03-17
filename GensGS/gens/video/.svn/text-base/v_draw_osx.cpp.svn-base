/**
 * Gens: Video Drawing class - OSX
 */ 

#include "v_draw_osx.hpp"

#include <string.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"

// CPU flags
#include "gens_core/misc/cpuflags.h"


VDraw_OSX::VDraw_OSX()
{
}

VDraw_OSX::VDraw_OSX(VDraw *oldDraw)
: VDraw(oldDraw)
{
}

VDraw_OSX::~VDraw_OSX()
{
}

/**
 * Init_Video(): Initialize the video subsystem.
 * @return 1 on success.
 */
int VDraw_OSX::Init_Video(void)
{
	const int w = 320;
	const int h = 240;
	
	
	// Return the status code from Init_OSX_Renderer().
	return 1;
}


/**
 * Init_OSX_Renderer(): Initialize the OSX renderer.
 * @param w Width of the screen.
 * @param h Height of the screen.
 * @return 1 on success; 0 on error.
 */
int VDraw_OSX::Init_OSX_Renderer(int w, int h)
{

	return 1;
}


/**
 * End_Video(): Closes the OSX renderer.
 */
void VDraw_OSX::End_Video(void)
{
}


/**
 * clearScreen(): Clears the screen.
 */
void VDraw_OSX::clearScreen(void)
{
	// Clear the screen.
}


/**
 * Flip_internal(): Flip the screen buffer. (Called by v_draw.)
 * @return 1 on success; 0 on error.
 */
int VDraw_OSX::flipInternal(void)
{
	// Draw the border.
	//drawBorder();
	/*
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);

	// Start of the OSX framebuffer.
//	const int pitch = screen->pitch;
	const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	const int HBorder = m_HBorder * (bytespp / 2);		// Left border width, in pixels.
	
	const int startPos = ((pitch * VBorder) + HBorder) * m_scale;	// Starting position from within the screen.
	
	// Start of the OSX framebuffer.
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
		else∑
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
	
	OSX_UnlockSurface(screen);
	
	OSX_Flip(screen);
	*/
	// TODO: Return appropriate error code.
	return 1;
}


/**
 * drawBorder(): Draw the border color.
 * Called from flipInternal().
 */
void VDraw_OSX::drawBorder(void)
{
	/*// TODO: Consolidate this function by using a macro.
	
	OSX_Rect border;
	
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
			OSX_FillRect(screen, &border, m_BorderColor_16B);
			border.y = screen->h - border.h;
			OSX_FillRect(screen, &border, m_BorderColor_16B);
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
			OSX_FillRect(screen, &border, m_BorderColor_16B);
			border.x = screen->w - border.w;
			OSX_FillRect(screen, &border, m_BorderColor_16B);
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
			OSX_FillRect(screen, &border, m_BorderColor_32B);
			border.y = screen->h - border.h;
			OSX_FillRect(screen, &border, m_BorderColor_32B);
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
			OSX_FillRect(screen, &border, m_BorderColor_32B);
			border.x = screen->w - border.w;
			OSX_FillRect(screen, &border, m_BorderColor_32B);
		}
	}*/
}


/**
 * Init_Subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int VDraw_OSX::Init_Subsystem(void)
{
	/*if (OSX_InitSubSystem(OSX_INIT_TIMER) < 0)
	{
		fprintf (stderr, OSX_GetError());
		return -1;
	}
	if (OSX_InitSubSystem(OSX_INIT_VIDEO) < 0)
	{
		fprintf(stderr, OSX_GetError());
		return -1;
	}
	*/
	/* Take it back down now that we know it works. */
//	OSX_QuitSubSystem(OSX_INIT_VIDEO);
	
	return 0;
}


/**
 * Shut_Down(): Shut down the graphics subsystem.
 */
int VDraw_OSX::Shut_Down(void)
{
//OSX_Quit();
	return 1;
}


/**
 * updateRenderer(): Update the renderer.
 */
void VDraw_OSX::updateRenderer(void)
{
	/*// Check if a resolution switch is needed.
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
	stretchAdjustInternal();*/
}


/**
 * updateVSync(): Update VSync value.
 * @param unused Unused in this function.
 */
void VDraw_OSX::updateVSync(bool unused)
{
	//GENS_UNUSED_PARAMETER(unused);
	
	// Unfortunately, plain old OSX doesn't support VSync. :(
	return;
}
