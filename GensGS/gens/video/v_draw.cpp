/**
 * Gens: Video Drawing base class.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "v_draw.hpp"

#include "emulator/g_md.hpp"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/cpuflags.h"
#include "gens_core/gfx/fastblur.hpp"
#include "emulator/g_main.hpp"
#include "emulator/g_palette.h"
#include "gens_core/vdp/vdp_io.h"

#ifndef GENS_OS_WIN32
#include "port/timer.h"
#endif /* GENS_OS_WIN32 */

// Miscellaneous Effects.
#include "v_effects.hpp"

// Inline video functions.
#include "v_inline.h"

// TODO: Add a wrapper call to sync the GraphicsMenu.
#include "gens/gens_window_sync.hpp"


/**
 * m_rInfo: Render Plugin information.
 */
MDP_Render_Info_t VDraw::m_rInfo;


VDraw::VDraw()
{
	// Initialize variables.
	m_scale = 1;
	m_BlitFS = NULL;
	m_BlitW = NULL;
	
	// Initialize the FPS counter.
	m_FPS = 0.0f;
	for (int i = 0; i < 8; i++)
		m_FPS_Frames[i] = 0.0f;
	
	m_FPSEnabled = false;
	m_FPS_OldTime = 0;
	m_FPS_ViewFPS = 0;
	m_FPS_IndexFPS = 0;
	m_FPS_FreqCPU[0] = 0;
	m_FPS_FreqCPU[1] = 0;
	m_FPS_NewTime[0] = 0;
	m_FPS_NewTime[1] = 0;
	m_FPSStyle.style = 0;
	
	// Initialze the onscreen message.
	m_MsgEnabled = true;
	m_MsgText = "";
	m_MsgVisible = false;
	m_MsgTime = 0;
	m_MsgStyle.style = 0;
	
	// Initialize the border variables.
	m_HBorder = 0;
	m_HBorder_Old = ~0;
	m_BorderColor_16B = ~0;
	m_BorderColor_32B = ~0;
	
	// Others.
	m_Stretch = false;
	m_IntroEffectColor = 7;
	m_FullScreen = false;
	m_fastBlur = false;
	
	// Initialize m_rInfo.
	m_rInfo.bpp = 0;
	m_rInfo.renderFlags = 0;
	
	// Set LUT16to32 to NULL initially.
	m_LUT16to32 = NULL;
	
	// Internal surface for rendering the 16-bit temporary image.
	m_tmp16img = NULL;
	m_tmp16img_scale = 0;
	m_tmp16img_pitch = 0;
	
	// Calculate the text style.
	calcTextStyle();
}

VDraw::VDraw(VDraw *oldDraw)
{
	// Initialize this VDraw based on an existing VDraw object.
	m_scale = oldDraw->scale();
	m_BlitFS = oldDraw->m_BlitFS;
	m_BlitW = oldDraw->m_BlitW;
	
	// Initialize the FPS counter.
	// TODO: Copy FPS variables from the other VDraw?
	m_FPS = 0.0f;
	for (int i = 0; i < 8; i++)
		m_FPS_Frames[i] = 0.0f;
	
	m_FPSEnabled = oldDraw->fpsEnabled();
	m_FPS_OldTime = 0;
	m_FPS_ViewFPS = 0;
	m_FPS_IndexFPS = 0;
	m_FPS_FreqCPU[0] = 0;
	m_FPS_FreqCPU[1] = 0;
	m_FPS_NewTime[0] = 0;
	m_FPS_NewTime[1] = 0;
	m_FPSStyle.style = oldDraw->fpsStyle();
	
	// Initialize the onscreen message.
	// TODO: Copy message variables from the other VDraw?
	m_MsgEnabled = oldDraw->msgEnabled();
	m_MsgText = "";
	m_MsgVisible = false;
	m_MsgTime = 0;
	m_MsgStyle.style = oldDraw->msgStyle();
	
	// Initialize the border variables.
	m_HBorder = 0;
	m_HBorder_Old = ~0;
	m_BorderColor_16B = ~0;
	m_BorderColor_32B = ~0;
	
	// Others.
	m_Stretch = oldDraw->stretch();
	m_IntroEffectColor = oldDraw->introEffectColor();
	m_FullScreen = oldDraw->fullScreen();
	m_fastBlur = oldDraw->fastBlur();
	
	// Initialize m_rInfo.
	m_rInfo.bpp = 0;
	m_rInfo.renderFlags = 0;
	
	// Set LUT16to32 to NULL initially.
	m_LUT16to32 = NULL;
	
	// Internal surface for rendering the 16-bit temporary image.
	m_tmp16img = NULL;
	m_tmp16img_scale = 0;
	m_tmp16img_pitch = 0;
	
	// Calculate the text style.
	calcTextStyle();
}

