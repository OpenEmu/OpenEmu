/******************************************************************************
*
* FILENAME: inputcfg.cpp
*
* DESCRIPTION:  This class will show the input configuration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/28/06  bberlin      Creation
* 0.6.2   06/25/09  bberlin      In changeGroup, fixed issue where currently 
*                                   selected control was not mCurrentCtrl
******************************************************************************/
#include "inputcfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/sdl_if.h"
	#include "../interface/db_if.h"
	extern t_atari_input g_input;
}

InputCfg::InputCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	int i,j;
	char tmp_string[10];
	char player_string[10];
	double dim_x[]={11,6,11,16,6,11,16,6,11,16};
	double dim_y[]={17,11.6,11.6,11.6,13.4,13.4,13.4,15.2,15.2,15.2};
	int key[] = {KEY_0>>1,KEY_1>>1,KEY_2>>1,KEY_3>>1,KEY_4>>1,
	             KEY_5>>1,KEY_6>>1,KEY_7>>1,KEY_8>>1,KEY_9>>1};

	t_config *p_config = config_get_ptr();

	setVisible ( false );
	setCaption ("Input Settings");

	/*
	 * Setup all of our widgets
	 */
	mProfileDec.setCaption ( "Profile" );
	mRename.setCaption ("Rename");
	mSaveAs.setCaption ("Save As");

	mOk.setCaption("Ok");
	mCancel.setCaption("Cancel");

	mDefault.setCaption ("Use as Default");

	mProfile.setListModel ( &mProfileList );

	mMouseLabel.setCaption ( "Mouse Speed" );
	mMouse.setListModel ( &mMouseList );

	/*
	 * Setup Player Tab and 4 player windows
	 */
	for ( i = 0; i < 4; ++i ) {
		sprintf ( player_string, "Player %d", i+1 );
		mPlayerTab[i].setCaption( player_string );

		mSettingsDec[i].setCaption ( "PC Control Settings" );
		mSensitivityLabel[i].setCaption ( "Sensitivity" );
		mSensitivity[i].setScale(1,100);
		mSensitivity[i].setStepLength(1.0);
		mSensitivityValue[i].setCaption( "0 %" );
		mDeadzoneLabel[i].setCaption( "Deadzone" );
		mDeadzone[i].setScale(1,100);
		mDeadzone[i].setStepLength(1.0);
		mDeadzoneValue[i].setCaption ( "0 %" );
		mSimulateAnalog[i].setCaption( "Simulate Analog" );

		mTypeDec[i].setCaption( "A5200 Controller" );
		mPluggedIn[i].setListModel( &mPluggedInList[i] );

		mPCDec[i].setCaption( "PC Input" );
		mDeviceLabel[i].setCaption( "Device" );
		mDevice[i].setListModel( &mDeviceList[i] );
		mDeviceNumLabel[i].setCaption ( "Device Num" );
		mPartTypeLabel[i].setCaption ( "Part Type" );
		mPartType[i].setListModel( &mPartTypeList[i] );
		mPartNumLabel[i].setCaption ( "Part Num" );
		mDirectionLabel[i].setCaption ( "Direction" );
		mDirection[i].setListModel( &mDirectionList[i] );
		mAutoDetect[i].setCaption( "Auto-Detect" );

		mStickDec[i].setCaption( "Joystick" );
		mPaddlesDec[i].setCaption( "Paddles" );
		mKeypadDec[i].setCaption( "Keypad" );
		mA5200Dec[i].setCaption( "Controller" );

		mPaddle1Label[i].setCaption( "1" );
		mPaddle2Label[i].setCaption( "2" );
	}

	/*
	 * Create Keyboard window for Atari 800
	 */
	mKeyboardTab.setCaption( "Keyboard" );

	mPCDec[i].setCaption( "PC Input" );
	mDeviceLabel[i].setCaption( "Device" );
	mDevice[i].setListModel( &mDeviceList[i] );
	mDeviceNumLabel[i].setCaption( "Device Num" );
	mPartTypeLabel[i].setCaption( "Part Type" );
	mPartType[i].setListModel( &mPartTypeList[i] );
	mPartNumLabel[i].setCaption( "Part Num" );
	mDirectionLabel[i].setCaption( "Direction" );
	mDirection[i].setListModel( &mDirectionList[i] );
	mAutoDetect[i].setCaption( "Auto-Detect" );

	strcpy ( player_string, "Keyboard" );

	mKeysDec.setCaption( "A800 Keys" );
	mControlsDec.setCaption( "A800 Controls" );

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mProfile.addActionListener ( this );
	mRename.addActionListener ( this );
	mSaveAs.addActionListener ( this );
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mDefault.addActionListener ( this );

	mProfile.setActionEventId ( "mProfile" );
	mRename.setActionEventId ( "mRename" );
	mSaveAs.setActionEventId ( "mSaveAs" );
	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );
	mDefault.setActionEventId ( "mDirty" );

	mMouse.addActionListener ( this );
	mMouse.setActionEventId ( "mDirty" );

	for ( i = 0; i < 4; ++i ) {
		mDeadzone[i].addActionListener ( this );
		mSensitivity[i].addActionListener ( this );
		mSimulateAnalog[i].addActionListener ( this );
		mPluggedIn[i].addActionListener ( this );
		mDevice[i].addActionListener ( this );
		mDirection[i].addActionListener ( this );
		mPartType[i].addActionListener ( this );
		mAutoDetect[i].addActionListener ( this );

		mDeadzone[i].setActionEventId ( "mDeadzone" );
		mSensitivity[i].setActionEventId ( "mSensitivity" );
		mSimulateAnalog[i].setActionEventId ( "mDirty" );
		mPluggedIn[i].setActionEventId ( "mDirty" );
		mDevice[i].setActionEventId ( "mDirty" );
		mDirection[i].setActionEventId ( "mDirty" );
		mPartType[i].setActionEventId ( "mDirty" );
		mAutoDetect[i].setActionEventId ( "mAutoDetect" );

		mDeviceNum[i].addMouseListener ( this );
		mPartNum[i].addMouseListener ( this );
	}

	mAutoDetect[i].addActionListener ( this );
	mAutoDetect[i].setActionEventId ( "mAutoDetect" );

	mDeviceNum[i].addMouseListener ( this );
	mPartNum[i].addMouseListener ( this );

	/*
	 * Size and place our widgets
	 */
	add ( &mProfileDec );
	add ( &mProfile);
	add ( &mDescriptionBox );
	add ( &mRename );
	add ( &mSaveAs );

	add ( &mOk );
	add ( &mCancel );

	add ( &mDefault );

	add ( &mMouse );
	add ( &mMouseLabel );

	add ( &mPlayerWindow );

	for ( i = 0; i < 4; ++i ) {

		mPlayerWindow.addTab ( 0, &mPlayerTab[i] );

		mPlayerTab[i].add ( &mSettingsDec[i] );
		mPlayerTab[i].add ( &mSensitivityLabel[i] );
		mPlayerTab[i].add ( &mSensitivity[i] );
		mPlayerTab[i].add ( &mSensitivityValue[i] );
		mPlayerTab[i].add ( &mDeadzoneLabel[i] );
		mPlayerTab[i].add ( &mDeadzone[i] );
		mPlayerTab[i].add ( &mDeadzoneValue[i] );
		mPlayerTab[i].add ( &mSimulateAnalog[i] );

		mPlayerTab[i].add ( &mTypeDec[i] );
		mPlayerTab[i].add ( &mPluggedIn[i] );

		mPlayerTab[i].add ( &mPCDec[i] );
		mPlayerTab[i].add ( &mDeviceLabel[i] );
		mPlayerTab[i].add ( &mDevice[i] );
		mPlayerTab[i].add ( &mDeviceNumLabel[i] );
		mPlayerTab[i].add ( &mDeviceNum[i] );
		mPlayerTab[i].add ( &mPartTypeLabel[i] );
		mPlayerTab[i].add ( &mPartType[i] );
		mPlayerTab[i].add ( &mPartNumLabel[i] );
		mPlayerTab[i].add ( &mPartNum[i] );
		mPlayerTab[i].add ( &mDirectionLabel[i] );
		mPlayerTab[i].add ( &mDirection[i] );
		mPlayerTab[i].add ( &mAutoDetect[i] );

		mPlayerTab[i].add ( &mStickDec[i] );
		mPlayerTab[i].add ( &mPaddlesDec[i] );
		mPlayerTab[i].add ( &mKeypadDec[i] );
		mPlayerTab[i].add ( &mA5200Dec[i] );

		mPlayerTab[i].add ( &mPaddle1Label[i] );
		mPlayerTab[i].add ( &mPaddle2Label[i] );
	}

	mPlayerWindow.addTab ( 0, &mKeyboardTab );
	mKeyboardShowing = true;
	mKeyboardTab.add ( &mPCDec[i] );
	mKeyboardTab.add ( &mDeviceLabel[i] );
	mKeyboardTab.add ( &mDevice[i] );
	mKeyboardTab.add ( &mDeviceNumLabel[i] );
	mKeyboardTab.add ( &mDeviceNum[i] );
	mKeyboardTab.add ( &mPartTypeLabel[i] );
	mKeyboardTab.add ( &mPartType[i] );
	mKeyboardTab.add ( &mPartNumLabel[i] );
	mKeyboardTab.add ( &mPartNum[i] );
	mKeyboardTab.add ( &mDirectionLabel[i] );
	mKeyboardTab.add ( &mDirection[i] );
	mKeyboardTab.add ( &mAutoDetect[i] );

	mKeyboardTab.add ( &mKeysDec );
	mKeyboardTab.add ( &mControlsDec );

	setDimension ( mGuiMain->getRectangle(2,2,76,26) );

	mProfileDec.setDimension ( mGuiMain->getRectangle(0.5,0.0,74.5,4.2) );
	mProfile.setDimension ( mGuiMain->getRectangle(1,1.0,56,2) );
	mDescriptionBox.setDimension ( mGuiMain->getRectangle(1,2.7,56,2) );
	mDefault.setDimension ( mGuiMain->getRectangle(58.3,2.7,14,1) );
	mRename.setDimension ( mGuiMain->getRectangle(58.3,1.0,3,1) );
	mSaveAs.setDimension ( mGuiMain->getRectangle(66.8,1.0,3,1) );
	mOk.setDimension ( mGuiMain->getRectangle(60,23,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(65,23,6,1) );

	mMouseLabel.setDimension ( mGuiMain->getRectangle(57,17,6,1) );
	mMouse.setDimension ( mGuiMain->getRectangle(69,17,5,1) );

	mPlayerWindow.setDimension ( mGuiMain->getRectangle(1,4.5,54,20.0) );
	for ( i = 0; i < 4; ++i ) {

		mPlayerTab[i].setDimension ( mGuiMain->getRectangle(-1,-1,55,21) );

		mSettingsDec[i].setDimension ( mGuiMain->getRectangle(24.5,12.5,29.5,6.0) );
		mSensitivityLabel[i].setDimension ( mGuiMain->getRectangle(25,14,11,1) );
		mSensitivity[i].setDimension ( mGuiMain->getRectangle(37,14,11,1) );
		mSensitivityValue[i].setDimension ( mGuiMain->getRectangle(49,14,4,1) );
		mDeadzoneLabel[i].setDimension ( mGuiMain->getRectangle(25,15.5,6,1) );
		mDeadzone[i].setDimension ( mGuiMain->getRectangle(37,15.5,11,1) );
		mDeadzoneValue[i].setDimension ( mGuiMain->getRectangle(49,15.5,6,1) );
		mSimulateAnalog[i].setDimension ( mGuiMain->getRectangle(25,17,6,1) );

		mTypeDec[i].setDimension ( mGuiMain->getRectangle(24.5,9.5,29.5,2.5) );
		mPluggedIn[i].setDimension ( mGuiMain->getRectangle(31,10.5,17,1) );

		mPCDec[i].setDimension ( mGuiMain->getRectangle(24.5,0.5,29.5,8.5) );
		mDeviceLabel[i].setDimension ( mGuiMain->getRectangle(26,1.5,6,1) );
		mDevice[i].setDimension ( mGuiMain->getRectangle(38,1.5,15,1) );
		mDeviceNumLabel[i].setDimension ( mGuiMain->getRectangle(26,2.5,6,1) );
		mDeviceNum[i].setDimension ( mGuiMain->getRectangle(38,2.5,15,1) );
		mPartTypeLabel[i].setDimension ( mGuiMain->getRectangle(26,3.5,6,1) );
		mPartType[i].setDimension ( mGuiMain->getRectangle(38,3.5,15,1) );
		mPartNumLabel[i].setDimension ( mGuiMain->getRectangle(26,4.5,6,1) );
		mPartNum[i].setDimension ( mGuiMain->getRectangle(38,4.5,15,1) );
		mDirectionLabel[i].setDimension ( mGuiMain->getRectangle(26,5.5,6,1) );
		mDirection[i].setDimension ( mGuiMain->getRectangle(38,5.5,15,1) );
		mAutoDetect[i].setDimension ( mGuiMain->getRectangle(26,7.0,6,1) );

		mA5200Dec[i].setDimension ( mGuiMain->getRectangle(0.8,0.5,23.0,18.3) );
		mKeypadDec[i].setDimension ( mGuiMain->getRectangle(1.5,10.5,21,8.3) );
		for ( j = 0; j < 10; ++j ) {
			mKey[j][i].setDimension ( mGuiMain->getRectangle(dim_x[j],dim_y[j],2,1) );
		}
		mStarKey[i].setDimension ( mGuiMain->getRectangle(6,17,6,1) );
		mPoundKey[i].setDimension ( mGuiMain->getRectangle(16,17,6,1) );
		mStartKey[i].setDimension ( mGuiMain->getRectangle(2,1.5,6,1) );
		mPauseKey[i].setDimension ( mGuiMain->getRectangle(9,1.5,6,1) );
		mResetKey[i].setDimension ( mGuiMain->getRectangle(16,1.5,6,1) );

		mTopButton[i].setDimension ( mGuiMain->getRectangle(1,3.5,6,1) );
		mBottomButton[i].setDimension ( mGuiMain->getRectangle(12,3.5,6,1) );
		m800Button[i].setDimension ( mGuiMain->getRectangle(17,8.8,6,1) );

		mStickDec[i].setDimension ( mGuiMain->getRectangle(1.5,4.5,21,6.0) );
		mStickDirection[i][DIR_LEFT].setDimension ( mGuiMain->getRectangle(4,7.0,6,1) );
		mStickDirection[i][DIR_RIGHT].setDimension ( mGuiMain->getRectangle(14,7.0,6,1) );
		mStickDirection[i][DIR_UP].setDimension ( mGuiMain->getRectangle(10,5.5,6,1) );
		mStickDirection[i][DIR_DOWN].setDimension ( mGuiMain->getRectangle(9,8.5,6,1) );

		mPaddlesDec[i].setDimension ( mGuiMain->getRectangle(1.5,0.4,21.0,4.0) );
		mPaddle1Label[i].setDimension ( mGuiMain->getRectangle(9,1.4,8,1) );
		mPaddle2Label[i].setDimension ( mGuiMain->getRectangle(9,3.1,8,1) );
		mPaddleDirection[i][DIR_LEFT].setDimension ( mGuiMain->getRectangle(10.5,1.2,6,1) );
		mPaddleDirection[i][DIR_RIGHT].setDimension ( mGuiMain->getRectangle(16,1.2,6,1) );
		mPaddleDirection[i][DIR_UP].setDimension ( mGuiMain->getRectangle(10.5,2.7,6,1) );
		mPaddleDirection[i][DIR_DOWN].setDimension ( mGuiMain->getRectangle(16,2.7,6,1) );
	}

	/*
	 * 800 Keyboards
	 */
	mKeyboardTab.setDimension ( mGuiMain->getRectangle(-1,-1,55,21) );
	mPCDec[i].setDimension ( mGuiMain->getRectangle(3.0,0.0,29.5,8.5) );
	mDeviceLabel[i].setDimension ( mGuiMain->getRectangle(4,1,6,1) );
	mDevice[i].setDimension ( mGuiMain->getRectangle(16,1,15,1) );
	mDeviceNumLabel[i].setDimension ( mGuiMain->getRectangle(4,2,6,1) );
	mDeviceNum[i].setDimension ( mGuiMain->getRectangle(16,2,15,1) );
	mPartTypeLabel[i].setDimension ( mGuiMain->getRectangle(4,3,6,1) );
	mPartType[i].setDimension ( mGuiMain->getRectangle(16,3,15,1) );
	mPartNumLabel[i].setDimension ( mGuiMain->getRectangle(4,4,6,1) );
	mPartNum[i].setDimension ( mGuiMain->getRectangle(16,4,15,1) );
	mDirectionLabel[i].setDimension ( mGuiMain->getRectangle(4,5,6,1) );
	mDirection[i].setDimension ( mGuiMain->getRectangle(16,5,15,1) );
	mAutoDetect[i].setDimension ( mGuiMain->getRectangle(4,6.5,6,1) );

	mKeysDec.setDimension ( mGuiMain->getRectangle(setColumn(-0.5),setRow(-0.6),50,10) );
	set800KeyDimensions();

	mControlsDec.setDimension ( mGuiMain->getRectangle(34.5,0,18,8.5) );
	m800Key[A800_KEY_HELP].setDimension ( mGuiMain->getRectangle(45,1.5,2,1) );
	m800StartKey.setDimension ( mGuiMain->getRectangle(36,1.5,2,1) );
	m800SelectKey.setDimension ( mGuiMain->getRectangle(36,3,2,1) );
	m800OptionKey.setDimension ( mGuiMain->getRectangle(36,4.5,2,1) );
	m800ResetKey.setDimension ( mGuiMain->getRectangle(45,3,2,1) );
	m800ShiftKey.setDimension ( mGuiMain->getRectangle(setColumn(0),setRow(3),2,1) );
	m800CtrlKey.setDimension ( mGuiMain->getRectangle(setColumn(0),setRow(2),2,1) );
	m800BrkKey.setDimension ( mGuiMain->getRectangle(setColumn(16.3),setRow(0),2,1) );

	mProfile.adjustHeight();
	mDescriptionBox.adjustHeight();
	mOk.adjustSize();
	mCancel.adjustSize();
	mRename.adjustSize();
	mSaveAs.adjustSize();

	mDefault.adjustSize();

	mMouseLabel.adjustSize();

	for ( i = 0; i < 4; ++i ) {
		mSensitivityLabel[i].adjustSize();
		mDeadzoneLabel[i].adjustSize();
		mSensitivityValue[i].adjustSize();
		mDeadzoneValue[i].adjustSize();
		mSimulateAnalog[i].adjustSize();

		mPluggedIn[i].adjustHeight();

		mDeviceLabel[i].adjustSize();
		mDevice[i].adjustHeight();
		mDeviceNumLabel[i].adjustSize();
		mDeviceNum[i].adjustHeight();
		mPartTypeLabel[i].adjustSize();
		mPartType[i].adjustHeight();
		mPartNumLabel[i].adjustSize();
		mPartNum[i].adjustHeight();
		mDirectionLabel[i].adjustSize();
		mDirection[i].adjustHeight();
		mAutoDetect[i].adjustSize();

		mPaddle1Label[i].adjustSize();
		mPaddle2Label[i].adjustSize();
		mPaddleDirection[i][DIR_LEFT].adjustSize();
		mPaddleDirection[i][DIR_RIGHT].adjustSize();
		mPaddleDirection[i][DIR_UP].adjustSize();
		mPaddleDirection[i][DIR_DOWN].adjustSize();

		sprintf ( player_string, "Player %d", i+1 );

		for ( j = 0; j < 10; ++j ) {
			sprintf ( tmp_string, "%d", j );
			setupInputButton ( &mKey[j][i], tmp_string, player_string, this,
			                                 &mPlayerTab[i], &mInput.players[i].keypad[key[j]] );
		}
		setupInputButton ( &mStarKey[i], "*", player_string, this,
		                      &mPlayerTab[i], &mInput.players[i].keypad[KEY_STAR>>1] );

		setupInputButton ( &mPoundKey[i], "#", player_string, this, &mPlayerTab[i], 
		                      &mInput.players[i].keypad[KEY_POUND>>1] );

		setupInputButton ( &mStartKey[i], "Start", player_string, this,
		                      &mPlayerTab[i], &mInput.players[i].keypad[KEY_START>>1] );

		setupInputButton ( &mPauseKey[i], "Pause", player_string, this,
		                      &mPlayerTab[i], &mInput.players[i].keypad[KEY_PAUSE>>1] );

		setupInputButton ( &mResetKey[i], "Reset", player_string, this,
		                      &mPlayerTab[i], &mInput.players[i].keypad[KEY_RESET>>1] );

		setupInputButton ( &mTopButton[i], "Top Button", player_string, this, 
		                      &mPlayerTab[i], &mInput.players[i].top_button );

		setupInputButton ( &mBottomButton[i], "Btm Button", player_string, this, 
		                      &mPlayerTab[i], &mInput.players[i].bottom_button );

		setupInputButton ( &m800Button[i], "Btn", player_string, this,
		                      &mPlayerTab[i], &mInput.players[i].bottom_button );

		setupInputButton ( &mStickDirection[i][DIR_LEFT], "Left", player_string, this,
		                      &mPlayerTab[i], 
		                      &mInput.players[i].stick.direction[DIR_LEFT] );

		setupInputButton ( &mStickDirection[i][DIR_RIGHT], "Right", player_string, 
		                       this, &mPlayerTab[i], 
		                       &mInput.players[i].stick.direction[DIR_RIGHT] );

		setupInputButton ( &mStickDirection[i][DIR_UP], "Up", player_string, this, 
                               &mPlayerTab[i], 
		                       &mInput.players[i].stick.direction[DIR_UP] );

		setupInputButton ( &mStickDirection[i][DIR_DOWN], "Down", player_string, this,
		                       &mPlayerTab[i], 
		                       &mInput.players[i].stick.direction[DIR_DOWN] );

		setupInputButton ( &mPaddleDirection[i][DIR_LEFT], "Left", player_string, 
		                        this, &mPlayerTab[i],
		                        &mInput.players[i].paddles.direction[DIR_LEFT] );

		setupInputButton ( &mPaddleDirection[i][DIR_RIGHT], "Right", player_string, 
		                        this, &mPlayerTab[i],
		                        &mInput.players[i].paddles.direction[DIR_RIGHT] );

		setupInputButton ( &mPaddleDirection[i][DIR_UP], "Left", player_string, this,
		                        &mPlayerTab[i], 
		                        &mInput.players[i].paddles.direction[DIR_UP] );

		setupInputButton ( &mPaddleDirection[i][DIR_DOWN], "Right", player_string, 
		                        this, &mPlayerTab[i],
		                        &mInput.players[i].paddles.direction[DIR_DOWN] );

		mStickDirection[i][DIR_LEFT].setSelected ( true );
	}

	strcpy ( player_string, "Keyboard" );

	for ( j = 0; j < 64; ++j ) {
		setupInputButton ( &m800Key[j], getKeyTitle(j), player_string, this,
		                      &mKeyboardTab, &mInput.keyboard[j] );
		if ( m800Key[j].getCaption () == "" )
			m800Key[j].setVisible ( false );
	}

	setupInputButton ( &m800SelectKey, "Select", player_string, this, 
	                       &mKeyboardTab, &mInput.select_key );
	setupInputButton ( &m800StartKey, "Start", player_string, this,
	                       &mKeyboardTab, &mInput.start_key );
	setupInputButton ( &m800OptionKey, "Option", player_string, this,
	                       &mKeyboardTab, &mInput.option_key );
	setupInputButton ( &m800ShiftKey, "Shift", player_string, this,
	                       &mKeyboardTab, &mInput.shift_key );
	setupInputButton ( &m800CtrlKey, "Ctrl", player_string, this,
	                       &mKeyboardTab, &mInput.ctrl_key );
	setupInputButton ( &m800BrkKey, "Break", player_string, this,
	                       &mKeyboardTab, &mInput.break_key );
	setupInputButton ( &m800ResetKey, "Reset", player_string, this,
	                       &mKeyboardTab, &mInput.reset_key );

	m800StartKey.setSelected ( true );

	mDeviceLabel[i].adjustSize();
	mDevice[i].adjustHeight();
	mDeviceNumLabel[i].adjustSize();
	mDeviceNum[i].adjustHeight();
	mPartTypeLabel[i].adjustSize();
	mPartType[i].adjustHeight();
	mPartNumLabel[i].adjustSize();
	mPartNum[i].adjustHeight();
	mDirectionLabel[i].adjustSize();
	mDirection[i].adjustHeight();
	mAutoDetect[i].adjustSize();

	/*
	 * Initialize the Controls list
	 */
	initGroupControls();
}

