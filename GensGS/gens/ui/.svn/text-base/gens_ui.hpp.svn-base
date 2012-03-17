/***************************************************************************
 * Gens: Common UI functions.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_UI_HPP
#define GENS_UI_HPP

#include "emulator/gens.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	AnyFile		= 0,
	ROMFile		= 1,
	SavestateFile	= 2,
	CDImage		= 3,
	ConfigFile	= 4,
	GYMFile		= 5,
} FileFilterType;

/**
 * Win32_ClearSoundBuffer(): Clear the sound buffer.
 * NOTE: TEMPORARY wrapper function for C files.
 * Remove this when all the windows are converted to C++.
 */
void Win32_ClearSoundBuffer(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>
using std::string;

class GensUI
{
	public:
		// The following functions are implemented by ui/[ui]/gens_ui_[ui].cpp.
		static void init(int argc, char *argv[]);
		static void update(void);
		static void sleep(const int ms, const bool noUpdate = false);
		
		static void setWindowTitle(const string& title);
		static void setWindowVisibility(const bool visibility);
		
		static string openFile(const string& title, const string& initFile,
				       const FileFilterType filterType, void* owner = NULL);
		static string saveFile(const string& title, const string& initFile,
				       const FileFilterType filterType, void* owner = NULL);
		static string selectDir(const string& title, const string& initDir, void* owner = NULL);
		
		// The following functions are implemented by ui/gens_ui.cpp.
		static void setWindowTitle_Idle(void);
		static void setWindowTitle_Game(const string& system, const string& game,
						const string& emptyGameName = "");
		static void setWindowTitle_Init(const string& system, const bool reinit);
		
		// Message box styles for GensUI::msgBox().
		// Format: 00000000 00000000 00000000 BBBBIIII
		// IIII == Icon
		// BBBB == Buttons
		
		// Buttons
		static const unsigned int MSGBOX_BUTTONS_MASK		= 0x0000000F;
		static const unsigned int MSGBOX_BUTTONS_OK		= (0 << 0);
		static const unsigned int MSGBOX_BUTTONS_OK_CANCEL	= (1 << 0);
		static const unsigned int MSGBOX_BUTTONS_YES_NO		= (3 << 0);
		
		// Icons
		static const unsigned int MSGBOX_ICON_MASK		= 0x000000F0;
		static const unsigned int MSGBOX_ICON_NONE		= (0 << 4);
		static const unsigned int MSGBOX_ICON_ERROR		= (1 << 4);
		static const unsigned int MSGBOX_ICON_QUESTION		= (2 << 4);
		static const unsigned int MSGBOX_ICON_WARNING 		= (3 << 4);
		static const unsigned int MSGBOX_ICON_INFO		= (4 << 4);
		
		// Responses
		enum MsgBox_Response
		{
			MSGBOX_RESPONSE_OK	= 0,
			MSGBOX_RESPONSE_CANCEL	= 1,
			MSGBOX_RESPONSE_YES	= 2,
			MSGBOX_RESPONSE_NO	= 3,
		};
		
		// msgBox function
		static MsgBox_Response msgBox(const string& msg, const string& title = GENS_APPNAME,
					      const unsigned int style = MSGBOX_ICON_INFO | MSGBOX_BUTTONS_OK,
					      void* owner = NULL);
};

#endif /* __cplusplus */

#endif /* GENS_UI_HPP */
