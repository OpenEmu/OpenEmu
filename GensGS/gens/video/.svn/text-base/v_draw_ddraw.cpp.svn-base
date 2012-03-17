/**
 * Gens: Video Drawing class - DirectDraw
 */ 

#include "v_draw_ddraw.hpp"

#include <string.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.hpp"
#include "gens/gens_window_sync.hpp"

// Inline video functions.
#include "v_inline.h"


inline void VDraw_DDraw::DDraw_Draw_Text(DDSURFACEDESC2* pddsd, LPDIRECTDRAWSURFACE4 lpDDS_Surface, const bool lock)
{
	if (lock)
		lpDDS_Surface->Lock(NULL, pddsd, DDLOCK_WAIT, NULL);
	
	// Determine the window size using the scaling factor.
	const int w = 320 * m_scale;
	const int h = 240 * m_scale;
	
	// +(8*bytespp) is needed for the lpSurface pointer because the DDraw module
	// includes the entire 336x240 MD_Screen. The first 8 pixels are offscreen,
	// so they won't show up at all.
	
	unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// NOTE: fullW must be (pddsd->lPitch / bytespp).
	// DirectDraw likes to use absurdly large line lengths in full screen mode.
	// (pddsd->lPitch / bytespp) does match pddsd->dwWidth in windowed mode, though.
	
	if (m_MsgVisible)
	{
		// Message is visible.
		drawText((unsigned char*)pddsd->lpSurface + (8*bytespp), pddsd->lPitch / bytespp,
			 w, h, m_MsgText.c_str(), m_MsgStyle, false);
	}
	else if (m_FPSEnabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		drawText((unsigned char*)pddsd->lpSurface + (8*bytespp), pddsd->lPitch / bytespp,
			 w, h, m_MsgText.c_str(), m_FPSStyle, false);
	}
	
	if (lock)
		lpDDS_Surface->Unlock(NULL);
}


VDraw_DDraw::VDraw_DDraw()
{
	// Initialize DDraw buffers to NULL.
	lpDD = NULL;
	lpDDS_Primary = NULL;
	lpDDS_Flip = NULL;
	lpDDS_Back = NULL;
	lpDDC_Clipper = NULL;
}

VDraw_DDraw::~VDraw_DDraw()
{
}


/**
 * Init_Fail(): Initialization failure.
 * @param HWND Window handle.
 * @param err Error message.
 * @return 0 to indicate an error has occurred.
 */
int VDraw_DDraw::Init_Fail(HWND hWnd, const char *err)
{
	End_Video();
	GensUI::msgBox(err, "DirectDraw Error", GensUI::MSGBOX_ICON_ERROR);
	DestroyWindow(hWnd);
	TerminateProcess(GetCurrentProcess(), 1); //Modif N
	return 0;
}


/**
 * Init_Video(): Initialize the video subsystem.
 * @return 1 on success; 0 on error.
 */
