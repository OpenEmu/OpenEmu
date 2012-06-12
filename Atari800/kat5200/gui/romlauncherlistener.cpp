/******************************************************************************
*
* FILENAME: romlauncherlistener.cpp
*
* DESCRIPTION:  Class that contains actions for the Launcher window
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/16/06  bberlin      Creation
******************************************************************************/
#include "filebrowser.hpp"
#include "guimain.hpp"

extern "C" {
	#include <sys/stat.h>
	#include "../interface/kconfig.h"
	#include "../interface/util.h"
	#include "../interface/media.h"
}

void RomLauncherListener::action(const gcn::ActionEvent &actionEvent)
{
	t_config * p_config = config_get_ptr();
	FileBrowser *browser = (FileBrowser *) actionEvent.getSource();
	GenericMessage *message = (GenericMessage *) actionEvent.getSource();
	static int scan_subs = 0;
	static int bios_check = 0;
	static int data[6];
	t_media tmp_cart;
	char dir[1100];

	if ( actionEvent.getId() == "Popup" ) {
		if ( message->getButton() == 0 )
			bios_check = 1;
		else
			bios_check = 0;
		message->hide();
		message->show( "Scanning... (Application may appear to freeze)","",false,"",false,"",false, false );
		media_clear_filenames ( p_config->machine_type );
		strcpy ( dir, mGuiMain->getFileBrowser()->getResult().c_str() );
		data[0] = p_config->machine_type;
		data[1] = scan_subs;
		data[2] = bios_check;
		mGuiMain->Message ( GuiMain::MSG_SCAN, data );
	}
	else if ( actionEvent.getId() == "AddBrowser" ) {
		if ( browser->getCancelPressed() == false ) {
			browser->hide();
			strcpy ( dir, mGuiMain->getFileBrowser()->getResult().c_str() );

			media_create_cart_from_file ( dir, &tmp_cart );
			media_save_settings ( &tmp_cart );
			mGuiMain->updateLauncherList();
		}
	}
	else if ( actionEvent.getId() == "DeletePopup" ) {
		message->hide();
		if ( message->getButton() == 0 ) {
			tmp_cart.crc = mGuiMain->getLauncherSelected();
			media_delete_crc ( tmp_cart.crc );
			mGuiMain->updateLauncherList();
		}
	}
	else if ( actionEvent.getId() == "ScanPopup" ) {
		if ( mGuiMain->getMessagePopup()->getButton() == 0 )
			scan_subs = 1;
		else
			scan_subs = 0;
		mGuiMain->getMessagePopup()->setActionEventId("Popup");
		mGuiMain->getMessagePopup()->show( "Set BIOS files if found?","Yes",true,"No",true,"",false, false );
	}
	else {
		if ( browser->getCancelPressed() == false ) {
			browser->hide();
			mGuiMain->getMessagePopup()->deleteActions();
			mGuiMain->getMessagePopup()->setActionEventId("ScanPopup");
			mGuiMain->getMessagePopup()->addActionListener(this);
			mGuiMain->getMessagePopup()->show( "Scan subdirectories as well?","Yes",true,"No",true,"",false, false );
		}
	}
}