VDraw::~VDraw()
{
#if !defined(GENS_UI_OPENEMU)
	if (m_LUT16to32)
	{
		// Unreference LUT16to32.
		MDP_Host.unrefPtr(MDP_PTR_LUT16to32);
		m_LUT16to32 = NULL;
	}
#endif
	// Internal surface for rendering the 16-bit temporary image.
	if (m_tmp16img)
	{
		free(m_tmp16img);
		m_tmp16img = NULL;
		m_tmp16img_scale = 0;
		m_tmp16img_pitch = 0;
	}
}


/**
 * Init_Fail(): Called by a child class when an error occurs during initializatio.
 * @param err Error message.
 */
void VDraw::Init_Fail(const char *err)
{
	fprintf(stderr, "%s\n", err);
	exit(0);
}


/**
 * Show_Genesis_Screen(): Show the Genesis screen.
 */
int VDraw::Show_Genesis_Screen(void)
{
	Do_VDP_Only();
	//Flip();
	
	return 1;
}


/**
 * Flip(): Flip the screen buffer.
 * @return 1 on success; 0 on error.
 */
int VDraw::flip(void)
{
	// TODO: Print the message and/or FPS counter on the screen buffer only.
	// Don't print it on MD_Screen.
	// Otherwise, messages and the FPS counter show up in screenshots.
	
	// Temporary buffer for sprintf().
	char tmp[64];
	
	// Check if any effects need to be applied.
	// TODO: Make constnats for Intro_Style.
	if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
		if (Video.pauseTint && (!Active || Paused))
		{
			// Emulation is paused.
			Pause_Screen();
		}
	}
	else if (Intro_Style == 1)
	{
		// Gens logo effect. (TODO: This is broken!)
		Update_Gens_Logo();
	}
	else if (Intro_Style == 2)
	{
		// "Strange" effect. (TODO: This is broken!)
		Update_Crazy_Effect(m_IntroEffectColor);
	}
	else if (Intro_Style == 3)
	{
		// Genesis BIOS. (TODO: This is broken!)
		Do_Genesis_Frame();
	}
	else
	{
		// Blank screen.
		Clear_Screen_MD();
	}
	
	if (m_MsgVisible)
	{
		if (GetTickCount() > m_MsgTime)
		{
			m_MsgVisible = false;
			m_MsgText = "";
		}
	}
	else if (m_FPSEnabled && (Genesis_Started || _32X_Started || SegaCD_Started) && !Paused)
	{
		if (m_FPS_FreqCPU[0] > 1)	// accurate timer ok
		{
			if (++m_FPS_ViewFPS >= 16)
			{
#ifdef GENS_OS_WIN32
				QueryPerformanceCounter((LARGE_INTEGER*)m_FPS_NewTime);
#else
				QueryPerformanceCounter((long long*)m_FPS_NewTime);
#endif
				if (m_FPS_NewTime[0] != m_FPS_OldTime)
				{
					m_FPS = (float)(m_FPS_FreqCPU[0]) * 16.0f / (float)(m_FPS_NewTime[0] - m_FPS_OldTime);
					sprintf(tmp, "%.1f", m_FPS);
					m_MsgText = tmp;
				}
				else
				{
					// IT'S OVER 9000 FPS!!!111!11!1
					m_MsgText = ">9000";
				}
				
				m_FPS_OldTime = m_FPS_NewTime[0];
				m_FPS_ViewFPS = 0;
			}
		}
		else if (m_FPS_FreqCPU[0] == 1)	// accurate timer not supported
		{
			if (++m_FPS_ViewFPS >= 10)
			{
				m_FPS_NewTime[0] = GetTickCount();
				
				if (m_FPS_NewTime[0] != m_FPS_OldTime)
					m_FPS_Frames[m_FPS_IndexFPS] = 10000.0f / (float)(m_FPS_NewTime[0] - m_FPS_OldTime);
				else
					m_FPS_Frames[m_FPS_IndexFPS] = 2000;
				
				m_FPS_IndexFPS++;
				m_FPS_IndexFPS &= 7;
				m_FPS = 0.0f;
				
				for (unsigned char i = 0; i < 8; i++)
					m_FPS += m_FPS_Frames[i];
				
				m_FPS /= 8.0f;
				m_FPS_OldTime = m_FPS_NewTime[0];
				m_FPS_ViewFPS = 0;
			}
			sprintf(tmp, "%.1f", m_FPS);
			m_MsgText = tmp;
		}
		else
		{
#ifdef GENS_OS_WIN32
			QueryPerformanceFrequency((LARGE_INTEGER*)m_FPS_FreqCPU);
#else
			QueryPerformanceFrequency((long long*)m_FPS_FreqCPU);
#endif
			if (m_FPS_FreqCPU[0] == 0)
				m_FPS_FreqCPU[0] = 1;
			
			// TODO: WTF is this for?
			// Assuming it just clears the string...
			//sprintf(Info_String, "", FPS);
			m_MsgText = "";
		}
	}
	
	// Blur the screen if requested.
	if (m_fastBlur)
		Fast_Blur();
	
	// Check if the display width changed.
	m_HBorder_Old = m_HBorder;
	if (isFullXRes())
		m_HBorder = 0;	// 320x224
	else
		m_HBorder = 64;	// 256x224
	
	if (m_HBorder != m_HBorder_Old)
	{
		// Display width change. Adjust the stretch parameters.
		stretchAdjustInternal();
	}
	
	if (m_HBorder > m_HBorder_Old)
	{
		// New screen width is smaller than old screen width.
		// Clear the screen.
		clearScreen();
	}
	
	// Flip the screen buffer.
	return flipInternal();
}


