/******************************************************************************
*
* FILENAME: romlaunchermenu.cpp
*
* DESCRIPTION:  This class will create a menu bar for the launcher
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   07/31/07  bberlin      Creation
******************************************************************************/
#include "romlauncher.hpp"
#include "guimain.hpp"

RomLauncherMenu::RomLauncherMenu ( RomLauncher *launcher )
{
	t_config *p_config = config_get_ptr();

	romLauncher = launcher;

	/*
	 * ROM Menu
	 */
	addMenuItem ( "", "mMedia", "Media Image", 0 );
	addMenuItem ( "mMedia", "mLaunch", "Launch", this );
	addMenuItem ( "mMedia", "mLaunchNoReset", "Launch - No Reset", this );
	addMenuItem ( "mMedia", "mAttach", "Attach", 0 );
	addMenuItem ( "mAttach", "mCart", "Cart Slot", this );
	addMenuItem ( "mAttach", "mCassette", "Cassette", this );
	addMenuItem ( "mAttach", "mDisk1", "Disk1", this );
	addMenuItem ( "mAttach", "mDisk2", "Disk2", this );
	addMenuItem ( "mAttach", "mDisk3", "Disk3", this );
	addMenuItem ( "mAttach", "mDisk4", "Disk4", this );
	addMenuItem ( "mAttach", "mDisk5", "Disk5", this );
	addMenuItem ( "mAttach", "mDisk6", "Disk6", this );
	addMenuItem ( "mAttach", "mDisk7", "Disk7", this );
	addMenuItem ( "mAttach", "mDisk8", "Disk8", this );
	addMenuItem ( "mMedia", "mDelete", "Delete", this );
	addMenuItem ( "mMedia", "mRemove", "Remove From Favorites", this );

	/*
	 * Group Menu
	 */
	addMenuItem ( "", "mGroup", "View", 0 );
	addMenuItem ( "mGroup", "mAll", "All", this );
	addMenuItem ( "mGroup", "mAvailable", "Available", this );
	addMenuItem ( "mGroup", "mUnAvailable", "Unavailable", this );
	addMenuItem ( "mGroup", "mFavorites", "Favorites", this );

	/*
	 * Misc Menu
	 */
	addMenuItem ( "", "mMisc", "Misc", 0 );
	addMenuItem ( "mMisc", "mAdd", "Add Media Image", this );
	addMenuItem ( "mMisc", "mScan", "Scan for Files", this );
	addMenuItem ( "mMisc", "mOptions", "Options", this );
}

RomLauncherMenu::~RomLauncherMenu ()
{
}

void RomLauncherMenu::setLauncher ( RomLauncher *launcher )
{
	romLauncher = launcher;
}

void RomLauncherMenu::setGroupChecked ( std::string id )
{
	setItemChecked ( "mAll", false );
	setItemChecked ( "mAvailable", false );
	setItemChecked ( "mUnAvailable", false );
	setItemChecked ( "mFavorites", false );

	setItemChecked ( id, true );
}

void RomLauncherMenu::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();

	if ( actionEvent.getId() == "mLaunch" )
		romLauncher->launch(0);

	else if ( actionEvent.getId() == "mLaunchNoReset" )
		romLauncher->launch(1);

	else if ( actionEvent.getId() == "mCart" )
	{
		romLauncher->load ( CART_MIN, 0 );
	}

	else if ( actionEvent.getId() == "mCassette" )
	{
		romLauncher->load ( CART_CASSETTE, 0 );
	}

	else if ( actionEvent.getId().find( "mDisk", 0 ) != std::string::npos )
	{
		romLauncher->load ( CART_FLOPPY, atoi(actionEvent.getId().c_str()+5)-1 );
	}

	else if ( actionEvent.getId() == "mDelete" )
	{
		mGuiMain->getMessagePopup()->deleteActions();
		mGuiMain->getMessagePopup()->setActionEventId("DeletePopup");
		mGuiMain->getMessagePopup()->addActionListener(&romLauncher->mRomListener);
		mGuiMain->getMessagePopup()->show( "Selected Entry will be permanently deleted. Are you sure?","Yes",true,"No",true,"",false, false );
	}

	else if ( actionEvent.getId() == "mRemove" )
		romLauncher->actOnFavorites();

	else if ( actionEvent.getId() == "mAll" )
		romLauncher->changeRomGroup ( 0x1 );

	else if ( actionEvent.getId() == "mAvailable" )
		romLauncher->changeRomGroup ( 0x2 );

	else if ( actionEvent.getId() == "mUnAvailable" )
		romLauncher->changeRomGroup ( 0x4 );

	else if ( actionEvent.getId() == "mFavorites" )
		romLauncher->changeRomGroup ( 0x8 );

	else if ( actionEvent.getId() == "mAdd" )
	{
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(&romLauncher->mRomListener);
		mGuiMain->getFileBrowser()->setActionEventId("AddBrowser");
		mGuiMain->getFileBrowser()->show("Load Rom File to Add",
		                                p_config->rom_file, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "mScan" )
	{
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(&romLauncher->mRomListener);
		mGuiMain->getFileBrowser()->setActionEventId("ScanBrowser");
		mGuiMain->getFileBrowser()->show("Navigate to Directory to Scan and Press Select",
		                                p_config->rom_file, FileBrowser::DIR_ONLY);
	}
	else if ( actionEvent.getId() == "mOptions" )
		mGuiMain->showLauncherOptions();
}
