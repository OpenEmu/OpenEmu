/**
 * Gens: Input handler.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>

#include "g_input.hpp"
#include "g_main.hpp"
#include "g_mcd.hpp"
#include "options.hpp"
#include "util/file/save.hpp"
#include "util/sound/gym.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/gfx/imageutil.hpp"

// Sound includes.
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"

// VDP
#include "gens_core/vdp/vdp_rend.h"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

#include "gens/gens_window_sync.hpp"
#include "gens_ui.hpp"

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// Due to bugs with SDL and GTK, modifier state has to be tracked manually.
// TODO: Shift-A works, but if shift is still held down and B is pressed, nothing shows up on SDL.
// TODO: This isn't actually a bug with SDL/GTK - it's an issue with keysnooping...
static int mod = 0;


/**
 * Input_KeyDown(): Check if a key is pressed.
 * @param key Keycode.
 */
void Input_KeyDown(int key)
{
#ifdef GENS_OS_UNIX
	string filename;
	
	switch (key)
	{
		case GENS_KEY_LCTRL:
			mod |= GENS_KMOD_LCTRL;
			break;
		case GENS_KEY_RCTRL:
			mod |= GENS_KMOD_RCTRL;
			break;
		case GENS_KEY_LALT:
			mod |= GENS_KMOD_LALT;
			break;
		case GENS_KEY_RALT:
			mod |= GENS_KMOD_RALT;
			break;
		case GENS_KEY_LSHIFT:
			mod |= GENS_KMOD_LSHIFT;
			break;
		case GENS_KEY_RSHIFT:
			mod |= GENS_KMOD_RSHIFT;
			break;
		case GENS_KEY_ESCAPE:
			if (Quick_Exit)
				close_gens();
			if (Debug)
			{
				Options::setDebugMode(DEBUG_NONE);
				Paused = 0;
				Sync_Gens_Window_CPUMenu();
			}
			else if (Paused)
			{
				Paused = 0;
			}
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio->clearSoundBuffer();
			}
			break;
		
		case GENS_KEY_PAUSE:
			if (Paused)
				Paused = 0;
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio->clearSoundBuffer();
			}
			break;
		
		case GENS_KEY_BACKSPACE:
			if (draw->fullScreen() && (mod & KMOD_SHIFT))
			{
				audio->clearSoundBuffer();
				ImageUtil::screenShot();
			}
			break;
		
		case GENS_KEY_TAB:
			Options::systemReset();
			break;
		
		case GENS_KEY_RETURN:
			if (mod & GENS_KMOD_ALT)
			{
				if (draw->fullScreen())
				{
					draw->setFullScreen(!draw->fullScreen());
					Sync_Gens_Window_GraphicsMenu();
				}
				
				// Reset the modifier key value to prevent Alt from getting "stuck".
				mod = 0;
			}
			break;
		
		case GENS_KEY_F1:
			fast_forward = 1;
			break;
		
		case GENS_KEY_F2:
			if (mod & GENS_KMOD_SHIFT)
			{
				Options::setStretch((Options::stretch() + 1) % 4);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (!mod)
			{
				Options::setFrameSkip(-1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F3:
			if (draw->fullScreen() && (mod & GENS_KMOD_SHIFT))
			{
				int newVSync = !(draw->fullScreen() ? Video.VSync_FS : Video.VSync_W);
				Options::setVSync(newVSync);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (!mod)
			{
				if (Options::frameSkip() == -1)
				{
					Options::setFrameSkip(0);
					Sync_Gens_Window_GraphicsMenu();
				}
				else if (Options::frameSkip() > 0)
				{
					Options::setFrameSkip(Options::frameSkip() - 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_F4:
			if (!mod)
			{
				if (Options::frameSkip() == -1)
				{
					Options::setFrameSkip(1);
					Sync_Gens_Window_GraphicsMenu();
				}
				else if (Options::frameSkip() < 8)
				{
					Options::setFrameSkip(Options::frameSkip() + 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_F5:
			if (!draw->fullScreen())
				break;
			
			//if (Check_If_Kaillera_Running()) return 0;
			
			if (mod == KMOD_SHIFT)
			{
				filename = Savestate::selectFile(true, State_Dir);
				if (!filename.empty())
					Savestate::saveState(filename);
			}
			else if (!mod)
			{
				filename = Savestate::getStateFilename();
				Savestate::saveState(filename);
			}
			break;
		
		case GENS_KEY_F6:
			if (!mod)
			{
				Options::setSaveSlot((Options::saveSlot() + 9) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F7:
			if (!mod)
			{
				Options::setSaveSlot((Options::saveSlot() + 1) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F8:
			if (!draw->fullScreen())
				break;
			
			//if (Check_If_Kaillera_Running()) return 0;
			
			if (mod == KMOD_SHIFT)
			{
				filename = Savestate::selectFile(false, State_Dir);
				if (!filename.empty())
					Savestate::loadState(filename);
			}
			else if (!mod)
			{
				filename = Savestate::getStateFilename();
				Savestate::loadState(filename);
			}
			break;
		
		case GENS_KEY_F9:
			#ifdef GENS_OS_WIN32 // TODO: Implement SW Render options on SDL?
			if (mod & GENS_KMOD_SHIFT)
				Options::setSwRender(!Options::swRender());
			else //if (!mod)
			#endif /* GENS_OS_WIN32 */
				Options::setFastBlur(!Options::fastBlur());
			break;
		
		case GENS_KEY_F10:
			if (mod & GENS_KMOD_SHIFT)
			{
				Options::setSoundDAC_Improved(!Options::soundDAC_Improved());
				Sync_Gens_Window_SoundMenu();
			}
			else if (draw->fullScreen() && !mod)
			{
				draw->setFPSEnabled(!draw->fpsEnabled());
			}
			break;
		
		case GENS_KEY_F11:
			if (mod & GENS_KMOD_SHIFT)
			{
				Options::setSoundPSG_Sine(!Options::soundPSG_Sine());
				Sync_Gens_Window_SoundMenu();
			}
			else //if (!mod)
			{
				int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
				if (rendMode > 0)
				{
					draw->setRender(rendMode - 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_F12:
			if (mod & GENS_KMOD_SHIFT)
			{
				Options::setSoundYM2612_Improved(!Options::soundYM2612_Improved());
				Sync_Gens_Window_SoundMenu();
			}
			else //if (!mod)
			{
				int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
				if (rendMode < (PluginMgr::vRenderPlugins.size() - 1))
				{
					draw->setRender(rendMode + 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_0:
		case GENS_KEY_1:
		case GENS_KEY_2:
		case GENS_KEY_3:
		case GENS_KEY_4:
		case GENS_KEY_5:
		case GENS_KEY_6:
		case GENS_KEY_7:
		case GENS_KEY_8:
		case GENS_KEY_9:
			if (!mod)
			{
				Options::setSaveSlot(key - GENS_KEY_0);
				Sync_Gens_Window_FileMenu();
			}
			else if (key != GENS_KEY_0 && (mod & GENS_KMOD_CTRL))
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (audio->playingGYM())
					Stop_Play_GYM();
				
				if (strlen(Recent_Rom[key - GENS_KEY_1]) > 0)
					ROM::openROM(Recent_Rom[key - GENS_KEY_1]);
				
				Sync_Gens_Window();
			}
			break;
		
#ifdef GENS_CDROM
		case GENS_KEY_b:
			if (draw->fullScreen() && (mod & GENS_KMOD_CTRL))
			{
				// Ctrl-B: Boot CD
				if (Num_CD_Drive == 0)
					return;	// return 1;
				//if (Check_If_Kaillera_Running()) return 0;
				if (audio->playingGYM())
					Stop_Play_GYM();
				ROM::freeROM(Game);	// Don't forget it !
				SegaCD_Started = Init_SegaCD(NULL);
			}
			break;
#endif /* GENS_CDROM */
		
		/*
		case GENS_KEY_w:
			if (mod & GENS_KMOD_CTRL)
			{
				if (Sound_Initialised)
					Clear_Sound_Buffer ();
				Debug = 0;
				if (Net_Play)
				{
					//if (Full_Screen)
						//Set_Render (0, -1, 1);
					Sync_Gens_Window();
				}
				Free_Rom (Game);
			}
			break;
		*/
		
		/*
		case GENS_KEY_f:
			if (mod & GENS_KMOD_CTRL)
				gl_linear_filter = !gl_linear_filter;
			break;
		*/
		
		/* TODO: Fix MINIMIZE.
		case GENS_KEY_g:
			if (mod & GENS_KMOD_CTRL)
			{
				//if (Check_If_Kaillera_Running()) return 0;                                            
				MINIMIZE;
				// TODO: Re-enable this when the GTK+ GUI is rewritten.
				//open_game_genie();
			}
			break;
		*/
		
		/*
		case GENS_KEY_o:
			if (mod & GENS_KMOD_CTRL)
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				MINIMIZE;
				Get_Rom();
			}
			break;
		*/
		
		/* TODO: Fix MINIMIZE.
		case GENS_KEY_p:
			if (mod & (GENS_KMOD_CTRL | GENS_KMOD_SHIFT))
			{
				if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
				{
					MINIMIZE;
					if (GYM_Playing)
						Stop_Play_GYM();
					else
						Start_Play_GYM();
					Sync_Gens_Window_SoundMenu();
				}
			}
			break;
		*/
		
		case GENS_KEY_q:
			if (draw->fullScreen() && (mod & KMOD_CTRL))
				close_gens();
			break;
		
#ifdef GENS_OPENGL
		case GENS_KEY_r:
			if (draw->fullScreen() && (mod & GENS_KMOD_SHIFT))
			{
				Options::setOpenGL(!Options::openGL());
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
#endif
		
#ifdef GENS_CDROM
		case GENS_KEY_v:
			if (mod & GENS_KMOD_CTRL)
			{
				if (SegaCD_Started)
					Change_CD();
			}
			break;
#endif /* GENS_CDROM */
		
		default:
			break;
	}
	
#ifdef GENS_DEBUGGER
	// If debugging, pass the key to the debug handler.
	if (Debug != DEBUG_NONE)
		Debug_Event(key, mod);
#endif /* GENS_DEBUGGER */

#endif /* GENS_OS_UNIX */
}


/**
 * Input_KeyUp(): Check if a key is released.
 * @param key Keycode.
 */
void Input_KeyUp(int key)
{
#ifdef GENS_OS_UNIX
	switch(key)
	{
		case GENS_KEY_LCTRL:
			mod &= ~GENS_KMOD_LCTRL;
			break;
		case GENS_KEY_RCTRL:
			mod &= ~GENS_KMOD_RCTRL;
			break;
		case GENS_KEY_LALT:
			mod &= ~GENS_KMOD_LALT;
			break;
		case GENS_KEY_RALT:
			mod &= ~GENS_KMOD_RALT;
			break;
		case GENS_KEY_LSHIFT:
			mod &= ~GENS_KMOD_LSHIFT;
			break;
		case GENS_KEY_RSHIFT:
			mod &= ~GENS_KMOD_RSHIFT;
			break;
		case GENS_KEY_F1:
			fast_forward = 0;
			break;
		default:
			break;
	}
#endif /* GENS_OS_UNIX */
}