InputCfg::~InputCfg ()
{
	int i;

	/*
	 * Remove the tabs before killing them
	 */
	for ( i = 0; i < 4; ++i ) {
		mPlayerWindow.removeTab ( &mPlayerTab[i] );
	}

	if ( mKeyboardShowing )
		mPlayerWindow.removeTab ( &mKeyboardTab );
}

void InputCfg::setupInputButton (GuiInputButton *btn, std::string caption, 
		                         std::string grp, gcn::ActionListener *listener, 
		                         gcn::Window *parent, t_atari_common *ctrl )
{
		btn->setCaption ( caption );
		btn->setGroup ( grp );
		btn->addActionListener ( listener );
		btn->setActionEventId ( "mInputButton" );
		parent->add ( btn );
		btn->adjustSize ( );
		btn->mCtrl = ctrl;
}

void InputCfg::show ( )
{
	t_config *p_config = config_get_ptr();
	std::string filestring;

	/*
	 * Load current media's profile, unless machine is different
	 */
	if ( (p_config->machine_type == media_get_ptr()->machine_type) ||
	     (p_config->machine_type != MACHINE_TYPE_5200 && 
	     media_get_ptr()->machine_type != MACHINE_TYPE_5200) )
		input_set_profile ( media_get_ptr()->input_profile );

	mInput = g_input;

	setMachine ( p_config->machine_type );

	initProfileList ();
	mProfile.setSelected ( mProfileList.getIndexFromLabel(mInput.name));

	filestring = mInput.name;

	mSettingsDirty = false;
	mAutoDetectAct = -1;

	if ( getParent () )
		getParent()->moveToTop (this);

	changeGroup ( filestring );
	setVisible ( true );
}

void InputCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void InputCfg::initProfileList ( )
{
	struct generic_node *nodeptr = 0;

	mProfileList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Input", &nodeptr );
	mProfileList.setList ( nodeptr );
}

void InputCfg::initGroupControls ( )
{
	int i;

	mMouseList.deleteAll();
	mMouseList.addElement ( "1", 1, 0 );
	mMouseList.addElement ( "2", 2, 0 );
	mMouseList.addElement ( "3", 3, 0 );
	mMouseList.addElement ( "4", 4, 0 );
	mMouseList.addElement ( "5", 5, 0 );

	for ( i = 0; i < 5; ++i ) {
		if ( i < 4 ) {
			mPluggedInList[i].deleteAll();
			mPluggedInList[i].addElement ( "None", CTRLR_TYPE_NONE, 0 );
			mPluggedInList[i].addElement ( "Joystick", CTRLR_TYPE_JOYSTICK, 0 );
			mPluggedInList[i].addElement ( "Trackball", CTRLR_TYPE_TRACKBALL, 0 );
			mPluggedInList[i].addElement ( "Paddles", CTRLR_TYPE_PADDLES, 0 );
			mPluggedInList[i].addElement ( "Keypad", CTRLR_TYPE_KEYPAD, 0 );
		}

		mDeviceList[i].deleteAll();
		mDeviceList[i].addElement ( "Keyboard", DEV_KEYBOARD, 0 );
		mDeviceList[i].addElement ( "Joystick", DEV_JOYSTICK, 0 );
		mDeviceList[i].addElement ( "Mouse", DEV_MOUSE, 0 );

		mPartTypeList[i].deleteAll();
		mPartTypeList[i].addElement ( "D-Pad/Hat", PART_TYPE_HAT, 0 );
		mPartTypeList[i].addElement ( "Axis", PART_TYPE_AXIS, 0 );
		mPartTypeList[i].addElement ( "TrackBall", PART_TYPE_BALL, 0 );
		mPartTypeList[i].addElement ( "Button", PART_TYPE_BUTTON, 0 );
		mPartTypeList[i].addElement ( "Key", PART_TYPE_KEY, 0 );

		mDirectionList[i].deleteAll();
		mDirectionList[i].addElement ( "Left", DIR_LEFT, 0 );
		mDirectionList[i].addElement ( "Right", DIR_RIGHT, 0 );
		mDirectionList[i].addElement ( "Up", DIR_UP, 0 );
		mDirectionList[i].addElement ( "Down", DIR_DOWN, 0 );
		mDirectionList[i].addElement ( "+", DIR_PLUS, 0 );
		mDirectionList[i].addElement ( "-", DIR_MINUS, 0 );
		mDirectionList[i].addElement ( "Press", DIR_PRESS, 0 );
		mDirectionList[i].addElement ( "Release", DIR_RELEASE, 0 );
	}
}

