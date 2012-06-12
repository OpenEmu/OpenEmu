/******************************************************************************
*
* FILENAME: katmenu.cpp
*
* DESCRIPTION:  This class will create a menu bar for the gui
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/29/06  bberlin      Creation
******************************************************************************/
#include <sstream>
#include "katmenu.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/video.h"
	#include "../interface/states.h"
	#include "../interface/ui.h"

	extern t_atari_video g_video;
	extern t_atari_sound g_sound;
}

KatMenu::KatMenu ( GuiMain *gMain ) : GuiObject(gMain)
{
	char tmp_string[10];
	std::string num_string;
	t_config *p_config = config_get_ptr();
	int i;

	/*
	 * File Menu
	 */
	addMenuItem ( "", "mFile", "File", 0 );
	addMenuItem ( "mFile", "mLoad", "Load Rom", this );
	addMenuItem ( "mFile", "mSaveState", "Save State", this );
	addMenuItem ( "mFile", "mLoadState", "Load State", this );
	addMenuItem ( "mFile", "mQuickSaveState", "Quick Save State", 0 );
	addMenuItem ( "mFile", "mQuickLoadState", "Quick Load State", 0 );
	for ( i = 1; i < 10; ++i )
	{
		sprintf ( tmp_string, "%d", i );
		num_string = tmp_string;
		addMenuItem ( "mQuickSaveState", "mQuickSaveState" + num_string, 
		                "Quick Save State " + num_string, this );
		addMenuItem ( "mQuickLoadState", "mQuickLoadState" + num_string, 
		                "Quick Load State " + num_string, this );
	}
	addMenuItem ( "mFile", "mLauncher", "Launcher", this );
	addMenuItem ( "mFile", "mBack", "Back To Game", this );
	addMenuItem ( "mFile", "mExit", "Exit", this );

	/*
	 * Machine Menu
	 */
	addMenuItem ( "", "mMachine", "Machine", 0 );
	addMenuItem ( "mMachine", "mSystem", "System", 0 );
	addMenuItem ( "mSystem", "mAtari5200", "Atari 5200", this );
	addMenuItem ( "mSystem", "mAtari800", "Atari 800", this );
	addMenuItem ( "mSystem", "mAtariXL", "Atari XL/XE", this );
	addMenuItem ( "mMachine", "mRam", "RAM", 0 );
	addMenuItem ( "mRam", "mRam16", "16 KB", this );
	addMenuItem ( "mRam", "mRam48", "48 KB", this );
	addMenuItem ( "mRam", "mRam52", "52 KB", this );
	addMenuItem ( "mRam", "mRam64", "64 KB", this );
	addMenuItem ( "mRam", "mRam128", "128 KB", this );
	addMenuItem ( "mMachine", "mVideo", "Video", 0 );
	addMenuItem ( "mVideo", "mNTSC", "NTSC", this );
	addMenuItem ( "mVideo", "mPAL", "PAL", this );
	addMenuItem ( "mMachine", "mThrottle", "Speed Throttle", this );
	addMenuItem ( "mMachine", "mBios", "BIOS Files", this );
	addMenuItem ( "mMachine", "mMedia", "Media", this );
	addMenuItem ( "mMachine", "mSioPatch", "SIO Patch", this );
	addMenuItem ( "mMachine", "mBasicEnabled", "BASIC Enabled", this );
	addMenuItem ( "mMachine", "mReset", "Reset", 0 );
	addMenuItem ( "mReset", "mResetWarm", "Warm", this );
	addMenuItem ( "mReset", "mResetCold", "Cold", this );

	/*
	 * View Menu
	 */
	addMenuItem ( "", "mView", "View", 0 );
	addMenuItem ( "mView", "mScreenSize", "Screen Size", 0 );
	addMenuItem ( "mScreenSize", "m1x", "1x", this );
	addMenuItem ( "mScreenSize", "m2x", "2x", this );
	addMenuItem ( "mScreenSize", "m3x", "3x", this );
	addMenuItem ( "mView", "mFullscreen", "Fullscreen", this );

	/*
	 * Options Menu
	 */
	addMenuItem ( "", "mOptions", "Options", 0 );
	addMenuItem ( "mOptions", "mVideo", "Video", this );
	addMenuItem ( "mOptions", "mSound", "Sound", this );
	addMenuItem ( "mOptions", "mInput", "Input", this );
	addMenuItem ( "mOptions", "mUI", "User Interface", this );

	/*
	 * Misc Menu
	 */
	addMenuItem ( "", "mMisc", "Misc", 0 );
	addMenuItem ( "mMisc", "mPalette", "Palette", 0 );
	addMenuItem ( "mPalette", "mImport", "Import", this );
	addMenuItem ( "mPalette", "mExport", "Export", this );
	addMenuItem ( "mPalette", "mPalEdit", "Edit", this );
	addMenuItem ( "mMisc", "mDebug", "Debug", this );
	addMenuItem ( "mMisc", "mMediaCreate", "Create Media", this );
	addMenuItem ( "mMisc", "mWizard", "Wizard", this );
	addMenuItem ( "mMisc", "mViewLog", "View Log", this );

	/*
	 * Setup choices
	 */
	setMachine ( p_config->machine_type );

	if ( p_config->system_type == NTSC )
		setItemChecked ( "mNTSC", true );
	else
		setItemChecked ( "mPAL", true );

	switch ( p_config->ram_size[p_config->machine_type] )
	{
		case 16: setItemChecked ( "mRam16", true ); break;
		case 48: setItemChecked ( "mRam48", true ); break;
		case 52: setItemChecked ( "mRam52", true ); break;
		case 64: setItemChecked ( "mRam64", true ); break;
		case 128: setItemChecked ( "mRam128", true ); break;
	}

	if ( p_config->sio_patch )
		setItemChecked ( "mSioPatch", true );

	if ( p_config->basic_enable )
		setItemChecked ( "mBasicEnabled", true );

	switch ( g_video.zoom ) {
		case 0: setItemChecked ( "m1x", true ); break;
		case 1: setItemChecked ( "m2x", true ); break;
		case 2: setItemChecked ( "m3x", true ); break;
		default: setItemChecked ( "m1x", true ); break;
	}

	if ( g_video.fullscreen )
		setItemChecked ( "mFullscreen", true );
}