template<typename pixel>
static inline void drawChar_1x(pixel *screen, const int fullW, const int x, const int y,
			       const VDraw::VDraw_Style& style, const pixel transparentMask,
			       const unsigned char ch)
{
	unsigned short cx, cy;
	pixel* screenPos;
	unsigned char cRow;
	
	screenPos = &screen[y*fullW + x];
	for (cy = 0; cy < 8; cy++)
	{
		// Each character is 8 bytes, with each row representing 8 dots.
		// A 1 indicates the dot is opaque, while a 0 indicates the dot is transparent.
		cRow = C64_charset[ch][cy];
		for (cx = 0; cx < 8; cx++)
		{
			if (cRow & 0x80)
			{
				// Dot is opaque. Draw it.
				// TODO: Original asm version had transparency in a separate function for performance.
				// See if that would actually help.
				if (!style.transparent)
					*screenPos = style.dotColor;
				else
					*screenPos = ((style.dotColor & transparentMask) >> 1) +
						     ((*screenPos & transparentMask) >> 1);
			}
			cRow <<= 1;
			screenPos++;
		}
		screenPos += (fullW - 8);
	}
}


template<typename pixel>
static inline void drawChar_2x(pixel *screen, const int fullW, const int x, const int y,
			       const VDraw::VDraw_Style& style, const pixel transparentMask,
			       const unsigned char ch)
{
	unsigned short cx, cy;
	pixel* screenPos;
	unsigned char cRow;
	
	screenPos = &screen[y*fullW + x];
	for (cy = 0; cy < 8; cy++)
	{
		// Each character is 8 bytes, with each row representing 8 dots.
		// A 1 indicates the dot is opaque, while a 0 indicates the dot is transparent.
		cRow = C64_charset[ch][cy];
		for (cx = 0; cx < 8; cx++)
		{
			if (cRow & 0x80)
			{
				// Dot is opaque. Draw it.
				// TODO: Original asm version had transparency in a separate function for performance.
				// See if that would actually help.
				if (!style.transparent)
				{
					*screenPos = style.dotColor;
					*(screenPos + 1) = style.dotColor;
					*(screenPos + fullW) = style.dotColor;
					*(screenPos + fullW + 1) = style.dotColor;
				}
				else
				{
					pixel trPix = (style.dotColor & transparentMask) >> 1;
					*screenPos = trPix + ((*screenPos & transparentMask) >> 1);
					*(screenPos + 1) = trPix + ((*(screenPos + 1) & transparentMask) >> 1);
					*(screenPos + fullW) = trPix + ((*(screenPos + fullW) & transparentMask) >> 1);
					*(screenPos + fullW + 1) = trPix + ((*(screenPos + fullW + 1) & transparentMask) >> 1);
				}
			}
			cRow <<= 1;
			screenPos += 2;
		}
		screenPos += (fullW*2 - 16);
	}
}


