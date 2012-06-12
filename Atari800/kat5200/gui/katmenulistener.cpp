/******************************************************************************
*
* FILENAME: katmenulistener.cpp
*
* DESCRIPTION:  Class that contains actions for menu items
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/31/06  bberlin      Creation
******************************************************************************/
#include "filebrowser.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../core/console.h"
	#include "../interface/kconfig.h"
	#include "../interface/states.h"
}

void KatMenuListener::action(const gcn::ActionEvent &actionEvent)
{
	t_config * p_config = config_get_ptr();
	struct atari *p_console = console_get_ptr();

	if ( actionEvent.getId() == "LoadBrowser" )
	{
		if (mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			strcpy ( p_config->rom_file, mGuiMain->getFileBrowser()->getResult().c_str() );
			mGuiMain->getFileBrowser()->hide();
			mGuiMain->launch( 0, 0 );
		}
	}
	else if ( actionEvent.getId() == "SaveState" )
	{
		if ( mGuiMain->getProfilePopup()->getCancelPressed() == false ) {
			if ( save_state ( mGuiMain->getProfilePopup()->getProfile().c_str(), p_console ) )
				mGuiMain->getInfoPopup()->show( "Error Saving State" );
		}
	}
	else if ( actionEvent.getId() == "LoadState" )
	{
		if ( mGuiMain->getProfilePopup()->getCancelPressed() == false ) {
			strcpy ( p_config->state, mGuiMain->getProfilePopup()->getProfile().c_str() );
			mGuiMain->getProfilePopup()->hide();
			if ( recall_state ( p_config->state, console_get_ptr() ) ) {
				mGuiMain->getInfoPopup()->deleteActions();
				mGuiMain->getInfoPopup()->show( "Error Loading State" );
				return;
			}
			mGuiMain->launch( 1, 0 );
		}
	}
}
