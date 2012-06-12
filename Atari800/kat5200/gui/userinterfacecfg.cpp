/******************************************************************************
*
* FILENAME: userinterfacecfg.cpp
*
* DESCRIPTION:  This class will show the UI configuration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/19/06  bberlin      Creation
******************************************************************************/
#include "userinterfacecfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/sdl_if.h"
	#include "../interface/db_if.h"

	extern t_ui g_ui;
}

UserInterfaceCfg::UserInterfaceCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	int i;
	char tmp_string[20];

	t_config *p_config = config_get_ptr();

	setVisible ( false );
	setCaption ("User Interface Settings");

	/*
	 * Setup all of our widgets
	 */
	mProfile.setListModel ( &mProfileList );
	mDefault.setCaption ("Use as Default");
	mRename.setCaption ("Rename");
	mSaveAs.setCaption ("SaveAs");
	mDescriptionLabel.setCaption ("Description");
	mProfileDec.setCaption ( "Profile" );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	mFontDec.setCaption ( "Font" );
	mFontBrowse.setCaption ("Browse" );

	mStartDec.setCaption ( "Start Options" );
	mStartGui.setCaption ( "Start Program with GUI" );
	mStartLauncher.setCaption ( "Start GUI with Launcher" );

	mPCDec.setCaption ( "PC Input" );
	mDeviceLabel.setCaption ( "Device" );
	mDevice.setListModel (&mDeviceList);
	mDeviceNumLabel.setCaption ( "Device Num" );
	mPartTypeLabel.setCaption ( "Part Type" );
	mPartType.setListModel (&mPartTypeList);
	mPartNumLabel.setCaption ( "Part Num" );
	mDirectionLabel.setCaption ( "Direction" );
	mDirection.setListModel (&mDirectionList);
	mAutoDetect.setCaption ("Auto-Detect");

	mUIDec.setCaption ( "" );

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mProfile.addActionListener ( this );
	mDescriptionBox.addMouseListener ( this );
	mRename.addActionListener ( this );
	mSaveAs.addActionListener ( this );
	mDefault.addActionListener ( this );

	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );

	mFontFile.addMouseListener ( this );
	mFontBrowse.addActionListener ( this );
	mFontBrowse.setActionEventId ( "mFontBrowse" );

	mProfile.setActionEventId ( "mProfile" );
	mRename.setActionEventId ( "mRename" );
	mSaveAs.setActionEventId ( "mSaveAs" );
	mDefault.setActionEventId ( "mDirty" );

	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );

	mStartGui.addActionListener ( this );
	mStartLauncher.addActionListener ( this );
	mStartGui.setActionEventId ( "mDirty" );
	mStartLauncher.setActionEventId ( "mDirty" );

	mDevice.addActionListener ( this );
	mDirection.addActionListener ( this );
	mPartType.addActionListener ( this );
	mAutoDetect.addActionListener ( this );

	mDevice.setActionEventId ( "mDirty" );
	mDirection.setActionEventId ( "mDirty" );
	mPartType.setActionEventId ( "mDirty" );
	mAutoDetect.setActionEventId ( "mAutoDetect" );

	mDeviceNum.addMouseListener ( this );
	mPartNum.addMouseListener ( this );

	/*
	 * Size and place our widgets
	 */
	add ( &mProfileDec );
	add ( &mProfile );
	add ( &mDefault );
	add ( &mRename );
	add ( &mSaveAs );
	add ( &mDescriptionBox );
	add ( &mDescriptionLabel );

	add ( &mOk );
	add ( &mCancel );

	add ( &mFontDec );
	add ( &mFontFile );
	add ( &mFontBrowse );

	add ( &mStartDec );
	add ( &mStartGui );
	add ( &mStartLauncher );

	add ( &mPCDec );
	add ( &mDeviceLabel );
	add ( &mDevice );
	add ( &mDeviceNumLabel );
	add ( &mDeviceNum );
	add ( &mPartTypeLabel );
	add ( &mPartType );
	add ( &mPartNumLabel );
	add ( &mPartNum );
	add ( &mDirectionLabel );
	add ( &mDirection );
	add ( &mAutoDetect );

	add ( &mUIDec );

	add ( &mStateDec );
	add ( &mStateDec2 );

	setDimension ( mGuiMain->getRectangle(2,2,76,26) );

	mProfile.setDimension ( mGuiMain->getRectangle(1,1.2,54,2) );
	mDescriptionLabel.setDimension ( mGuiMain->getRectangle(1,2.5,54,2) );
	mDescriptionBox.setDimension ( mGuiMain->getRectangle(1,3.5,54,2) );
	mDefault.setDimension ( mGuiMain->getRectangle(57,3.5,14,1) );
	mRename.setDimension ( mGuiMain->getRectangle(57,1.2,3,1) );
	mSaveAs.setDimension ( mGuiMain->getRectangle(65.5,1.2,3,1) );
	mProfileDec.setDimension ( mGuiMain->getRectangle(0.5,0,72.5,5) );

	mOk.setDimension ( mGuiMain->getRectangle(28,22.8,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(38,22.8,6,1) );

	mFontDec.setDimension ( mGuiMain->getRectangle(0.5,5.0,74,3) );
	mFontFile.setDimension ( mGuiMain->getRectangle(1,6.2,61,1) );
	mFontBrowse.setDimension ( mGuiMain->getRectangle(64,6,6,1) );

	mStartDec.setDimension ( mGuiMain->getRectangle(0.5,8.0,74,2.5) );
	mStartGui.setDimension ( mGuiMain->getRectangle(11,9,6,1) );
	mStartLauncher.setDimension ( mGuiMain->getRectangle(40,9,6,1) );

	mPCDec.setDimension ( mGuiMain->getRectangle(46.5,16.5,28.5,8.3) );
	mAutoDetect.setDimension ( mGuiMain->getRectangle(51,17.5,6,1) );
	mDeviceLabel.setDimension ( mGuiMain->getRectangle(47,19.5,6,1) );
	mDevice.setDimension ( mGuiMain->getRectangle(59,19.5,15,1) );
	mDeviceNumLabel.setDimension ( mGuiMain->getRectangle(47,20.5,6,1) );
	mDeviceNum.setDimension ( mGuiMain->getRectangle(59,20.5,15,1) );
	mPartTypeLabel.setDimension ( mGuiMain->getRectangle(47,21.5,6,1) );
	mPartType.setDimension ( mGuiMain->getRectangle(59,21.5,15,1) );
	mPartNumLabel.setDimension ( mGuiMain->getRectangle(47,22.5,6,1) );
	mPartNum.setDimension ( mGuiMain->getRectangle(59,22.5,15,1) );
	mDirectionLabel.setDimension ( mGuiMain->getRectangle(47,23.5,6,1) );
	mDirection.setDimension ( mGuiMain->getRectangle(59,23.5,15,1) );

	mUIDec.setDimension ( mGuiMain->getRectangle(0.5,10.5,18,11.5) );
	mExit.setDimension ( mGuiMain->getRectangle(1,11.5,6,1) );
	mBack.setDimension ( mGuiMain->getRectangle(1,13.2,6,1) );
	mFullScreen.setDimension ( mGuiMain->getRectangle(1,14.9,6,1) );
	mScreenShot.setDimension ( mGuiMain->getRectangle(1,16.6,6,1) );

	mStateDec.setDimension ( mGuiMain->getRectangle(19.5,10.5,26.2,11.5) );
	for ( i = 0; i < 6; ++i )
	{
		mLoadState[i].setDimension ( mGuiMain->getRectangle(20,11.5+(i)*1.7,6,1) );
		mSaveState[i].setDimension ( mGuiMain->getRectangle(33,11.5+(i)*1.7,6,1) );
	}
	mStateDec2.setDimension ( mGuiMain->getRectangle(46.5,10.5,28.5,6.0) );
	for ( i = 6; i < 9; ++i )
	{
		mLoadState[i].setDimension ( mGuiMain->getRectangle(47,11.5+(i-6)*1.7,6,1) );
		mSaveState[i].setDimension ( mGuiMain->getRectangle(60,11.5+(i-6)*1.7,6,1) );
	}

	setupInputButton ( &mExit, "Exit", "UI", &mUI.keys[UI_KEY_EXIT] );
	setupInputButton ( &mBack, "Back", "UI", &mUI.keys[UI_KEY_BACK] );
	setupInputButton ( &mFullScreen, "Toggle Fullscreen", "UI", &mUI.keys[UI_KEY_FULLSCREEN] );
	setupInputButton ( &mScreenShot, "ScreenShot", "UI", &mUI.keys[UI_KEY_SCREENSHOT] );

	mStateDec.setCaption ( "" );
	mStateDec2.setCaption ( "" );
	for ( i = 1; i < 10; ++i )
	{
		sprintf ( tmp_string, "Load State %d", i );
		setupInputButton ( &mLoadState[i-1], tmp_string, "UI", 
		                        &mUI.keys[UI_KEY_LOAD_STATE_1+(i-1)] );
		sprintf ( tmp_string, "Save State %d", i );
		setupInputButton ( &mSaveState[i-1], tmp_string, "UI", 
		                        &mUI.keys[UI_KEY_SAVE_STATE_1+(i-1)] );
	}
	mProfile.adjustHeight();
	mDescriptionLabel.adjustSize();
	mDescriptionBox.adjustHeight();
	mRename.adjustSize();
	mSaveAs.adjustSize();
	mDefault.adjustSize();

	mOk.adjustSize();
	mCancel.adjustSize();

	mFontFile.adjustHeight();
	mFontBrowse.adjustSize();

	mStartGui.adjustSize();
	mStartLauncher.adjustSize();

	mDeviceLabel.adjustSize();
	mDevice.adjustHeight();
	mDeviceNumLabel.adjustSize();
	mDeviceNum.adjustHeight();
	mPartTypeLabel.adjustSize();
	mPartType.adjustHeight();
	mPartNumLabel.adjustSize();
	mPartNum.adjustHeight();
	mDirectionLabel.adjustSize();
	mDirection.adjustHeight();
	mAutoDetect.adjustSize();

	initGroupControls();
	initProfileList();

	mExit.setSelected ( true );
}