void InputCfg::setMachine ( e_machine_type type )
{
	int i;

	if ( type == MACHINE_TYPE_5200 )
	{
		for ( i = 0; i < 4; ++i )
	   	{
			mSimulateAnalog[i].setVisible ( true );
			mStartKey[i].setVisible ( true );
			mPauseKey[i].setVisible ( true );
			mResetKey[i].setVisible ( true );
			mTopButton[i].setVisible ( true );
			mBottomButton[i].setVisible ( true );
			m800Button[i].setVisible ( false );
			mPaddle1Label[i].setVisible ( false );
			mPaddle2Label[i].setVisible ( false );
			mPaddleDirection[i][DIR_LEFT].setVisible ( false );
			mPaddleDirection[i][DIR_RIGHT].setVisible ( false );
			mPaddleDirection[i][DIR_UP].setVisible ( false );
			mPaddleDirection[i][DIR_DOWN].setVisible ( false );

			mA5200Dec[i].setVisible ( true );
			mPaddlesDec[i].setVisible ( false );
			mStickDec[i].setVisible ( false );
			mKeypadDec[i].setVisible ( false );

			mTypeDec[i].setCaption ( "A5200 Controller" );
			mPluggedInList[i].deleteAll();
			mPluggedInList[i].addElement ( "None", CTRLR_TYPE_NONE, 0 );
			mPluggedInList[i].addElement ( "Joystick", CTRLR_TYPE_JOYSTICK, 0 );
			mPluggedInList[i].addElement ( "Trackball", CTRLR_TYPE_TRACKBALL, 0 );
		}
		if ( mKeyboardShowing ) {
			mPlayerWindow.removeTab ( &mKeyboardTab );
			mKeyboardShowing = false;
		}
	}
	else 
	{
		for ( i = 0; i < 4; ++i )
	   	{
			mSimulateAnalog[i].setVisible ( false );
			mStartKey[i].setVisible ( false );
			mPauseKey[i].setVisible ( false );
			mResetKey[i].setVisible ( false );
			mTopButton[i].setVisible ( false );
			mBottomButton[i].setVisible ( false );
			m800Button[i].setVisible ( true );
			mPaddle1Label[i].setVisible ( true );
			mPaddle2Label[i].setVisible ( true );
			mPaddleDirection[i][DIR_LEFT].setVisible ( true );
			mPaddleDirection[i][DIR_RIGHT].setVisible ( true );
			mPaddleDirection[i][DIR_UP].setVisible ( true );
			mPaddleDirection[i][DIR_DOWN].setVisible ( true );

			mA5200Dec[i].setVisible ( false );
			mPaddlesDec[i].setVisible ( true );
			mStickDec[i].setVisible ( true );
			mKeypadDec[i].setVisible ( true );

			mTypeDec[i].setCaption ( "A800 Controller" );
			mPluggedInList[i].deleteAll();
			mPluggedInList[i].addElement ( "None", CTRLR_TYPE_NONE, 0 );
			mPluggedInList[i].addElement ( "Joystick", CTRLR_TYPE_JOYSTICK, 0 );
			mPluggedInList[i].addElement ( "Paddles", CTRLR_TYPE_PADDLES, 0 );
			mPluggedInList[i].addElement ( "Keypad", CTRLR_TYPE_KEYPAD, 0 );
		}
		if ( !mKeyboardShowing ) {
			mPlayerWindow.addTab ( 0, &mKeyboardTab );
			mKeyboardShowing = true;
		}
	}
}