template<typename pixel>
void VDraw::drawText_int(pixel *screen, const int fullW, const int w, const int h,
			 const char *msg, const pixel transparentMask, const VDraw_Style& style,
			 const bool adjustForScreenSize)
{
	int msgLength, cPos;
	unsigned short linebreaks, msgWidth;
	unsigned short x, y, cx, cy;
	unsigned char charSize;
	
	// The message must be specified.
	if (!msg)
		return;
	
	// Character size
	if (style.doubleSize)
		charSize = 16;
	else
		charSize = 8;
	
	// Bottom-left of the screen.
	if (adjustForScreenSize)
	{
		// Adjust for screen size. (SDL/GL)
		x = ((m_HBorder / 2) * m_scale) + 8;
		y = h - (((240 - VDP_Num_Vis_Lines) / 2) * m_scale);
	}
	else
	{
		// Don't adjust for screen size. (DDraw)
		if (!m_FullScreen && Video.Render_W == 0)
		{
			// Hack for windowed 1x rendering.
			x = 8;
			y = VDP_Num_Vis_Lines * m_scale;
		}
		else if (m_FullScreen && Video.Render_FS == 0)
		{
			// Hacks for fullscreen 1x rendering.
			if (m_swRender)
			{
				x = ((m_HBorder / 2) * m_scale);
				y = VDP_Num_Vis_Lines + 8;
			}
			else
			{
				x = 8;
				y = VDP_Num_Vis_Lines;
			}
		}
		else
		{
			x = ((m_HBorder / 2) * m_scale);
			y = VDP_Num_Vis_Lines * m_scale;
		}
		
		if (m_scale > 1)
			y += (8 * m_scale);
	}
	
	// Move the text down by another 2px in 1x rendering.
	if ((!m_FullScreen && Video.Render_W == 0) ||
	    (m_FullScreen && Video.Render_FS == 0))
	{
		y += 2;
	}
	
	// Character size is 8x8 normal, 16x16 double.
	y -= (8 + charSize);
	
	// Get the message length.
	msgLength = strlen(msg);
	
	// Determine how many linebreaks are needed.
	msgWidth = w - 16 - (m_HBorder * m_scale);
	linebreaks = ((msgLength - 1) * charSize) / msgWidth;
	y -= (linebreaks * charSize);
	
	VDraw_Style textShadowStyle = style;
	textShadowStyle.dotColor = 0;
	
	cx = x; cy = y;
	for (cPos = 0; cPos < msgLength; cPos++)
	{
		if (style.doubleSize)
		{
			// TODO: Make text shadow an option.
			drawChar_2x(screen, fullW, cx+1, cy+1, textShadowStyle,
				    transparentMask, (unsigned char)msg[cPos]);
			
			drawChar_2x(screen, fullW, cx-1, cy-1, style,
				    transparentMask, (unsigned char)msg[cPos]);
		}
		else
		{
			// TODO: Make text shadow an option.
			drawChar_1x(screen, fullW, cx+1, cy+1, textShadowStyle,
				    transparentMask, (unsigned char)msg[cPos]);
			
			drawChar_1x(screen, fullW, cx, cy, style,
				    transparentMask, (unsigned char)msg[cPos]);
		}
		
		cx += charSize;
		if (cx - x >= msgWidth)
		{
			cx = x;
			cy += charSize;
		}
	}
}


void VDraw::drawText(void *screen, const int fullW, const int w, const int h,
		     const char *msg, const VDraw_Style& style,
		     const bool adjustForScreenSize)
{
	if (bppOut == 15 || bppOut == 16)
	{
		// 15/16-bit color.
		drawText_int((unsigned short*)screen, fullW, w, h, msg,
			     (unsigned short)m_Transparency_Mask, style,
			     adjustForScreenSize);
	}
	else //if (bppOut == 32)
	{
		// 32-bit color.
		drawText_int((unsigned int*)screen, fullW, w, h, msg,
			     m_Transparency_Mask, style,
			     adjustForScreenSize);
	}
}