int VDraw_DDraw::Init_Video(void)
{
	DDSURFACEDESC2 ddsd;
	
	End_Video();
	
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	const int scale = PluginMgr::getPluginFromID_Render(rendMode)->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	const int w = 320 * scale;
	const int h = 240 * scale;
	const int mdW = 336 * scale;
	
	if (m_FullScreen)
	{
		Res_X = w;
		Res_Y = h;
	}
	
	if (FAILED(DirectDrawCreate(NULL, &lpDD_Init, NULL)))
		return Init_Fail(Gens_hWnd, "Error with DirectDrawCreate!");
	
	if (FAILED(lpDD_Init->QueryInterface(IID_IDirectDraw4, (LPVOID *) &lpDD)))
		return Init_Fail(Gens_hWnd, "Error with QueryInterface!\nUpgrade your DirectX version.");
	
	lpDD_Init->Release();
	lpDD_Init = NULL;
	
	// TODO: 15-bit color override ("Force 555" or "Force 565" in the config file).
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	// TODO: Figure out what FS_No_Res_Change is for.
	// TODO: Figure out if this is correct.
	if (m_FullScreen /* && !FS_No_Res_Change*/)
	{
		// Always use 16-bit color in fullscreen.
		if (FAILED(lpDD->SetDisplayMode(Res_X, Res_Y, 16, 0, 0)))
			return Init_Fail(Gens_hWnd, "Error with lpDD->SetDisplayMode()!");
	}
	
	// Check the current color depth.
	unsigned char newBpp;
	lpDD->GetDisplayMode(&ddsd);
	switch (ddsd.ddpfPixelFormat.dwGBitMask)
	{
		case 0x03E0:
			// 15-bit color.
			newBpp = 15;
			break;
		case 0x07E0:
			// 16-bit color.
			newBpp = 16;
			break;
		case 0x00FF00:
		default:
			// 32-bit color.
			newBpp = 32;
			break;
	}
	
	if (newBpp != bppOut)
		setBpp(newBpp, false);
	
	setCooperativeLevel();
	
	// Clear ddsd.
	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	if (m_FullScreen && Video.VSync_FS)
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 2;
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}
	
	if (FAILED(lpDD->CreateSurface(&ddsd, &lpDDS_Primary, NULL)))
	{
		return Init_Fail(Gens_hWnd, "Error with lpDD->CreateSurface()! [lpDDS_Primary]");
	}
	
	if (m_FullScreen)
	{
		if (Video.VSync_FS)
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
			
			if (FAILED(lpDDS_Primary->GetAttachedSurface(&ddsd.ddsCaps, &lpDDS_Flip)))
				return Init_Fail(Gens_hWnd, "Error with lpDDPrimary->GetAttachedSurface()!");
			
			lpDDS_Blit = lpDDS_Flip;
		}
		else
		{
			lpDDS_Blit = lpDDS_Primary;
		}
	}
	else
	{
		if (FAILED(lpDD->CreateClipper(0, &lpDDC_Clipper, NULL )))
			return Init_Fail(Gens_hWnd, "Error with lpDD->CreateClipper !");
		
		if (FAILED(lpDDC_Clipper->SetHWnd(0, Gens_hWnd)))
			return Init_Fail(Gens_hWnd, "Error with lpDDC_Clipper->SetHWnd !");
		
		if (FAILED(lpDDS_Primary->SetClipper(lpDDC_Clipper)))
			return Init_Fail(Gens_hWnd, "Error with lpDDS_Primary->SetClipper !");
	}
	
	// Clear ddsd again.
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	
	// Determine the width and height.
	// NOTE: For DirectDraw, the actual 336 width is used.
	if (rendMode == 0)
	{
		// Normal render mode. 320x240 [336x240]
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		ddsd.dwWidth = 336;
		ddsd.dwHeight = 240;
	}
	else
	{
		// Larger than 1x.
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth = mdW;
		ddsd.dwHeight = h;
	}
	
	if (FAILED(lpDD->CreateSurface(&ddsd, &lpDDS_Back, NULL)))
		return Init_Fail(Gens_hWnd, "Error with lpDD->CreateSurface()! [lpDDS_Back]");
	
	// TODO: Check if this is right.
	// I think this might be causing the frame counter flicker in full screen mode.
	//if (!m_FullScreen || (rendMode >= 1 && (/*FS_No_Res_Change ||*/ Res_X != 640 || Res_Y != 480)))
	if (!(m_FullScreen && rendMode == 0))
		lpDDS_Blit = lpDDS_Back;
	
	if (rendMode == 0)
	{
		// Normal rendering mode uses MD_Screen directly.
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		
		if (FAILED(lpDDS_Back->GetSurfaceDesc(&ddsd)))
			return Init_Fail(Gens_hWnd, "Error with lpDD_Back->GetSurfaceDesc()!");
		
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
		ddsd.dwWidth = 336;
		ddsd.dwHeight = 240;
		
		if (ddsd.ddpfPixelFormat.dwRGBBitCount > 16)
		{
			// 32-bit color.
			ddsd.lpSurface = MD_Screen32;
			ddsd.lPitch = 336 * 4;
		}
		else
		{
			// 15-bit or 16-bit color.
			ddsd.lpSurface = MD_Screen;
			ddsd.lPitch = 336 * 2;
		}
		
		if (FAILED(lpDDS_Back->SetSurfaceDesc(&ddsd, 0)))
			return Init_Fail(Gens_hWnd, "Error with lpDD_Back->SetSurfaceDesc()!");
	}
	
	// Reset the render mode.
	setRender(rendMode, false);
	
	// Synchronize menus.
	Sync_Gens_Window();
	
	// VDraw_DDraw initialized.
	return 1;
}


