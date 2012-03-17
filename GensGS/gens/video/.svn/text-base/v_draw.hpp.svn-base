/**
 * Gens: Video Drawing base class.
 */

#ifndef GENS_V_DRAW_HPP
#define GENS_V_DRAW_HPP

// Font for onscreen messages
#include "C64_charset.h"

#ifdef __cplusplus

#include <stdint.h>

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// CPU flags
#include "gens_core/misc/cpuflags.h"

// MDP Host Services
#include "plugins/mdp_host.h"

// C++ includes
#include <string>
using std::string;

// Constants for m_MsgStyle and m_FPSStyle.
// These match the #define's in gens_core/misc/misc.h.
const unsigned char STYLE_EMU_MODE	= 0x01;
const unsigned char STYLE_COLOR_WHITE	= 0x00;
const unsigned char STYLE_COLOR_BLUE	= 0x02;
const unsigned char STYLE_COLOR_GREEN	= 0x04;
const unsigned char STYLE_COLOR_RED	= 0x06;
const unsigned char STYLE_TRANSPARENT	= 0x08;
const unsigned char STYLE_DOUBLESIZE	= 0x10;


class VDraw
{
	public:
		VDraw();
		VDraw(VDraw *oldDraw);
		virtual ~VDraw() = 0;
		
		virtual int Init_Video(void) = 0;
		virtual void End_Video(void) = 0;
		
		// Initialize the graphics subsystem.
		virtual int Init_Subsystem(void) = 0;
		
		// Shut down the graphics subsystem.
		virtual int Shut_Down(void) = 0;
		
		// Clear the screen.
		virtual void clearScreen(void) = 0;
		//virtual void Clear_Primary_Screen(void) = 0;
		//virtual void Clear_Back_Screen(void) = 0;
		
		// Flip the screen buffer.
		int flip(void);
		
		// Sets the bpp value.
		void setBpp(const int newBpp, const bool resetVideo = true);
		
		// Refresh the video subsystem.
		void Refresh_Video(void);
		
		// Set the rendering mode.
		int setRender(const int newMode, const bool forceUpdate = true);
		
		// Update VSync value.
		virtual void updateVSync(const bool data = false) = 0;
		
		// Write text to the screen.
		void writeText(const string& msg, const unsigned short duration);
		
		// Properties
		uint8_t stretch(void);
		void setStretch(const uint8_t newStretch);
		bool swRender(void);
		void setSwRender(const bool newSwRender);
		int scale(void);
		void setScale(const int newScale);
		bool msgEnabled(void);
		void setMsgEnabled(const bool newMsgEnable);
		bool fpsEnabled(void);
		void setFPSEnabled(const bool newFPSEnable);
		bool fullScreen(void);
		void setFullScreen(const bool newFullScreen);
		bool fastBlur(void);
		void setFastBlur(const bool newFastBlur);
		
		// Style properties
		unsigned char msgStyle(void);
		void setMsgStyle(const unsigned char newMsgStyle);
		unsigned char fpsStyle(void);
		void setFPSStyle(const unsigned char newFPSStyle);
		unsigned char introEffectColor(void);
		void setIntroEffectColor(const unsigned char newIntroEffectColor);
		
		// TODO: Move these functions out of v_draw.cpp.
		static int Show_Genesis_Screen(void);
		
		struct VDraw_Style
		{
			unsigned char style;
			unsigned char color;	// STYLE_COLOR_* values
			unsigned int dotColor;	// Actual RGB color value
			bool doubleSize;
			bool transparent;
		};
		
		// Stretch constants.
		static const uint8_t STRETCH_NONE = 0x00;
		static const uint8_t STRETCH_H    = 0x01;
		static const uint8_t STRETCH_V    = 0x02;
		static const uint8_t STRETCH_FULL = 0x03;
		
		// Render functions
		// TODO: Make these properties.
		MDP_Render_Fn m_BlitFS;
		MDP_Render_Fn m_BlitW;
		
	protected:
		// Called if initialization fails.
		void Init_Fail(const char *err);
		
		// Screen scaling.
		int m_scale;
		
		// Stretch option.
		uint8_t m_Stretch;
		
		// Flip the screen buffer. (Renderer-specific function)
		virtual int flipInternal(void) = 0;
		
		// Adjust stretch parameters.
		virtual void stretchAdjustInternal(void) { };
		
		// Update the renderer.
		virtual void updateRenderer(void) = 0;
		
		// Draw a line of text on the screen.
		void drawText(void *screen, const int fullW, const int w, const int h,
			      const char *msg, const VDraw_Style& style,
			      const bool adjustForScreenSize = true);
		
		template<typename pixel>
		void drawText_int(pixel *screen, const int fullW, const int w, const int h,
				  const char *msg, const pixel transparentMask, const VDraw_Style& style,
				  const bool adjustForScreenSize = true);
		
		// Calculates the text style values.
		void calcTextStyle(void);
		
		// Transparency mask
		unsigned int m_Transparency_Mask;
		
		// FPS counter
		bool m_FPSEnabled; // Is the FPS counter currently enabled by the user?
		float m_FPS, m_FPS_Frames[8];
		unsigned int m_FPS_OldTime, m_FPS_ViewFPS, m_FPS_IndexFPS;
		unsigned int m_FPS_FreqCPU[2], m_FPS_NewTime[2];
		
		// On-screen message
		bool m_MsgEnabled; // Is the message function enabled by the user?
		string m_MsgText;
		bool m_MsgVisible; // Is the message currently visible onscreen?
		unsigned int m_MsgTime;
		
		// Style values
		VDraw_Style m_MsgStyle;
		VDraw_Style m_FPSStyle;
		
		// Intro effect color
		unsigned char m_IntroEffectColor;
		
		// Full Screen
		bool m_FullScreen;
		
		// Screen HBorder.
		// Usually 64 in 256x224 mode. (320 - 256 == 64)
		unsigned char m_HBorder, m_HBorder_Old;
		
		// Current border color.
		unsigned short m_BorderColor_16B;
		unsigned int m_BorderColor_32B;
		
		// Miscellaneous settings.
		bool m_swRender;
		bool m_fastBlur;
		
		// Render Plugin information.
		static MDP_Render_Info_t m_rInfo;
		
		// 16-bit to 32-bit conversion table.
		int *m_LUT16to32;
		
		// Internal surface for rendering the 16-bit temporary image.
		uint16_t *m_tmp16img;
		int m_tmp16img_scale;
		int m_tmp16img_pitch;
		void Render_16to32(uint32_t *dest, uint16_t *src,
				   int width, int height,
				   int pitchDest, int pitchSrc);
		
		// Win32 stuff
		virtual int reinitGensWindow(void);
};

#endif /* __cplusplus */

#endif /* GENS_V_DRAW_HPP */