void InputCfg::saveSettings ( std::string profile_name )
{
	int i;
	t_atari_input *p_input = &mInput;
	t_config *p_config = config_get_ptr();

	int key[] = {KEY_0>>1,KEY_1>>1,KEY_2>>1,KEY_3>>1,KEY_4>>1,
	             KEY_5>>1,KEY_6>>1,KEY_7>>1,KEY_8>>1,KEY_9>>1};

	p_input->mouse_speed = mMouseList.getValueFromIndex ( mMouse.getSelected() );

	strcpy ( p_input->description, mDescriptionBox.getText().c_str() );

	// Figure out which inputs are selected and update those
	for ( i = 0; i < 4; ++i ) {
		p_input->players[i].stick.deadzone = (int)(mDeadzone[i].getValue() + 0.5);
		p_input->players[i].stick.sensitivity = (int)(mSensitivity[i].getValue() + 0.5);
		if ( mSimulateAnalog[i].isSelected() )
			p_input->players[i].stick.simulate_analog = 1;
		else
			p_input->players[i].stick.simulate_analog = 0;

		p_input->players[i].control_type = (e_ctrlr_type) mPluggedInList[i].getValueFromIndex( mPluggedIn[i].getSelected() );
		getInput ( i, mStartKey[i].getMarkedButton()->mCtrl );

	} /* end for player */

	/*
	 * Now check keyboard for the 800
	 */
	getInput ( i, m800StartKey.getMarkedButton()->mCtrl );

	p_input->machine_type = p_config->machine_type;

	if ( !input_save_profile ( (char *)profile_name.c_str(), p_input ) ) {

		strcpy ( p_input->name, profile_name.c_str() );

		initProfileList();
		mProfile.setSelected ( mProfileList.getIndexFromLabel(p_input->name));

		if ( mDefault.isSelected() == true ) {
			if ( p_config->machine_type == MACHINE_TYPE_5200 )
				strcpy ( p_config->default_input_profile, p_input->name );
			else
				strcpy ( p_config->default_800input_profile, p_input->name );
			config_save();
		}
	}

}

