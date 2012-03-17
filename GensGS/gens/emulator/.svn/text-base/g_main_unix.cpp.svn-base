/**
 * Gens: Main loop. (Linux specific code)
 */


#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g_main.hpp"
#include "g_main_unix.hpp"

#include "g_palette.h"
#include "gens_ui.hpp"
#include "g_md.hpp"
#include "g_32x.hpp"

#include "parse.hpp"
#include "options.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "util/file/config_file.hpp"
#include "util/file/ggenie.h"
#include "util/sound/gym.hpp"

#include "video/v_draw_sdl.hpp"
#include "input/input_sdl.hpp"
#include "audio/audio_sdl.hpp"

#include "gens/gens_window_sync.hpp"

#include "port/timer.h"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


/**
 * Get_Save_Path(): Create the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	strncpy(buf, getenv ("HOME"), n);
	strcat(buf, "/.gens/");
}


/**
 * Create_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	mkdir(dir, 0700);
}


/**
 * main(): Main loop.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Error code.
 */
int main(int argc, char *argv[])
{
	// Initialize the timer.
	// TODO: Make this unnecessary.
	init_timer();
	
	// Initialize the drawing object.
	// TODO: Select VDraw_SDL(), VDraw_SDL_GL(), or VDraw_DDraw() depending on other factors.
	draw = new VDraw_SDL();
	
	// Initialize the input object.
	input = new Input_SDL();
	
	// Initialize the audio object.
	audio = new Audio_SDL();
	
	// Initialize the Settings struct.
	Init_Settings();
	
	// Initialize the Game Genie array.
	Init_GameGenie();
	
	// Parse command line arguments.
	parseArgs(argc, argv);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
	// Initialize the UI.
	GensUI::init(argc, argv);
	
	// not yet finished (? - wryun)
	//initializeConsoleRomsView();
	
#ifdef GENS_OPENGL
	// Check if OpenGL needs to be enabled.
	// This also initializes SDL or SDL+GL.
	Options::setOpenGL(Video.OpenGL);
#else
	// Initialize SDL.
	draw->Init_Video();
#endif
	
	if (strcmp(PathNames.Start_Rom, "") != 0)
	{
		if (ROM::openROM(PathNames.Start_Rom) == -1)
		{
			fprintf(stderr, "Failed to load %s\n", PathNames.Start_Rom);
		}
	}
	
	// Update the UI.
	GensUI::update();
	
	int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
	if (!draw->setRender(rendMode))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (!draw->setRender(0))
		{
			// Cannot initialize normal mode.
			fprintf(stderr, "FATAL ERROR: Cannot initialize any renderers.\n");
			return 1;
		}
	}
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	// Run the Gens Main Loop.
	GensMainLoop();
	
	Get_Save_Path(Str_Tmp, GENS_PATH_MAX);
	strcat(Str_Tmp, "gens.cfg");
	Config::save(Str_Tmp);
	
	End_All();
	return 0;
}