KatMenu::~KatMenu ()
{

}

void KatMenu::setMachine(e_machine_type machine)
{
	t_config *p_config = config_get_ptr();

	// If switching between 800 and 5200 then change input so when input config
	//   is loaded, it's not all messed up
	if ( machine == MACHINE_TYPE_5200 && p_config->machine_type != MACHINE_TYPE_5200 )
		input_set_profile ( p_config->default_input_profile );

	if ( machine != MACHINE_TYPE_5200 && p_config->machine_type == MACHINE_TYPE_5200 )
		input_set_profile ( p_config->default_800input_profile );

	p_config->machine_type = machine;

	if ( p_config->machine_type == MACHINE_TYPE_5200 ) 
	{
		setItemChecked ( "mAtari5200", true );
		setItemChecked ( "mAtari800", false );
		setItemChecked ( "mAtariXL", false );

		setItemDimmed ( "mRam16", false );
		setItemDimmed ( "mRam48", true );
		setItemDimmed ( "mRam52", true );
		setItemDimmed ( "mRam64", true );
		setItemDimmed ( "mRam128", true );
	}
	else if ( p_config->machine_type == MACHINE_TYPE_800 ) 
	{
		setItemChecked ( "mAtari5200", false );
		setItemChecked ( "mAtari800", true );
		setItemChecked ( "mAtariXL", false );

		setItemDimmed ( "mRam16", false );
		setItemDimmed ( "mRam48", false );
		setItemDimmed ( "mRam52", false );
		setItemDimmed ( "mRam64", true );
		setItemDimmed ( "mRam128", true );
	}
	else 
	{
		setItemChecked ( "mAtari5200", false );
		setItemChecked ( "mAtari800", false );
		setItemChecked ( "mAtariXL", true );

		setItemDimmed ( "mRam16", false );
		setItemDimmed ( "mRam48", true );
		setItemDimmed ( "mRam52", true );
		setItemDimmed ( "mRam64", false );
		setItemDimmed ( "mRam128", false );
	}
}