UserInterfaceCfg::~UserInterfaceCfg ()
{

}

void UserInterfaceCfg::setupInputButton (GuiInputButton *btn, std::string caption, 
		                                  std::string grp, t_atari_common *ctrl )
{
		btn->setCaption ( caption );
		btn->setGroup ( grp );
		btn->addActionListener ( this );
		btn->setActionEventId ( "mInputButton" );
		add ( btn );
		btn->adjustSize ();
		btn->mCtrl = ctrl;
}

void UserInterfaceCfg::show ( )
{
	mConfig = *(config_get_ptr());
	mUI = g_ui;

	std::string profile_name;

	mSettingsDirty = false;

	profile_name = mUI.name;

	setInput ( mExit.mCtrl );

	mFontFile.setText ( mConfig.font_file );
	mStartGui.setSelected ( mConfig.start_with_gui ? true : false );
	mStartLauncher.setSelected ( mConfig.start_gui_with_launcher ? true : false );

	mSettingsDirty = false;
	mAutoDetectAct = -1;

	if ( getParent () )
		getParent()->moveToTop (this);

	changeGroup ( profile_name );
	setVisible ( true );
}

void UserInterfaceCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void UserInterfaceCfg::initProfileList ( )
{
	struct generic_node *nodeptr = 0;

	mProfileList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM UIKeys", &nodeptr );
	mProfileList.setList ( nodeptr );
}

