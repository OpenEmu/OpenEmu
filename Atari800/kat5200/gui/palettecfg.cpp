/******************************************************************************
*
* FILENAME: palettecfg.cpp
*
* DESCRIPTION:  This class will show the sound configuration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/24/06  bberlin      Creation
******************************************************************************/
#include <math.h>
#include "palettecfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/video.h"
	#include "../interface/palette.h"
	#include "../interface/db_if.h"

	extern t_atari_video g_video;
}

PaletteCfg::PaletteCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );
	setCaption ("Palette Settings");

	/*
	 * Setup all of our widgets
	 */
	mProfileDec.setCaption ( "Profile" );
	mDescriptionLabel.setCaption ("Description");
	mRename.setCaption ("Rename");
	mSaveAs.setCaption ("Save As");
	mProfile.setListModel ( &mProfileList );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	mPalDec.setCaption ( "" );
	mIndexLabel.setCaption ("Index (0-255)");
	mValueLabel.setCaption ("Value (Hex)");
	mIndex.setText ( "0" );

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mDescriptionBox.addMouseListener ( this );
	mValue.addMouseListener ( this );
	mProfile.addActionListener ( this );
	mRename.addActionListener ( this );
	mSaveAs.addActionListener ( this );
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mIndex.addActionListener ( this );

	mRename.setActionEventId ( "mRename" );
	mSaveAs.setActionEventId ( "mSaveAs" );
	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );
	mIndex.setActionEventId ( "mIndex" );
	mProfile.setActionEventId ( "mProfile" );

	/*
	 * Size and place our widgets
	 */
	add ( &mProfileDec );
	add ( &mPalDec );
	add ( &mProfile );
	add ( &mDescriptionLabel );
	add ( &mDescriptionBox );
	add ( &mRename );
	add ( &mSaveAs );

	add ( &mOk );
	add ( &mCancel );

	add ( &mIndex );
	add ( &mIndexLabel );
	add ( &mValue );
	add ( &mValueLabel );

	setDimension ( mGuiMain->getRectangle(3,5,73,15) );

	mProfile.setDimension ( mGuiMain->getRectangle(1,1.2,50,4) );
	mDescriptionLabel.setDimension ( mGuiMain->getRectangle(1,2.8,50,4) );
	mDescriptionBox.setDimension ( mGuiMain->getRectangle(1,3.8,50,4) );
	mRename.setDimension ( mGuiMain->getRectangle(53,1.2,3,1) );
	mSaveAs.setDimension ( mGuiMain->getRectangle(61.5,1.2,3,1) );
	mProfileDec.setDimension ( mGuiMain->getRectangle(0.5,0.0,71,5.5) );

	mOk.setDimension ( mGuiMain->getRectangle(30,12,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(37,12,6,1) );

	mPalDec.setDimension ( mGuiMain->getRectangle(0.5,5.5,71,6) );
	mIndexLabel.setDimension ( mGuiMain->getRectangle(5,6.5,6,1) );
	mIndex.setDimension ( mGuiMain->getRectangle(18,6.5,6,5) );
	mValueLabel.setDimension ( mGuiMain->getRectangle(33,6.5,6,1) );
	mValue.setDimension ( mGuiMain->getRectangle(44,6.5,10,1) );

	mProfile.adjustHeight();
	mDescriptionLabel.adjustSize();
	mDescriptionBox.adjustHeight();
	mRename.adjustSize();
	mSaveAs.adjustSize();
	mOk.adjustSize();
	mCancel.adjustSize();

	mIndexLabel.adjustSize();
	mValueLabel.adjustSize();
	mIndex.adjustHeight();
	mValue.adjustHeight();

	/*
	 * Initialize the list
	 */
	initProfileList();

	mCurrentIndex = 0;	
	mCurrentName = "";
}

PaletteCfg::~PaletteCfg ()
{
}

void PaletteCfg::show ( )
{
	t_config *p_config = config_get_ptr();

	std::string profile_name;

	mSettingsDirty = false;

	if ( p_config->system_type == NTSC )
		profile_name = g_video.ntsc_palette;
	else
		profile_name = g_video.pal_palette;

	if ( getParent () )
		getParent()->moveToTop (this);

	changeGroup ( profile_name );
	setVisible ( true );
}

void PaletteCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void PaletteCfg::initProfileList ( )
{
	struct generic_node *nodeptr = 0;

	mProfileList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Palette", &nodeptr );
	mProfileList.setList ( nodeptr );
}

void PaletteCfg::saveSettings ( std::string profile_name )
{
	t_config *p_config = config_get_ptr();
	int index,value;

	index = atoi ( mIndex.getText().c_str() );
	value = strtol ( mValue.getText().c_str(), NULL, 16 );

	if ( index < 0 )
		index = 0;
	if ( index > 255 )
		index = 255;
	mPal[index] = value;

	if ( !save_palette ( (char *)profile_name.c_str(), mPal ) ) {

		initProfileList();
		mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()));
	}
}

void PaletteCfg::changeGroup ( std::string profile_name )
{
	char tmp_string[10];
	t_config *p_config = config_get_ptr();

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
	if ( recall_palette ( profile_name.c_str(), mPal ) ) {

		mGuiMain->showError(ERR_PROFILE_LOAD);

		profile_name = "";
	}

	mCurrentName = profile_name;
	mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

	/*
	 * Set values on the GUI
	 */
	if ( mCurrentIndex < 0 )
		mCurrentIndex = 0;
	if ( mCurrentIndex > 255 )
		mCurrentIndex = 255;
	sprintf ( tmp_string, "%d", mCurrentIndex );
	mIndex.setText( tmp_string );

	sprintf ( tmp_string, "%#.6x", mPal[mCurrentIndex] );
	mValue.setText( tmp_string );
}

void PaletteCfg::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();
	std::string new_name;
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
	else if ( actionEvent.getId() == "mIndex" ) {
		mPal[mCurrentIndex] = strtol ( mValue.getText().c_str(), NULL, 16 );

		mCurrentIndex = atoi ( mIndex.getText().c_str() );
		if ( mCurrentIndex < 0 )
			mCurrentIndex = 0;
		if ( mCurrentIndex > 255 )
			mCurrentIndex = 255;

		sprintf ( tmp_string, "%d", mCurrentIndex );
		mIndex.setText ( tmp_string );

		sprintf ( tmp_string, "%#.6x", mPal[mCurrentIndex] );
		mValue.setText ( tmp_string );
	}
	else if ( actionEvent.getId() == "mProfile" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mCurrentName );
		}
		changeGroup ( mProfileList.getLabelFromIndex(mProfile.getSelected()) );
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
				sprintf ( tmp_string, "UPDATE Palette SET Name='%s' WHERE Name='%s'", 
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
		mGuiMain->getProfilePopup()->show( "Choose Profile or Create New", "SELECT Name FROM Palette", 
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
}

void PaletteCfg::mousePressed(gcn::MouseEvent& mouseEvent)
{
	bool comp_has_mouse = false;

	if ( mouseEvent.getSource() == &mDescriptionBox || mouseEvent.getSource() == &mValue )
		comp_has_mouse = true;

    if ((comp_has_mouse == true) && mouseEvent.getButton() == gcn::MouseInput::LEFT)
    {
		mSettingsDirty = true;
    }

	gcn::Window::mousePressed ( mouseEvent );
}