void KatMenu::setRam(e_machine_type machine, int ram)
{
	t_config *p_config = config_get_ptr();

	p_config->ram_size[machine] = ram;

	setItemChecked ( "mRam16", false );
	setItemChecked ( "mRam48", false );
	setItemChecked ( "mRam52", false );
	setItemChecked ( "mRam64", false );
	setItemChecked ( "mRam128", false );

	switch ( ram )
	{
		case 16: setItemChecked ( "mRam16", true ); break;
		case 48: setItemChecked ( "mRam48", true ); break;
		case 52: setItemChecked ( "mRam52", true ); break;
		case 64: setItemChecked ( "mRam64", true ); break;
		case 128: setItemChecked ( "mRam128", true ); break;
		default: setItemChecked ( "mRam16", true ); break;
	}

	config_save();
}

void KatMenu::checkSaveStateExistance()
{
	int exists[9];
	char tmp_string[3];
	std::string num_string;
	int i;

	states_get_qs_existance ( exists );

	for ( i = 0; i < 9; ++i )
	{
		sprintf ( tmp_string, "%d", i+1 );
		num_string = tmp_string;
		setItemDimmed ( "mQuickLoadState" + num_string, exists[i] ? false : true );
	}
}

void KatMenu::action(const gcn::ActionEvent &actionEvent)
{
	std::string tmp_string;
	t_config *p_config = config_get_ptr();

	mMenuListener.setGuiMain ( mGuiMain );

	if ( actionEvent.getId() == "mLoad" ) 
	{
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(&mMenuListener);
		mGuiMain->getFileBrowser()->setActionEventId("LoadBrowser");
		mGuiMain->getFileBrowser()->show("Load ROM", p_config->rom_file, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "mSaveState" ) 
	{
		mGuiMain->getProfilePopup()->deleteActions();
		mGuiMain->getProfilePopup()->addActionListener(&mMenuListener);
		mGuiMain->getProfilePopup()->setActionEventId("SaveState");
		mGuiMain->getProfilePopup()->show( "Choose State or Create New", "SELECT Name FROM States", "", true );
	}
	else if ( actionEvent.getId() == "mLoadState" ) 
	{
		mGuiMain->getProfilePopup()->deleteActions();
		mGuiMain->getProfilePopup()->addActionListener(&mMenuListener);
		mGuiMain->getProfilePopup()->setActionEventId("LoadState");
		mGuiMain->getProfilePopup()->show( "Load State", "SELECT Name FROM States", "", false );
	}
	else if ( actionEvent.getId().find( "mQuickSaveState", 0 ) != std::string::npos ) 
	{
		save_quick_state ( atoi(actionEvent.getId().c_str()+15), console_get_ptr() );
		checkSaveStateExistance();
	}
	else if ( actionEvent.getId().find( "mQuickLoadState", 0 ) != std::string::npos ) 
	{
		if ( recall_quick_state ( atoi(actionEvent.getId().c_str()+15), console_get_ptr() ) )
		{
			mGuiMain->getInfoPopup()->deleteActions();
			mGuiMain->getInfoPopup()->show( "Error Loading State" );
			return;
		}
		mGuiMain->launch( 1, 0 );
	}
	else if ( actionEvent.getId() == "mLauncher" ) 
		mGuiMain->showLauncher();

	else if ( actionEvent.getId() == "mBack" ) 
		mGuiMain->launch( 0, 0 );

	else if ( actionEvent.getId() == "mExit" ) 
		mGuiMain->exit();

	else if ( actionEvent.getId().find( "mAtari", 0 ) != std::string::npos ) 
	{
		if ( actionEvent.getId().find( "5200", 0 ) != std::string::npos ) 
			setMachine ( MACHINE_TYPE_5200 );
		else if ( actionEvent.getId().find( "800", 0 ) != std::string::npos ) 
			setMachine ( MACHINE_TYPE_800 );
		else
			setMachine ( MACHINE_TYPE_XL );
		setRam ( p_config->machine_type, p_config->ram_size[p_config->machine_type] );
		config_save();
	}
	else if ( actionEvent.getId().find( "mRam", 0 ) != std::string::npos ) 
		setRam ( p_config->machine_type, atoi(actionEvent.getId().c_str()+4) );

	else if ( actionEvent.getId() == "mNTSC" ) 
	{
		setItemChecked ( "mNTSC", true );
		setItemChecked ( "mPAL", false );
		p_config->system_type = NTSC;
		config_save();
	}
	else if ( actionEvent.getId() == "mPAL" ) 
	{
		setItemChecked ( "mNTSC", false );
		setItemChecked ( "mPAL", true );
		p_config->system_type = PAL;
		config_save();
	}
	else if ( actionEvent.getId() == "mThrottle" ) 
		mGuiMain->showThrottle();

	else if ( actionEvent.getId() == "mBios" ) 
		mGuiMain->showBiosCfg();

	else if ( actionEvent.getId() == "mMedia" ) 
		mGuiMain->showMediaCfg();

	else if ( actionEvent.getId() == "mSioPatch" ) 
	{
		if ( p_config->sio_patch )
		{
			setItemChecked ( "mSioPatch", false );
			p_config->sio_patch = 0;
		}
		else
		{
			setItemChecked ( "mSioPatch", true );
			p_config->sio_patch = 1;
		}
		config_save();
	}
	else if ( actionEvent.getId() == "mBasicEnabled" ) 
	{
		if ( p_config->basic_enable )
		{
			setItemChecked ( "mBasicEnabled", false );
			p_config->basic_enable = 0;
		}
		else
		{
			setItemChecked ( "mBasicEnabled", true );
			p_config->basic_enable = 1;
		}
		config_save();
	}
	else if ( actionEvent.getId() == "mResetWarm" ) 
	{
		/* FIXME: What if no ROM loaded?  It will crash! */
		reset_6502();
	}
	else if ( actionEvent.getId() == "mResetCold" ) {
		console_reset ( p_config->machine_type, p_config->system_type, p_config->ram_size[p_config->machine_type], g_sound.freq  );
	}
	else if ( actionEvent.getId() == "m1x" ) 
	{
		setItemChecked ( "m1x", true );
		setItemChecked ( "m2x", false );
		setItemChecked ( "m3x", false );
		g_video.zoom = 0;
		video_save_profile ( g_video.name, &g_video );
	}
	else if ( actionEvent.getId() == "m2x" ) 
	{
		setItemChecked ( "m1x", false );
		setItemChecked ( "m2x", true );
		setItemChecked ( "m3x", false );
		g_video.zoom = 1;
		video_save_profile ( g_video.name, &g_video );
	}
	else if ( actionEvent.getId() == "m3x" ) 
	{
		setItemChecked ( "m1x", false );
		setItemChecked ( "m2x", false );
		setItemChecked ( "m3x", true );
		g_video.zoom = 2;
		video_save_profile ( g_video.name, &g_video );
	}
	else if ( actionEvent.getId() == "mFullscreen" ) 
	{
		if ( g_video.fullscreen )
		{
			g_video.fullscreen = 0;
			setItemChecked ( "mFullscreen", false );
		}
		else
		{
			g_video.fullscreen = 1;
			setItemChecked ( "mFullscreen", true );
		}
		video_save_profile ( g_video.name, &g_video );
	}
	else if ( actionEvent.getId() == "mVideo" ) 
		mGuiMain->showVideoCfg();

	else if ( actionEvent.getId() == "mSound" ) 
		mGuiMain->showSoundCfg();

	else if ( actionEvent.getId() == "mInput" ) 
		mGuiMain->showInputCfg();

	else if ( actionEvent.getId() == "mPalEdit" ) 
		mGuiMain->showPaletteCfg();

	else if ( actionEvent.getId() == "mUI" ) 
		mGuiMain->showUserInterfaceCfg();

	else if ( actionEvent.getId() == "mImport" ) 
		mGuiMain->showConvert ( Convert::CONVERT_A800 );

	else if ( actionEvent.getId() == "mExport" ) 
		mGuiMain->showConvert ( Convert::CONVERT_KAT5200 );

	else if ( actionEvent.getId() == "mWizard" ) 
	{
		tmp_string = " ";
		mGuiMain->showWizard( tmp_string );
	}

	else if ( actionEvent.getId() == "mMediaCreate" ) 
		mGuiMain->showMediaCreate();

	else if ( actionEvent.getId() == "mViewLog" ) 
		mGuiMain->showLog();
}