static inline void calcTextStyle_int(VDraw::VDraw_Style& style)
{
	// Calculate the dot color.
	
	if (bppOut == 15)
	{
		if ((style.style & 0x07) == STYLE_COLOR_RED)
			style.dotColor = 0x7C00;
		else if ((style.style & 0x07) == STYLE_COLOR_GREEN)
			style.dotColor = 0x03E0;
		else if ((style.style & 0x07) == STYLE_COLOR_BLUE)
			style.dotColor = 0x001F;
		else //if ((style.style & 0x07) == STYLE_COLOR_WHITE)
			style.dotColor = 0x7FFF;
	}
	else if (bppOut == 16)
	{
		if ((style.style & 0x07) == STYLE_COLOR_RED)
			style.dotColor = 0xF800;
		else if ((style.style & 0x07) == STYLE_COLOR_GREEN)
			style.dotColor = 0x07E0;
		else if ((style.style & 0x07) == STYLE_COLOR_BLUE)
			style.dotColor = 0x001F;
		else //if ((style.style & 0x07) == STYLE_COLOR_WHITE)
			style.dotColor = 0xFFFF;
	}
	else //if (bppOut == 32)
	{
		if ((style.style & 0x07) == STYLE_COLOR_RED)
			style.dotColor = 0xFF0000;
		else if ((style.style & 0x07) == STYLE_COLOR_GREEN)
			style.dotColor = 0x00FF00;
		else if ((style.style & 0x07) == STYLE_COLOR_BLUE)
			style.dotColor = 0x0000FF;
		else //if ((style.style & 0x07) == STYLE_COLOR_WHITE)
			style.dotColor = 0xFFFFFF;
	}
	
	style.doubleSize = (style.style & STYLE_DOUBLESIZE);
	style.transparent = (style.style & STYLE_TRANSPARENT);
}


/**
 * calcTextStyle(): Calculates the text style values
 */
void VDraw::calcTextStyle(void)
{
	// Calculate the transparency mask.
	if (bppOut == 15)
		m_Transparency_Mask = 0x7BDE;
	else if (bppOut == 16)
		m_Transparency_Mask = 0xF7DE;
	else //if (bppOut == 32)
		m_Transparency_Mask = 0xFEFEFE;
	
	// Calculate the style values for FPS and Msg.
	calcTextStyle_int(m_FPSStyle);
	calcTextStyle_int(m_MsgStyle);
}


/**
 * setBpp(): Sets the bpp value.
 * @param newbpp New bpp value.
 * @param resetVideo If true, resets the video subsystem if bpp needs to be changed.
 */
void VDraw::setBpp(const int newBpp, const bool resetVideo)
{
	// If the new bpp is the same as the current bpp, don't do anything else.
	if (bppOut == newBpp)
		return;
	
	bppOut = newBpp;
	
	if (resetVideo)
	{
		End_Video();
		Init_Video();
	}
	
	// Reset the renderer.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	if (!setRender(rendMode, resetVideo))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (!setRender(0, resetVideo))
		{
			// Cannot initialize normal mode.
			fprintf(stderr, "%s: FATAL ERROR: Cannot initialize any renderers.\n", __func__);
			exit(1);
		}
	}
	
	// Recalculate palettes.
	Recalculate_Palettes();
	
	// Calculate the text style.
	calcTextStyle();
	
#if !(defined(GENS_UI_OPENEMU))
	// Synchronize the Graphics menu.
	Sync_Gens_Window_GraphicsMenu();
#endif
	
	// TODO: After switching color depths, the screen buffer isn't redrawn
	// until something's updated. Figure out how to trick the renderer
	// into updating anyway.
	
	// NOTE: This only seems to be a problem with 15-to-16 or 16-to-15 at the moment.
	
	// TODO: Figure out if 32-bit rendering still occurs in 15/16-bit mode and vice-versa.
}


/**
 * writeText(): Write text to the screen.
 * @param msg Message to write.
 * @param duration Duration for the message to appear, in milliseconds.
 */
void VDraw::writeText(const string& msg, const unsigned short duration)
{
	if (!m_MsgEnabled)
		return;
	m_MsgText = msg;
	m_MsgTime = GetTickCount() + duration;
	m_MsgVisible = true;
}


/**
 * Refresh_Video(): Refresh the video subsystem.
 */
void VDraw::Refresh_Video(void)
{
	// Reset the border color to make sure it's redrawn.
	m_BorderColor_16B = ~MD_Palette[0];
	m_BorderColor_32B = ~MD_Palette32[0];
	
	End_Video();
	Init_Video();
	stretchAdjustInternal();
}