void UserInterfaceCfg::initGroupControls ( )
{
	mDeviceList.deleteAll();
	mDeviceList.addElement ( "Keyboard", DEV_KEYBOARD, 0 );
	mDeviceList.addElement ( "Joystick", DEV_JOYSTICK, 0 );
	mDeviceList.addElement ( "Mouse", DEV_MOUSE, 0 );

	mPartTypeList.deleteAll();
	mPartTypeList.addElement ( "D-Pad/Hat", PART_TYPE_HAT, 0 );
	mPartTypeList.addElement ( "Axis", PART_TYPE_AXIS, 0 );
	mPartTypeList.addElement ( "TrackBall", PART_TYPE_BALL, 0 );
	mPartTypeList.addElement ( "Button", PART_TYPE_BUTTON, 0 );
	mPartTypeList.addElement ( "Key", PART_TYPE_KEY, 0 );

	mDirectionList.deleteAll();
	mDirectionList.addElement ( "Left", DIR_LEFT, 0 );
	mDirectionList.addElement ( "Right", DIR_RIGHT, 0 );
	mDirectionList.addElement ( "Up", DIR_UP, 0 );
	mDirectionList.addElement ( "Down", DIR_DOWN, 0 );
	mDirectionList.addElement ( "+", DIR_PLUS, 0 );
	mDirectionList.addElement ( "-", DIR_MINUS, 0 );
	mDirectionList.addElement ( "Press", DIR_PRESS, 0 );
	mDirectionList.addElement ( "Release", DIR_RELEASE, 0 );
}