void InputCfg::changeGroup ( std::string profile_name )
{
	t_atari_input *p_input = &mInput;
	t_config *p_config = config_get_ptr();
	char tmp_string[10];
	char *d_profile;
	int i;
	int player = mPlayerWindow.getSelectedIndex();

	int key[] = {KEY_0>>1,KEY_1>>1,KEY_2>>1,KEY_3>>1,KEY_4>>1,
	             KEY_5>>1,KEY_6>>1,KEY_7>>1,KEY_8>>1,KEY_9>>1};

	mSettingsDirty = false;

	if ( p_config->machine_type == MACHINE_TYPE_5200 )
		d_profile = p_config->default_input_profile;
	else
		d_profile = p_config->default_800input_profile;

	/*
	 * Get values for this Group
	 */
	if ( input_load_profile ( profile_name.c_str(), &mInput ) ) {

		mGuiMain->showError(ERR_PROFILE_LOAD);

		input_load_profile ( d_profile, &mInput );
		profile_name = d_profile;
	}

	mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

	mDescriptionBox.setText ( p_input->description );

	if ( !strcmp(profile_name.c_str(), d_profile) )
		mDefault.setSelected ( true );
	else
		mDefault.setSelected ( false );

	mMouse.setSelected ( mMouseList.getIndexFromValue(p_input->mouse_speed) );

	/*
	 * Set values on the GUI
	 */
	for ( i = 0; i < 4; ++i ) {
		mDeadzone[i].setValue ( p_input->players[i].stick.deadzone );
		mSensitivity[i].setValue ( p_input->players[i].stick.sensitivity );

		sprintf ( tmp_string, "%3.0lf%%", mDeadzone[i].getValue() );
		mDeadzoneValue[i].setCaption( tmp_string );
		mDeadzoneValue[i].adjustSize();

		sprintf ( tmp_string, "%3.0lf%%", mSensitivity[i].getValue() );
		mSensitivityValue[i].setCaption( tmp_string );
		mSensitivityValue[i].adjustSize();

		if ( p_input->players[i].stick.simulate_analog )
			mSimulateAnalog[i].setSelected( true );
		else
			mSimulateAnalog[i].setSelected( false );

		mPluggedIn[i].setSelected ( mPluggedInList[i].getIndexFromValue(p_input->players[i].control_type) );

		setInput ( i, mStartKey[i].getMarkedButton()->mCtrl );

	} /* for each player */

	/*
	 * Now look at 800 keyboard
	 */
	setInput ( i, m800StartKey.getMarkedButton()->mCtrl );

	/*
	 * Make sure that correct current control is assigned
	 */
	if ( player < 4 )
	{
		setInput ( player, mStartKey[player].getMarkedButton()->mCtrl );
	}
}

void InputCfg::getInput( int player, t_atari_common *ctrl )
{
	char tmp_string[256];

	ctrl->device = (e_dev_type) mDeviceList[player].getValueFromIndex( mDevice[player].getSelected() );
	ctrl->device_num = atoi ( mDeviceNum[player].getText().c_str() );
	ctrl->part_type = (e_part_type) mPartTypeList[player].getValueFromIndex( mPartType[player].getSelected() );
	if ( ctrl->device == DEV_KEYBOARD ) {
		strcpy ( tmp_string, mPartNum[player].getText().c_str() );
		ctrl->part_num = pc_interpret_key ( tmp_string );
	}
	else {
		ctrl->part_num = atoi ( mPartNum[player].getText().c_str() );
	}

	ctrl->direction = (e_direction) mDirectionList[player].getValueFromIndex( mDirection[player].getSelected() );
}

void InputCfg::setInput( int player, t_atari_common *ctrl )
{
	char tmp_string[20];

	mCurrentCtrl = ctrl;

	mDevice[player].setSelected ( mDeviceList[player].getIndexFromValue(ctrl->device) );
	sprintf ( tmp_string, "%d", ctrl->device_num );
	mDeviceNum[player].setText( tmp_string );

	mPartType[player].setSelected( mPartTypeList[player].getIndexFromValue( ctrl->part_type ) );

	if ( ctrl->device == DEV_KEYBOARD ) {
		pc_interpret_key_value ( ctrl->part_num, tmp_string );
	}
	else {
		sprintf ( tmp_string, "%d", ctrl->part_num );
	}
	mPartNum[player].setText( tmp_string );

	mDirection[player].setSelected ( mDirectionList[player].getIndexFromValue(ctrl->direction) );
}