/**
 * setRender(): Set the rendering mode.
 * @param newMode Rendering mode / filter.
 * @param forceUpdate If true, forces a renderer update.
 */
int VDraw::setRender(const int newMode, const bool forceUpdate)
{
#if !defined(GENS_UI_OPENEMU)
	if (PluginMgr::vRenderPlugins.size() == 0)
		return 0;
	
	int oldRend, *Rend;
	MDP_Render_Fn *rendFn;
	bool reinit = false;
	
	if (m_FullScreen)
	{
		Rend = &Video.Render_FS;
		oldRend = Video.Render_FS;
		rendFn = &m_BlitFS;
	}
	else
	{
		Rend = &Video.Render_W;
		oldRend = Video.Render_W;
		rendFn = &m_BlitW;
	}
	
	// Get the old scaling factor.
	const int oldScale = PluginMgr::getPluginFromID_Render(oldRend)->scale;
	
	// Checks if an invalid mode number was passed.
	if (newMode < 0 || newMode >= PluginMgr::vRenderPlugins.size())
	{
		// Invalid mode number.
		MESSAGE_NUM_L("Error: Render mode %d is not available.",
			      "Error: Render mode %d is not available.", newMode, 1500);
		return 0;
	}
	
	// Renderer function found.
	MDP_Render_t *rendPlugin = PluginMgr::getPluginFromID_Render(newMode);
	*rendFn = rendPlugin->blit;
	
	if (*Rend != newMode)
	{
		MESSAGE_STR_L("Render Mode: %s", "Render Mode: %s", rendPlugin->tag, 1500);
	}
	else
	{
		reinit = true;
	}
	
	// Set the new render mode number.
	*Rend = newMode;
	
	// Set the scaling value.
	m_scale = rendPlugin->scale;
	
	// Set the MD bpp output value.
	if (bppOut != 32)
	{
		// Not 32-bit color. Always use the destination surface color depth.
		bppMD = bppOut;
		m_rInfo.mdScreen = (void*)(&MD_Screen[8]);
	}
	else
	{
		if (rendPlugin->flags & MDP_RENDER_FLAG_SRC16DST32)
		{
			// Render plugin only supports 16-bit color.
			bppMD = 16;
			m_rInfo.mdScreen = (void*)(&MD_Screen[8]);
		}
		else
		{
			// MD surface should be the same color depth as the destination surface.
			bppMD = bppOut;
			m_rInfo.mdScreen = (void*)(&MD_Screen32[8]);
		}
	}
	
	// Set the source pitch.
	m_rInfo.srcPitch = 336 * (bppMD == 15 ? 2 : bppMD / 8);
	
	//if (Num>3 || Num<10)
	//Clear_Screen();
	// if( (Old_Rend==NORMAL && Num==DOUBLE)||(Old_Rend==DOUBLE && Num==NORMAL) ||Opengl)
	// this doesn't cover hq2x etc. properly. Let's just always refresh.
	
	// Update the renderer.
	if (forceUpdate && is_gens_running())
		updateRenderer();
	
	if ((reinit && forceUpdate) || (oldScale != m_scale))
	{
		// The Gens window must be reinitialized.
		return reinitGensWindow();
	}
#endif// #if !defined(GENS_UI_OPENEMU)
	return 1;
}


/**
 * reinitGensWindow(): Reinitialize the Gens window.
 * @return 1 on success; 0 on error.
 */
int VDraw::reinitGensWindow(void)
{
	// Does nothing by default...
	return 1;
}


/**
 * Render_16to32(): Convert a 16-bit color image to 32-bit color using the lookup table.
 * @param dest Destination surface.
 * @param src Source surface.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitchDest Pitch of the destination surface.
 * @param pitchSrc Pitch of the source surface.
 */