/**
 * reinitGensWindow(): Reinitialize the Gens window.
 * @return 1 on success; 0 on error.
 */
int VDraw_DDraw::reinitGensWindow(void)
{
	// Reinitialize the Gens window.
	RECT r;
	
	if (audio->soundInitialized())
		audio->clearSoundBuffer();
	
	End_Video();
	
	// Rebuild the menu bar.
	// This is needed if the mode is switched from windowed to fullscreen, or vice-versa.
	create_gens_window_menubar();
	
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	const int scale = PluginMgr::getPluginFromID_Render(rendMode)->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (m_FullScreen)
	{
		while (ShowCursor(true) < 1) { }
		while (ShowCursor(false) >= 0) { }
		
		SetWindowLong(Gens_hWnd, GWL_STYLE, NULL);
		SetWindowPos(Gens_hWnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		while (ShowCursor(false) >= 0) { }
		while (ShowCursor(true) < 1) { }
		
		// MoveWindow / ResizeWindow code
		SetWindowLong(Gens_hWnd, GWL_STYLE, GetWindowLong(Gens_hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW);
		SetWindowPos(Gens_hWnd, NULL, Window_Pos.x, Window_Pos.y, 0, 0,
			     SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		Win32_setActualWindowSize(Gens_hWnd, w, h);
	}
	
	return Init_Video();
}


/**
 * End_Video(): Closes the DirectDraw renderer.
 */
void VDraw_DDraw::End_Video(void)
{
	if (lpDDC_Clipper)
	{
		lpDDC_Clipper->Release();
		lpDDC_Clipper = NULL;
	}
	
	if (lpDDS_Back)
	{
		lpDDS_Back->Release();
		lpDDS_Back = NULL;
	}
	
	if (lpDDS_Flip)
	{
		lpDDS_Flip->Release();
		lpDDS_Flip = NULL;
	}
	
	if (lpDDS_Primary)
	{
		lpDDS_Primary->Release();
		lpDDS_Primary = NULL;
	}
	
	if (lpDD)
	{
		lpDD->SetCooperativeLevel(Gens_hWnd, DDSCL_NORMAL);
		lpDD->Release();
		lpDD = NULL;
	}
	
	lpDDS_Blit = NULL;
}


HRESULT VDraw_DDraw::RestoreGraphics(void)
{
	HRESULT rval1 = lpDDS_Primary->Restore();
	HRESULT rval2 = lpDDS_Back->Restore();
	
	// Modif N. -- fixes lost surface handling when the color depth has changed
	if (rval1 == DDERR_WRONGMODE || rval2 == DDERR_WRONGMODE)
		return Init_Video() ? DD_OK : DDERR_GENERIC;
	
	return SUCCEEDED(rval2) ? rval1 : rval2;
}


/**
 * stretchAdjustInternal(): Adjust stretch parameters.
 * Called by either VDraw or another function in VDraw_DDraw.
 */
void VDraw_DDraw::stretchAdjustInternal(void)
{
	if (m_Stretch & STRETCH_H)
		m_HStretch = ((m_HBorder * 0.0625f) / 64.0f);
	else
		m_HStretch = 0;
	
	if (m_Stretch & STRETCH_V)
		m_VStretch = (((240 - VDP_Num_Vis_Lines) / 240.0f) / 2.0);
	else
		m_VStretch = 0;
	
	if (m_Stretch != STRETCH_FULL)
		clearScreen();
}


/**
 * clearScreen(): Clears the screen.
 */
void VDraw_DDraw::clearScreen(void)
{
	// Clear both screen buffers.
	clearPrimaryScreen();
	clearBackScreen();
	
	// Reset the border color to make sure it's redrawn.
	m_BorderColor_16B = ~MD_Palette[0];
	m_BorderColor_32B = ~MD_Palette32[0];
}


int VDraw_DDraw::clearPrimaryScreen(void)
{
	if (!lpDD || !lpDDS_Primary)
		return 0;
	
	DDSURFACEDESC2 ddsd;
	DDBLTFX ddbltfx;
	RECT rd;
	POINT p;
	
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0; // Black
	
	if (m_FullScreen)
	{
		if (Video.VSync_FS)
		{
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
			
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
			
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
		}
		else
		{
			lpDDS_Primary->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
		}
	}
	else
	{
		p.x = p.y = 0;
		GetClientRect(Gens_hWnd, &rd);
		ClientToScreen(Gens_hWnd, &p);
		
		rd.left = p.x;
		rd.top = p.y;
		rd.right += p.x;
		rd.bottom += p.y;
		
		if (rd.top < rd.bottom)
			lpDDS_Primary->Blt(&rd, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	}
	
	return 1;
}


int VDraw_DDraw::clearBackScreen(void)
{
	if (!lpDD || !lpDDS_Back)
		return 0;
	
	DDSURFACEDESC2 ddsd;
	DDBLTFX ddbltfx;
	
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;
	
	lpDDS_Back->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	
	return 1;
}


void VDraw_DDraw::restorePrimary(void)
{
	if (lpDD && m_FullScreen && Video.VSync_FS)
	{
		while (lpDDS_Primary->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_SURFACEBUSY) { }
		lpDD->FlipToGDISurface();
	}
}


// Render_Mode is input, RectDest is input and output, everything else is output only
void VDraw_DDraw::CalculateDrawArea(int Render_Mode, RECT& RectDest, RECT& RectSrc, float& Ratio_X, float& Ratio_Y, int& Dep)
{
	Ratio_X = (float)RectDest.right / 320.0f;  //Upth-Modif - why use two lines of code
	Ratio_Y = (float)RectDest.bottom / 240.0f; //Upth-Modif - when you can do this?
	Ratio_X = Ratio_Y = (Ratio_X < Ratio_Y) ? Ratio_X : Ratio_Y; //Upth-Add - and here we floor the value
	
	POINT q; //Upth-Add - For determining the correct ratio
	q.x = RectDest.right; //Upth-Add - we need to get
	q.y = RectDest.bottom; //Upth-Add - the bottom-right corner
	
	if (m_scale == 1)
	{
		RectSrc.top = 0;
		RectSrc.bottom = VDP_Num_Vis_Lines;

		if ((VDP_Num_Vis_Lines == 224) && !(m_Stretch & STRETCH_V))
		{
			RectDest.top = (int) ((q.y - (224 * Ratio_Y))/2); //Upth-Modif - Centering the screen properly
			RectDest.bottom = (int) (224 * Ratio_Y) + RectDest.top; //Upth-Modif - along the y axis
		}
	}
	else
	{
		if (VDP_Num_Vis_Lines == 224)
		{
			RectSrc.top = 8 * m_scale;
			RectSrc.bottom = (224 + 8) * m_scale;

			if (!(m_Stretch & STRETCH_V))
			{
				RectDest.top = (int) ((q.y - (224 * Ratio_Y)) / 2); //Upth-Modif - Centering the screen properly
				RectDest.bottom = (int) (224 * Ratio_Y) + RectDest.top; //Upth-Modif - along the y axis again
			}
		}
		else
		{
			RectSrc.top = 0; //Upth-Modif - Was "0 * 2"
			RectSrc.bottom = (240 * m_scale);
		}
	}

	if (isFullXRes())
	{
		Dep = 0;

		if (Render_Mode == 0)
		{
			RectSrc.left = 8 + 0 ;
			RectSrc.right = 8 + 320;
		}
		else
		{
			RectSrc.left = 0; //Upth-Modif - Was "0 * 2"
			RectSrc.right = 320 * m_scale;
		}
		RectDest.left = (int) ((q.x - (320 * Ratio_X)) / 2); //Upth-Add - center the picture
		RectDest.right = (int) (320 * Ratio_X) + RectDest.left; //Upth-Add - along the x axis
	}
	else // less-wide X resolution:
	{
		Dep = 64;
		
		if (!(m_Stretch & STRETCH_H))
		{
			RectDest.left = (q.x - (int)(256.0f * Ratio_X)) / 2; //Upth-Modif - center the picture properly
			RectDest.right = (int)(256.0f * Ratio_X) + RectDest.left; //Upth-Modif - along the x axis
		}
		
		if (Render_Mode == 0)
		{
			RectSrc.left = 8 + 0;
			RectSrc.right = 8 + 256;
		}
		else
		{
			RectSrc.left = 32 * m_scale;
			RectSrc.right = (32 + 256) * m_scale;
		}
	}
}


/**
 * flipInternal(): Flip the screen buffer. (Called by v_draw.)
 * @return 1 on success; 0 on error.
 */
int VDraw_DDraw::flipInternal(void)
{
	// TODO: Add border drawing, like in v_draw_sdl.
	
	if (!lpDD)
		return 0;
	
	HRESULT rval = DD_OK;
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	RECT RectDest, RectSrc;
	POINT p;
	float Ratio_X, Ratio_Y;
	int Dep = 0;
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// Set up the render information.
	if (m_rInfo.bpp != bppMD)
	{
		// bpp has changed. Reinitialize the screen pointers.
		m_rInfo.bpp = bppMD;
		m_rInfo.cpuFlags = CPU_Flags;
	}
	
	if (m_FullScreen)
	{
		//Upth-Add - So we can set the fullscreen resolution to the current res without changing the value that gets saved to the config
		int FS_X, FS_Y;
		
#if 0
		if (Res_X < (320 << (int)(Video.Render_FS > 0)))
			Res_X = 320 << (int)(Video.Render_FS > 0); //Upth-Add - Flooring the resolution to 320x240
		if (Res_Y < (240 << (int)(Video.Render_FS > 0)))
			Res_Y = 240 << (int)(Video.Render_FS > 0); //Upth-Add - or 640x480, as appropriate
#endif
			
		// TODO: FS_No_Res_Change
#if 0
		if (FS_No_Res_Change)
		{
			//Upth-Add - If we didn't change resolution when we went Full Screen
			DEVMODE temp;
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&temp); //Upth-Add - Gets the current screen resolution
			FS_X = temp.dmPelsWidth;
			FS_Y = temp.dmPelsHeight;
		}
		else
		{
#endif
			//Upth-Add - Otherwise use the configured resolution values
			FS_X = Res_X; 
			FS_Y = Res_Y;
#if 0
		}
#endif
		
		Ratio_X = (float)FS_X / 320.0f; //Upth-Add - Find the current size-ratio on the x-axis
		Ratio_Y = (float)FS_Y / 240.0f; //Upth-Add - Find the current size-ratio on the y-axis
		
		Ratio_X = Ratio_Y = (Ratio_X < Ratio_Y) ? Ratio_X : Ratio_Y; //Upth-Add - Floor them to the smaller value for correct ratio display
		
		if (isFullXRes())
		{
			if (Flag_Clr_Scr != 40)
			{
				clearPrimaryScreen();
				clearBackScreen();
				Flag_Clr_Scr = 40;
			}
			
			Dep = 0;
			RectSrc.left = 0 + 8;
			RectSrc.right = 320 + 8;
			RectDest.left = (int) ((FS_X - (320 * Ratio_X))/2); //Upth-Modif - Offset the left edge of the picture to the center of the screen
			RectDest.right = (int) (320 * Ratio_X) + RectDest.left; //Upth-Modif - Stretch the picture and move the right edge the same amount
		}
		else
		{
			if (Flag_Clr_Scr != 32)
			{
				clearPrimaryScreen();
				clearBackScreen();
				Flag_Clr_Scr = 32;
			}
			
			Dep = 64;
			RectSrc.left = 0 + 8;
			RectSrc.right = 256 + 8;
			
			if (m_Stretch)
			{
				RectDest.left = 0;
				RectDest.right = FS_X; //Upth-Modif - use the user configured value
				RectDest.top = 0;      //Upth-Add - also, if we have stretch enabled
				RectDest.bottom = FS_Y;//Upth-Add - we don't correct the screen ratio
			}
			else
			{
				RectDest.left = (FS_X - (int)(256.0f * Ratio_X)) / 2; //Upth-Modif - Centering the screen left-right
				RectDest.right = (int)(256.0f * Ratio_X) + RectDest.left; //Upth-modif - again
			}
			RectDest.top = (int) ((FS_Y - (240 * Ratio_Y))/2); //Upth-Add - Centers the screen top-bottom, in case Ratio_X was the floor.
		}
		
		// TODO: Figure out how to get this working.
		// Until I can get it working, fall back to the standard 2x renderer.
#if 0
		if (Video.Render_FS == 1)
		{
			// 2x rendering.
			if (m_swRender)
			{
				// Software rendering is enabled.
				// Ignore the Stretch setting.
				rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
				
				if (FAILED(rval))
					goto cleanup_flip;
				
				int VBorder = ((240 - VDP_Num_Vis_Lines) / 2) << m_shift;	// Top border height, in pixels.
				int HBorder = (Dep * (bytespp / 2)) << m_shift;			// Left border width, in pixels.
				int startPos = (ddsd.lPitch * VBorder) + HBorder;
				unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
				Blit_FS(start, ddsd.lPitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + (Dep * 2));
				
				lpDDS_Blit->Unlock(NULL);
				
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				RectSrc.top = 0;
				RectSrc.bottom = VDP_Num_Vis_Lines;

				if ((VDP_Num_Vis_Lines == 224) && !m_Stretch)
				{
					RectDest.top = (int) ((FS_Y - (224 * Ratio_Y))/2); //Upth-Modif - centering top-bottom
					RectDest.bottom = (int) (224 * Ratio_Y) + RectDest.top; //Upth-Modif - with the method I already described for left-right
				}
				else
				{
					RectDest.top = (int) ((FS_Y - (240 * Ratio_Y))/2); //Upth-Modif - centering top-bottom under other circumstances
					RectDest.bottom = (int) (240 * Ratio_Y) + RectDest.top; //Upth-Modif - using the same method
				}
				RectDest.left = (int) ((FS_X - (320 * Ratio_X))/2); //Upth-Add - Centering left-right
				RectDest.right = (int) (320 * Ratio_X) + RectDest.left; //Upth-Add - I wonder why I had to change the center-stuff three times...

				if (Video.VSync_FS)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
				{
					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
				}
			}
		}
		else
#endif
		if (Video.Render_FS == 0)
		{
			// 1x rendering.
			if (m_swRender)
			{
				// Software rendering is enabled.
				// Ignore the Stretch setting.
				
				rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
				
				if (FAILED(rval))
					goto cleanup_flip;
				
				const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
				const int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
				
				const int startPos = ((ddsd.lPitch * VBorder) + HBorder) * m_scale;	// Starting position from within the screen.
				
				// Start of the DDraw framebuffer.
				unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
				
				m_rInfo.destScreen = (void*)start;
				m_rInfo.width = 320 - m_HBorder;
				m_rInfo.height = VDP_Num_Vis_Lines;
				m_rInfo.destPitch = ddsd.lPitch;
				
				m_BlitFS(&m_rInfo);
				
				// Draw the text.
				DDraw_Draw_Text(&ddsd, lpDDS_Blit, false);
				
				lpDDS_Blit->Unlock(NULL);
				
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				// Software rendering is disabled.
				// If Stretch is enabled, stretch the image.
				
				RectSrc.top = 0;
				RectSrc.bottom = VDP_Num_Vis_Lines;

				if (!(m_Stretch & STRETCH_V))
				{
					RectDest.top = (int)((FS_Y - VDP_Num_Vis_Lines) / 2); //Upth-Add - But we still
					RectDest.bottom = RectDest.top + VDP_Num_Vis_Lines;   //Upth-Add - center the screen
				}
				else
				{
					RectDest.top = (int)((FS_Y - 240) / 2); //Upth-Add - for both of the
					RectDest.bottom = RectDest.top + 240;   //Upth-Add - predefined conditions
				}
				
				if (!(m_Stretch & STRETCH_H))
				{
					RectDest.left = (int)((FS_X - (320 - Dep))/2); //Upth-Add - and along the
					RectDest.right = (320 - Dep) + RectDest.left;  //Upth-Add - x axis, also
				}
				else
				{
					RectDest.left = (int)((FS_X - 320)/2); //Upth-Add - and along the
					RectDest.right = 320 + RectDest.left;  //Upth-Add - x axis, also
				}
				
				DDraw_Draw_Text(&ddsd, lpDDS_Back, true);
				if (Video.VSync_FS)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					//DDraw_Draw_Text(&ddsd, lpDDS_Flip, Video.Render_FS, true);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
				{
					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					//DDraw_Draw_Text(&ddsd, lpDDS_Primary, Video.Render_FS, true);
//					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
				}
			}
		}
		else
		{
			// Other renderer.
			
			LPDIRECTDRAWSURFACE4 curBlit = lpDDS_Blit;
#ifdef CORRECT_256_ASPECT_RATIO
			if(!IS_FULL_X_RESOLUTION)
				curBlit = lpDDS_Back; // have to use it or the aspect ratio will be way off
#endif
			rval = curBlit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			
			if (FAILED(rval))
				goto cleanup_flip;
			
			const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
			const int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
				
			const int startPos = ((ddsd.lPitch * VBorder) + HBorder) * m_scale;	// Starting position from within the screen.
				
			// Start of the DDraw framebuffer.
			unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
			m_rInfo.destScreen = (void*)start;
			m_rInfo.width = 320 - m_HBorder;
			m_rInfo.height = VDP_Num_Vis_Lines;
			m_rInfo.destPitch = ddsd.lPitch;
			
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
					      ddsd.lPitch, m_tmp16img_pitch);
			}
			else
			{
				m_BlitFS(&m_rInfo);
			}
			
			// Draw the text.
			DDraw_Draw_Text(&ddsd, curBlit, false);
			
			curBlit->Unlock(NULL);
			
			if (curBlit == lpDDS_Back) // note: this can happen in windowed fullscreen, or if CORRECT_256_ASPECT_RATIO is defined and the current display mode is 256 pixels across
			{
				RectDest.left = 0;
				RectDest.top = 0;
				RectDest.right = GetSystemMetrics(SM_CXSCREEN); // not SM_XVIRTUALSCREEN since we only want the primary monitor if there's more than one
				RectDest.bottom = GetSystemMetrics(SM_CYSCREEN);

				CalculateDrawArea(Video.Render_FS, RectDest, RectSrc, Ratio_X, Ratio_Y, Dep);

				if (Video.VSync_FS)
				{
					int vb;
					lpDD->GetVerticalBlankStatus(&vb);
					if (!vb) lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
				}

				lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
			}
			else
			{
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
		}
	}
	else
	{
		// Windowed mode
		GetClientRect(Gens_hWnd, &RectDest);
		CalculateDrawArea(Video.Render_W, RectDest, RectSrc, Ratio_X, Ratio_Y, Dep);

		int Clr_Cmp_Val = isFullXRes() ? 40 : 32;
		if (Flag_Clr_Scr != Clr_Cmp_Val)
		{
			clearPrimaryScreen();
			clearBackScreen();
			Flag_Clr_Scr = Clr_Cmp_Val;
		}

		if (Video.Render_W >= 1)
		{
			rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			
			if (FAILED(rval))
				goto cleanup_flip;
			
			const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
			const int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
				
			const int startPos = ((ddsd.lPitch * VBorder) + HBorder) * m_scale;	// Starting position from within the screen.
				
			// Start of the DDraw framebuffer.
			unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
			m_rInfo.destScreen = (void*)start;
			m_rInfo.width = 320 - m_HBorder;
			m_rInfo.height = VDP_Num_Vis_Lines;
			m_rInfo.destPitch = ddsd.lPitch;
			
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
					      ddsd.lPitch, m_tmp16img_pitch);
			}
			else
			{
				m_BlitW(&m_rInfo);
			}
			
			// Draw the text.
			DDraw_Draw_Text(&ddsd, lpDDS_Blit, false);
			
			lpDDS_Blit->Unlock(NULL);
		}
		else
		{
			// Draw the text.
			DDraw_Draw_Text(&ddsd, lpDDS_Blit, true);
		}
		
		p.x = p.y = 0;
		ClientToScreen(Gens_hWnd, &p);
		
		RectDest.top += p.y; //Upth-Modif - this part moves the picture into the window
		RectDest.bottom += p.y; //Upth-Modif - I had to move it after all of the centering
		RectDest.left += p.x;   //Upth-Modif - because it modifies the values
		RectDest.right += p.x;  //Upth-Modif - that I use to find the center

		if (RectDest.top < RectDest.bottom)
		{
			if (Video.VSync_W)
			{
				int vb;
				lpDD->GetVerticalBlankStatus(&vb);
				if (!vb)
					lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
			}

			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
		}
	}

cleanup_flip:
	if (rval == DDERR_SURFACELOST)
		rval = RestoreGraphics();
	
	return 1;
}