void UserInterfaceCfg::saveSettings ( std::string profile_name )
{
	t_config *p_config = config_get_ptr();

	// Figure out which inputs are selected and update those
	getInput ( mExit.getMarkedButton()->mCtrl );

	strcpy ( mUI.description, mDescriptionBox.getText().c_str() );

	strcpy ( mConfig.font_file, mFontFile.getText().c_str() );
	*(config_get_ptr()) = mConfig;

	if ( !ui_save_profile ( (char *)profile_name.c_str(), &mUI ) ) {

		strcpy ( mUI.name, profile_name.c_str() );

		initProfileList();
		mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

		if ( mDefault.isSelected() == true ) {
			strcpy ( p_config->default_ui_profile, mUI.name );
			ui_load_profile ( profile_name.c_str(), &g_ui );
		}
	}

	p_config->start_with_gui = mStartGui.isSelected() ? 1 : 0;
	p_config->start_gui_with_launcher = mStartLauncher.isSelected() ? 1 : 0;

	config_save ( );
}

void UserInterfaceCfg::changeGroup ( std::string profile_name )
{
	t_config *p_config = config_get_ptr();
	t_ui *p_ui = &mUI;

	/*
	 * Save current settings, if necessary
	 */
	if ( mSettingsDirty == true ) {
		saveSettings( mProfileList.getLabelFromIndex(mProfile.getSelected()) );
	}

	mSettingsDirty = false;

	/*
	 * Get values for this Group
	 */
	if ( ui_load_profile ( profile_name.c_str(), p_ui ) ) {
		mGuiMain->showError(ERR_PROFILE_LOAD);

		ui_load_profile ( p_config->default_ui_profile, &mUI );
		profile_name = p_config->default_ui_profile;
	}
	mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

	/*
	 * Set values on the GUI
	 */
	mDescriptionBox.setText ( p_ui->description );

	if ( !strcmp(profile_name.c_str(), p_config->default_ui_profile) )
		mDefault.setSelected ( true );
	else
		mDefault.setSelected ( false );

	setInput ( mExit.getMarkedButton()->mCtrl );
}

void UserInterfaceCfg::getInput( t_atari_common *ctrl )
{
	char tmp_string[256];

	ctrl->device = (e_dev_type) mDeviceList.getValueFromIndex( mDevice.getSelected() );

	if ( ctrl->device == DEV_KEYBOARD )
	{
		strcpy ( tmp_string, mDeviceNum.getText().c_str() );
		ctrl->device_num = pc_interpret_mod_key(tmp_string);
	}
	else
		ctrl->device_num = atoi(mDeviceNum.getText().c_str());

	ctrl->part_type = (e_part_type) mPartTypeList.getValueFromIndex( mPartType.getSelected() );
	if ( ctrl->device == DEV_KEYBOARD ) {
		strcpy ( tmp_string, mPartNum.getText().c_str() );
		ctrl->part_num = pc_interpret_key ( tmp_string );
	}
	else {
		ctrl->part_num = atoi ( mPartNum.getText().c_str() );
	}

	ctrl->direction = (e_direction) mDirectionList.getValueFromIndex( mDirection.getSelected() );
}

void UserInterfaceCfg::setInput( t_atari_common *ctrl )
{
	char tmp_string[10];

	mCurrentCtrl = ctrl;

	mDevice.setSelected ( mDeviceList.getIndexFromValue(ctrl->device) );

	if ( ctrl->device == DEV_KEYBOARD )
		pc_interpret_mod_key_value ( ctrl->device_num, tmp_string );
	else
		sprintf ( tmp_string, "%d", ctrl->device_num );
	mDeviceNum.setText( tmp_string );

	mPartType.setSelected( mPartTypeList.getIndexFromValue( ctrl->part_type ) );

	if ( ctrl->device == DEV_KEYBOARD )
		pc_interpret_key_value ( ctrl->part_num, tmp_string );
	else
		sprintf ( tmp_string, "%d", ctrl->part_num );
	mPartNum.setText( tmp_string );

	mDirection.setSelected ( mDirectionList.getIndexFromValue(ctrl->direction) );
}