void VDraw::Render_16to32(uint32_t *dest, uint16_t *src,
			  int width, int height,
			  int pitchDest, int pitchSrc)
{
#if !defined(GENS_UI_OPENEMU)
	// Make sure the lookup table is referenced.
	if (!m_LUT16to32)
		m_LUT16to32 = static_cast<int*>(MDP_Host.refPtr(MDP_PTR_LUT16to32));
	
	const int pitchDestDiff = ((pitchDest / 4) - width);
	const int pitchSrcDiff = ((pitchSrc / 2) - width);
	
	// Process four pixels at a time.
	width >>= 2;
	
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			*(dest + 0) = m_LUT16to32[*(src + 0)];
			*(dest + 1) = m_LUT16to32[*(src + 1)];
			*(dest + 2) = m_LUT16to32[*(src + 2)];
			*(dest + 3) = m_LUT16to32[*(src + 3)];
			
			dest += 4;
			src += 4;
		}
		
		dest += pitchDestDiff;
		src += pitchSrcDiff;
	}
#endif
}


/** Properties **/


uint8_t VDraw::stretch(void)
{
	return m_Stretch;
}
void VDraw::setStretch(const uint8_t newStretch)
{
	if (m_Stretch == newStretch)
		return;
	
	if (newStretch > STRETCH_FULL)
	{
		// TODO: Throw an exception.
		return;
	}
	
	m_Stretch = newStretch;
	stretchAdjustInternal();
}


bool VDraw::swRender(void)
{
	return m_swRender;
}
void VDraw::setSwRender(const bool newSwRender)
{
	if (m_swRender == newSwRender)
		return;
	m_swRender = newSwRender;
	
	// TODO: Figure out what to do here...
}


int VDraw::scale(void)
{
	return m_scale;
}
void VDraw::setScale(const int newScale)
{
	if (m_scale == newScale)
		return;
	m_scale = newScale;
	
	// Adjust internal stretch.
	stretchAdjustInternal();
}


bool VDraw::msgEnabled(void)
{
	return m_MsgEnabled;
}
void VDraw::setMsgEnabled(const bool newMsgEnabled)
{
	if (m_MsgEnabled == newMsgEnabled)
		return;
	m_MsgEnabled = newMsgEnabled;
	
	// TODO: Figure out what to do here...
}


bool VDraw::fpsEnabled(void)
{
	return m_FPSEnabled;
}
void VDraw::setFPSEnabled(const bool newFPSEnabled)
{
	if (m_FPSEnabled == newFPSEnabled)
		return;
	m_FPSEnabled = newFPSEnabled;
	
	// TODO: Figure out what to do here...
}


// Style properties
unsigned char VDraw::msgStyle(void)
{
	return m_MsgStyle.style;
}
void VDraw::setMsgStyle(const unsigned char newMsgStyle)
{
	if (m_MsgStyle.style == newMsgStyle)
		return;
	m_MsgStyle.style = newMsgStyle;
	
	// Calculate the text style.
	calcTextStyle_int(m_MsgStyle);
}


unsigned char VDraw::fpsStyle(void)
{
	return m_FPSStyle.style;
}
void VDraw::setFPSStyle(const unsigned char newFPSStyle)
{
	if (m_FPSStyle.style == newFPSStyle)
		return;
	m_FPSStyle.style = newFPSStyle;
	
	// Calculate the text style.
	calcTextStyle_int(m_FPSStyle);
}


unsigned char VDraw::introEffectColor(void)
{
	return m_IntroEffectColor;
}
void VDraw::setIntroEffectColor(const unsigned char newIntroEffectColor)
{
	if (m_IntroEffectColor == newIntroEffectColor)
		return;
	if (/*newIntroEffectColor < 0 ||*/ newIntroEffectColor > 7)
		return;
	
	m_IntroEffectColor = newIntroEffectColor;
	
	// TODO: Figure out what to do here...
}


bool VDraw::fullScreen(void)
{
	return m_FullScreen;
}
void VDraw::setFullScreen(const bool newFullScreen)
{
	if (m_FullScreen == newFullScreen)
		return;
	
	m_FullScreen = newFullScreen;
	
	// Set the renderer.
	int newRend = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	setRender(newRend, false);
	
#ifdef GENS_OS_WIN32
	// Reinitialize the Gens window, if necessary.
	//if (Video.Render_FS == Video.Render_W)
		reinitGensWindow();
#endif
	
#ifndef GENS_OS_WIN32
	// Refresh the video subsystem, if Gens is running.
	if (is_gens_running())
		Refresh_Video();
#endif
}


bool VDraw::fastBlur(void)
{
	return m_fastBlur;
}
void VDraw::setFastBlur(const bool newFastBlur)
{
	if (m_fastBlur == newFastBlur)
		return;
	
	m_fastBlur = newFastBlur;
}