/**
 * Init_Subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int VDraw_DDraw::Init_Subsystem(void)
{
	// Does nothing...
	return 0;
}


/**
 * Shut_Down(): Shut down the graphics subsystem.
 */
int VDraw_DDraw::Shut_Down(void)
{
	// Does nothing...
	return 1;
}


/**
 * updateRenderer(): Update the renderer.
 */
void VDraw_DDraw::updateRenderer(void)
{
	// Does nothing...
}


/**
 * updateVSync(): Update VSync value.
 * @param fromInitSDLGL True if being called from Init_SDL_DDraw_Renderer().
 */
void VDraw_DDraw::updateVSync(bool fromInitSDLGL)
{
	// If Full Screen, reinitialize the video subsystem.
	if (m_FullScreen)
		Refresh_Video();
}


/**
 * setCooperativeLevel(): Sets the cooperative level.
 */
void VDraw_DDraw::setCooperativeLevel(void)
{
	if (!Gens_hWnd || !lpDD)
		return;
	
	HRESULT rval;
#ifdef DISABLE_EXCLUSIVE_FULLSCREEN_LOCK
	Video.VSync_FS = 0;
	rval = lpDD->SetCooperativeLevel(Gens_hWnd, DDSCL_NORMAL);
#else
	if (m_FullScreen)
		rval = lpDD->SetCooperativeLevel(Gens_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	else
		rval = lpDD->SetCooperativeLevel(Gens_hWnd, DDSCL_NORMAL);
#endif
	
	if (FAILED(rval))
	{
		fprintf(stderr, "%s(): lpDD->SetCooperativeLevel() failed.\n", __func__);
		// TODO: Error handling code.
	}
	else
	{
		fprintf(stderr, "%s(): lpDD->SetCooperativeLevel() succeeded.\n", __func__);
	}
}