void UserInterfaceCfg::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();
	std::string new_name;
	static int first_axis_event;
	char tmp_string[257];

	if ( actionEvent.getId() == "mOk" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mProfileList.getLabelFromIndex(mProfile.getSelected()) );
		}
		hide();
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}
	else if ( actionEvent.getId() == "mRename" ) {
		if ( !mProfileList.getNumberOfItems() ) 
		{
			mGuiMain->getInfoPopup()->show("No Existing Profiles.  No Action Taken");
			return;
		}
		mGuiMain->getMessagePopup()->deleteActions();
		mGuiMain->getMessagePopup()->addActionListener(this);
		mGuiMain->getMessagePopup()->setActionEventId("Rename");
		mGuiMain->getMessagePopup()->setInput(mProfileList.getLabelFromIndex(mProfile.getSelected()));
		mGuiMain->getMessagePopup()->show("Enter Profile Name", "Ok", true, 
		                                     "Cancel", true, "", false, true);
	}
	else if ( actionEvent.getId() == "Rename" ) {
		mGuiMain->getMessagePopup()->hide();
		if ( mGuiMain->getMessagePopup()->getButton() == 0 ) {
			new_name = mGuiMain->getMessagePopup()->getInput();
			if ( mProfileList.getIndexFromLabel(new_name.c_str()) < mProfileList.getNumberOfItems() ) {
				mGuiMain->getInfoPopup()->show("Profile Exists.  No Action Taken");
			}
			else {
				sprintf ( tmp_string, "UPDATE UIKeys SET Name='%s' WHERE Name='%s'", 
				          new_name.c_str(), mProfileList.getLabelFromIndex(mProfile.getSelected()) );
				db_if_exec_sql ( tmp_string, NULL, NULL );
				initProfileList();
				changeGroup (new_name);
			}
		}
	}
	else if ( actionEvent.getId() == "mSaveAs" ) {
		mGuiMain->getProfilePopup()->deleteActions();
		mGuiMain->getProfilePopup()->addActionListener(this);
		mGuiMain->getProfilePopup()->setActionEventId("SaveBrowser");
		mGuiMain->getProfilePopup()->show( "Choose Profile or Create New", "SELECT Name FROM UIKeys", 
		                     mProfileList.getLabelFromIndex(mProfile.getSelected()), true );
	}
	else if ( actionEvent.getId() == "SaveBrowser" ) {
		if ( mGuiMain->getProfilePopup()->getCancelPressed() == false ) {
			saveSettings ( mGuiMain->getProfilePopup()->getProfile() );
			mSettingsDirty = false;
		}
	}
	else if ( actionEvent.getId() == "mDirty" ) {
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mInputButton" ) {
		getInput ( mCurrentCtrl );
		setInput ( ((GuiInputButton *)(actionEvent.getSource()))->mCtrl );
	}
	else if ( actionEvent.getId() == "mFontBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FontFile" );
		mGuiMain->getFileBrowser()->show("Load Font File", p_config->font_file, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "FontFile" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mFontFile.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
			mSettingsDirty = true;
		}
	}
	else if ( actionEvent.getId() == "mAutoDetect" ) {
		if ( mAutoDetectAct == -1 ) 
		{
			pc_detect_and_open_joysticks ();

			first_axis_event = 1;
			mAutoListener.mCountModKey = false;
			mAutoListener.setWindow ( mGuiMain->getAutoWindow() );

			mGuiMain->getAutoWindow()->deleteAllListeners ( );
			mGuiMain->getAutoWindow()->addActionListener ( this );
			mGuiMain->getAutoWindow()->addKeyListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->addMouseListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->addJoystickListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->setActionEventId ( "mAutoWindow" );
			mGuiMain->getAutoWindow()->setDimension ( mGuiMain->getRectangle(11,16.5,60,7) );

			mGuiMain->showAutoWindow( "User Action Required", "Perform action to associate with controller part" );
		}
		else {
			mAutoDetectAct--;
		}
	}
	else if ( actionEvent.getId() == "mAutoWindow" ) {
		mAutoListener.getInput ( 0, &mCurrentCtrl->device, 
		                                  &mCurrentCtrl->device_num,
		                                  &mCurrentCtrl->part_type, 
		                                  &mCurrentCtrl->part_num,
		                                  &mCurrentCtrl->direction );
		if ( mCurrentCtrl->device != DEV_JOYSTICK || 
		     mCurrentCtrl->part_type != PART_TYPE_AXIS || !first_axis_event )
		{
			mSettingsDirty = true;
			mGuiMain->hideAutoWindow();
			setInput ( mCurrentCtrl );
			if ( mCurrentCtrl->device == DEV_MOUSE && 
			     mCurrentCtrl->part_type == PART_TYPE_BUTTON )
				mAutoDetectAct = 0;
		}
		first_axis_event = 0;
	}
}

void UserInterfaceCfg::mousePressed(gcn::MouseEvent &mouseEvent)
{
	bool comp_has_mouse = false;

	if ( &mDeviceNum == mouseEvent.getSource() || 
	     &mPartNum == mouseEvent.getSource() || 
		 &mFontFile == mouseEvent.getSource() )
		comp_has_mouse = true;

    if ((comp_has_mouse == true) && mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
		mSettingsDirty = true;
    }

	gcn::Window::mousePressed(mouseEvent);
}

