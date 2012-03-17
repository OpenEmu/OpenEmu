/**
 * Gens: Option adjustment functions.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifndef GENS_OPTIONS_HPP
#define GENS_OPTIONS_HPP

#ifdef __cplusplus

#include <stdint.h>

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

class Options
{
	public:
		static bool spriteLimit(void);
		static void setSpriteLimit(const bool newSpriteLimit);
		
		static int saveSlot(void);
		static void setSaveSlot(const int newSaveSlot);
		
		static int frameSkip(void);
		static void setFrameSkip(const int newFrameSkip);
		
#ifdef GENS_DEBUGGER
		static DEBUG_MODE debugMode(void);
		static void setDebugMode(const DEBUG_MODE newDebugMode);
#endif /* GENS_DEBUGGER */
		
		static int country(void);
		static void setCountry(const int newCountry);
		
		/** Sound **/
		
		static bool soundEnable(void);
		static int setSoundEnable(const bool newSoundEnable);
		
		static bool soundStereo(void);
		static void setSoundStereo(const bool newSoundStereo);
		
		static bool soundZ80(void);
		static void setSoundZ80(const bool newSoundZ80);
		
		static bool soundYM2612(void);
		static void setSoundYM2612(const bool newSoundYM2612);
		
		static bool soundYM2612_Improved(void);
		static void setSoundYM2612_Improved(const bool newSoundYM2612_Improved);
		
		static bool soundDAC(void);
		static void setSoundDAC(const bool newSoundDAC);
		
		static bool soundDAC_Improved(void);
		static void setSoundDAC_Improved(const bool newSoundDAC_Improved);
		
		static bool soundPSG(void);
		static void setSoundPSG(const bool newSoundPSG);
		
		static bool soundPSG_Sine(void);
		static void setSoundPSG_Sine(const bool newSoundPSG_Sine);
		
		static bool soundPCM(void);
		static void setSoundPCM(const bool newSoundPCM);
		
		static bool soundPWM(void);
		static void setSoundPWM(const bool newSoundPWM);
		
		static bool soundCDDA(void);
		static void setSoundCDDA(const bool newSoundCDDA);
		
		static int soundSampleRate(void);
		static void setSoundSampleRate(const int newRate);
		
		/** SegaCD **/
		
		static bool segaCD_PerfectSync(void);
		static void setSegaCD_PerfectSync(const bool newSegaCD_PerfectSync);
		
		static int segaCD_SRAMSize(void);
		static void setSegaCD_SRAMSize(const int newSegaCD_SRAMSize);
		
		/** Graphics **/
		
		static bool fastBlur(void);
		static void setFastBlur(const bool newFastBlur);
		
		static uint8_t stretch(void);
		static void setStretch(const uint8_t newStretch);
		
		static bool vsync(void);
		static void setVSync(const bool newVSync);
		
#ifdef GENS_OS_WIN32
		static bool swRender(void);
		static void setSwRender(const bool newSwRender);
#endif /* GENS_OS_WIN32 */
		
#ifdef GENS_OPENGL
		static bool openGL(void);
		static void setOpenGL(const bool newOpenGL);
		
		static void setOpenGL_Resolution(int w, int h);
#endif /* GENS_OPENGL */
		
		/** Functions not related to options. **
		** TODO: Move these somewhere else. **/
		
		static void systemReset(void);
		static void setGameName(void);
};

#endif /* __cplusplus */

#endif /* GENS_OPTIONS_HPP */