void InputCfg::action(const gcn::ActionEvent &actionEvent)
{
	t_atari_input *p_input = &mInput;
	t_config *p_config = config_get_ptr();
	char tmp_string[256];
	std::string new_name;
	static int first_axis_event;
	int player = mPlayerWindow.getSelectedIndex();
	int key[] = {KEY_0>>1,KEY_1>>1,KEY_2>>1,KEY_3>>1,KEY_4>>1,
	             KEY_5>>1,KEY_6>>1,KEY_7>>1,KEY_8>>1,KEY_9>>1};

	if ( actionEvent.getId() == "mOk" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mProfileList.getLabelFromIndex(mProfile.getSelected()) );
		}
		hide();
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}
	else if ( actionEvent.getId() == "mProfile" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mInput.name );
		}
		changeGroup (mProfileList.getLabelFromIndex(mProfile.getSelected()) );
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
				sprintf ( tmp_string, "UPDATE Input SET Name='%s' WHERE Name='%s'", 
				          new_name.c_str(), mProfileList.getLabelFromIndex(mProfile.getSelected()) );
				db_if_exec_sql ( tmp_string, NULL, NULL );
				initProfileList();
				changeGroup (new_name );
			}
		}
	}
	else if ( actionEvent.getId() == "mSaveAs" ) {
		mGuiMain->getProfilePopup()->deleteActions();
		mGuiMain->getProfilePopup()->addActionListener(this);
		mGuiMain->getProfilePopup()->setActionEventId("SaveBrowser");
		mGuiMain->getProfilePopup()->show( "Choose Profile or Create New", "SELECT Name FROM Input", 
		                     mProfileList.getLabelFromIndex(mProfile.getSelected()), true );
	}
	else if ( actionEvent.getId() == "SaveBrowser" ) {
		if ( mGuiMain->getProfilePopup()->getCancelPressed() == false ) {
			saveSettings ( mGuiMain->getProfilePopup()->getProfile() );
			mSettingsDirty = false;
		}
	}
	else if ( actionEvent.getId() == "mDeadzone" ) {
		sprintf ( tmp_string, "%3.0lf%%", mDeadzone[player].getValue() );
		mDeadzoneValue[player].setCaption( tmp_string );
		mDeadzoneValue[player].adjustSize();

		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mSensitivity" ) {
		sprintf ( tmp_string, "%3.0lf%%", mSensitivity[player].getValue() );
		mSensitivityValue[player].setCaption( tmp_string );
		mSensitivityValue[player].adjustSize();

		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mDirty" ) {
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mInputButton" ) {
		getInput ( player, mCurrentCtrl );
		setInput ( player, ((GuiInputButton *)(actionEvent.getSource()))->mCtrl ); 
	}
	else if ( actionEvent.getId() == "mAutoDetect" ) {
		if ( mAutoDetectAct == -1 ) 
		{
			pc_detect_and_open_joysticks ();

			first_axis_event = 1;
			mGuiMain->getKatGui()->setJoystickEnable ( false );

			mAutoListener.mCountModKey = true;
			mAutoListener.setWindow ( mGuiMain->getAutoWindow() );

			mGuiMain->getAutoWindow()->deleteAllListeners ( );
			mGuiMain->getAutoWindow()->addActionListener ( this );
			mGuiMain->getAutoWindow()->addKeyListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->addMouseListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->addJoystickListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->setActionEventId ( "mAutoWindow" );
			mGuiMain->getAutoWindow()->setDimension ( mGuiMain->getRectangle(7,11.0,60,7) );

			mGuiMain->showAutoWindow( "User Action Required", "Perform action to associate with controller part" );
		}
		else {
			mAutoDetectAct--;
		}
	}
	else if ( actionEvent.getId() == "mAutoWindow" ) {
		mAutoListener.getInput ( player, &mCurrentCtrl->device, 
		                                  &mCurrentCtrl->device_num,
		                                  &mCurrentCtrl->part_type, 
		                                  &mCurrentCtrl->part_num,
		                                  &mCurrentCtrl->direction );
		if ( mCurrentCtrl->device != DEV_JOYSTICK || 
		     mCurrentCtrl->part_type != PART_TYPE_AXIS || !first_axis_event )
		{
			mSettingsDirty = true;
			mGuiMain->hideAutoWindow();
			setInput ( player, mCurrentCtrl );
			if ( mCurrentCtrl->device == DEV_MOUSE && 
			     mCurrentCtrl->part_type == PART_TYPE_BUTTON )
				mAutoDetectAct = 0;
		}
		first_axis_event = 0;
	}
}

void InputCfg::mousePressed(gcn::MouseEvent& mouseEvent)
{
	int i;

	bool comp_has_mouse = false;

	for ( i = 0; i < 4; ++i ) {
		if ( &mDeviceNum[i] == mouseEvent.getSource() || 
		     &mPartNum[i] == mouseEvent.getSource() )
			comp_has_mouse = true;
	}
    if ((comp_has_mouse == true) && mouseEvent.getButton() == gcn::MouseEvent::LEFT)
  		{
		mSettingsDirty = true;
   	}

	gcn::Window::mousePressed(mouseEvent);
}

std::string InputCfg::getKeyTitle(int key)
{
	switch ( key )
	{
		case A800_KEY_L:      return "L"; break;
		case A800_KEY_J:      return "J"; break;
		case A800_KEY_SEMI:   return ";"; break;
		case A800_KEY_K:      return "K"; break;
		case A800_KEY_PLUS:   return "+"; break;
		case A800_KEY_STAR:   return "*"; break;
		case A800_KEY_O:      return "O"; break;
		case A800_KEY_P:      return "P"; break;
		case A800_KEY_U:      return "U"; break;
		case A800_KEY_RETURN: return "RETURN"; break;
		case A800_KEY_I:      return "I"; break;
		case A800_KEY_MINUS:  return "-"; break;
		case A800_KEY_EQUALS: return "="; break;
		case A800_KEY_V:      return "V"; break;
		case A800_KEY_HELP:   return "HELP"; break;
		case A800_KEY_C:      return "C"; break;
		case A800_KEY_B:      return "B"; break;
		case A800_KEY_X:      return "X"; break;
		case A800_KEY_Z:      return "Z"; break;
		case A800_KEY_4:      return "4"; break;
		case A800_KEY_3:      return "3"; break;
		case A800_KEY_6:      return "6"; break;
		case A800_KEY_ESC:    return "ESC"; break;
		case A800_KEY_5:      return "5"; break;
		case A800_KEY_2:      return "2"; break;
		case A800_KEY_1:      return "1"; break;
		case A800_KEY_COMMA:  return ","; break;
		case A800_KEY_SPACE:  return "SPACE"; break;
		case A800_KEY_PERIOD: return "."; break;
		case A800_KEY_N:      return "N"; break;
		case A800_KEY_M:      return "M"; break;
		case A800_KEY_SLASH:  return "/"; break;
		case A800_KEY_FUJI:   return "FUJI"; break;
		case A800_KEY_R:      return "R"; break;
		case A800_KEY_E:      return "E"; break;
		case A800_KEY_Y:      return "Y"; break;
		case A800_KEY_TAB:    return "TAB"; break;
		case A800_KEY_T:      return "T"; break;
		case A800_KEY_W:      return "W"; break;
		case A800_KEY_Q:      return "Q"; break;
		case A800_KEY_9:      return "9"; break;
		case A800_KEY_0:      return "0"; break;
		case A800_KEY_7:      return "7"; break;
		case A800_KEY_BKSP: return "BKSP"; break;
		case A800_KEY_8:      return "8"; break;
		case A800_KEY_LESST:  return "<"; break;
		case A800_KEY_MORET:  return ">"; break;
		case A800_KEY_F:      return "F"; break;
		case A800_KEY_H:      return "H"; break;
		case A800_KEY_D:      return "D"; break;
		case A800_KEY_CAPS:   return "CAPS"; break;
		case A800_KEY_G:      return "G"; break;
		case A800_KEY_S:      return "S"; break;
		case A800_KEY_A:      return "A"; break;
		default: return ""; break;

	} /* end switch key */
}

double InputCfg::setRow( double row )
{
	return 10 + (row * 1.8);
}

double InputCfg::setColumn( double column )
{
	return 4 + (column * 2.4);
}

void InputCfg::set800KeyDimensions()
{
	m800Key[A800_KEY_L].setDimension ( mGuiMain->getRectangle(setColumn(10.1),setRow(2),2,1) );
	m800Key[A800_KEY_J].setDimension ( mGuiMain->getRectangle(setColumn(8.1),setRow(2),2,1) );
	m800Key[A800_KEY_SEMI].setDimension ( mGuiMain->getRectangle(setColumn(11.1),setRow(2),2,1) );
	m800Key[A800_KEY_K].setDimension ( mGuiMain->getRectangle(setColumn(9.1),setRow(2),2,1) );
	m800Key[A800_KEY_PLUS].setDimension ( mGuiMain->getRectangle(setColumn(12.1),setRow(2),2,1) );
	m800Key[A800_KEY_STAR].setDimension ( mGuiMain->getRectangle(setColumn(13.1),setRow(2),2,1) );
	m800Key[A800_KEY_O].setDimension ( mGuiMain->getRectangle(setColumn(9.9),setRow(1),2,1) );
	m800Key[A800_KEY_P].setDimension ( mGuiMain->getRectangle(setColumn(10.9),setRow(1),2,1) );
	m800Key[A800_KEY_U].setDimension ( mGuiMain->getRectangle(setColumn(7.9),setRow(1),2,1) );
	m800Key[A800_KEY_RETURN].setDimension ( mGuiMain->getRectangle(setColumn(13.9),setRow(1),2,1) );
	m800Key[A800_KEY_I].setDimension ( mGuiMain->getRectangle(setColumn(8.9),setRow(1),2,1) );
	m800Key[A800_KEY_MINUS].setDimension ( mGuiMain->getRectangle(setColumn(11.9),setRow(1),2,1) );
	m800Key[A800_KEY_EQUALS].setDimension ( mGuiMain->getRectangle(setColumn(12.9),setRow(1),2,1) );
	m800Key[A800_KEY_V].setDimension ( mGuiMain->getRectangle(setColumn(5.5),setRow(3),2,1) );
	m800Key[A800_KEY_C].setDimension ( mGuiMain->getRectangle(setColumn(4.5),setRow(3),2,1) );
	m800Key[A800_KEY_B].setDimension ( mGuiMain->getRectangle(setColumn(6.5),setRow(3),2,1) );
	m800Key[A800_KEY_X].setDimension ( mGuiMain->getRectangle(setColumn(3.5),setRow(3),2,1) );
	m800Key[A800_KEY_Z].setDimension ( mGuiMain->getRectangle(setColumn(2.5),setRow(3),2,1) );
	m800Key[A800_KEY_4].setDimension ( mGuiMain->getRectangle(setColumn(4.9),setRow(0),2,1) );
	m800Key[A800_KEY_3].setDimension ( mGuiMain->getRectangle(setColumn(3.9),setRow(0),2,1) );
	m800Key[A800_KEY_6].setDimension ( mGuiMain->getRectangle(setColumn(6.9),setRow(0),2,1) );
	m800Key[A800_KEY_ESC].setDimension ( mGuiMain->getRectangle(setColumn(0),setRow(0),2,1) );
	m800Key[A800_KEY_5].setDimension ( mGuiMain->getRectangle(setColumn(5.9),setRow(0),2,1) );
	m800Key[A800_KEY_2].setDimension ( mGuiMain->getRectangle(setColumn(2.9),setRow(0),2,1) );
	m800Key[A800_KEY_1].setDimension ( mGuiMain->getRectangle(setColumn(1.9),setRow(0),2,1) );
	m800Key[A800_KEY_COMMA].setDimension ( mGuiMain->getRectangle(setColumn(9.6),setRow(3),2,1) );
	m800Key[A800_KEY_SPACE].setDimension ( mGuiMain->getRectangle(setColumn(8),setRow(4),2,1) );
	m800Key[A800_KEY_PERIOD].setDimension ( mGuiMain->getRectangle(setColumn(10.5),setRow(3),2,1) );
	m800Key[A800_KEY_N].setDimension ( mGuiMain->getRectangle(setColumn(7.5),setRow(3),2,1) );
	m800Key[A800_KEY_M].setDimension ( mGuiMain->getRectangle(setColumn(8.5),setRow(3),2,1) );
	m800Key[A800_KEY_SLASH].setDimension ( mGuiMain->getRectangle(setColumn(11.5),setRow(3),2,1) );
	m800Key[A800_KEY_FUJI].setDimension ( mGuiMain->getRectangle(setColumn(12.5),setRow(3),2,1) );
	m800Key[A800_KEY_R].setDimension ( mGuiMain->getRectangle(setColumn(4.9),setRow(1),2,1) );
	m800Key[A800_KEY_E].setDimension ( mGuiMain->getRectangle(setColumn(3.9),setRow(1),2,1) );
	m800Key[A800_KEY_Y].setDimension ( mGuiMain->getRectangle(setColumn(6.9),setRow(1),2,1) );
	m800Key[A800_KEY_TAB].setDimension ( mGuiMain->getRectangle(setColumn(0),setRow(1),2,1) );
	m800Key[A800_KEY_T].setDimension ( mGuiMain->getRectangle(setColumn(5.9),setRow(1),2,1) );
	m800Key[A800_KEY_W].setDimension ( mGuiMain->getRectangle(setColumn(2.9),setRow(1),2,1) );
	m800Key[A800_KEY_Q].setDimension ( mGuiMain->getRectangle(setColumn(1.9),setRow(1),2,1) );
	m800Key[A800_KEY_9].setDimension ( mGuiMain->getRectangle(setColumn(9.9),setRow(0),2,1) );
	m800Key[A800_KEY_0].setDimension ( mGuiMain->getRectangle(setColumn(10.9),setRow(0),2,1) );
	m800Key[A800_KEY_7].setDimension ( mGuiMain->getRectangle(setColumn(7.9),setRow(0),2,1) );
	m800Key[A800_KEY_BKSP].setDimension ( mGuiMain->getRectangle(setColumn(13.9),setRow(0),2,1) );
	m800Key[A800_KEY_8].setDimension ( mGuiMain->getRectangle(setColumn(8.9),setRow(0),2,1) );
	m800Key[A800_KEY_LESST].setDimension ( mGuiMain->getRectangle(setColumn(11.9),setRow(0),2,1) );
	m800Key[A800_KEY_MORET].setDimension ( mGuiMain->getRectangle(setColumn(12.9),setRow(0),2,1) );
	m800Key[A800_KEY_F].setDimension ( mGuiMain->getRectangle(setColumn(5.1),setRow(2),2,1) );
	m800Key[A800_KEY_H].setDimension ( mGuiMain->getRectangle(setColumn(7.1),setRow(2),2,1) );
	m800Key[A800_KEY_D].setDimension ( mGuiMain->getRectangle(setColumn(4.1),setRow(2),2,1) );
	m800Key[A800_KEY_CAPS].setDimension ( mGuiMain->getRectangle(setColumn(14.2),setRow(2),2,1) );
	m800Key[A800_KEY_G].setDimension ( mGuiMain->getRectangle(setColumn(6.1),setRow(2),2,1) );
	m800Key[A800_KEY_S].setDimension ( mGuiMain->getRectangle(setColumn(3.1),setRow(2),2,1) );
	m800Key[A800_KEY_A].setDimension ( mGuiMain->getRectangle(setColumn(2.1),setRow(2),2,1) );
}
