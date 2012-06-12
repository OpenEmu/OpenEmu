/******************************************************************************
*
* FILENAME: soundcfg.cpp
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
#include "soundcfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/sound.h"
	#include "../interface/db_if.h"

	extern t_atari_sound g_sound;
}

SoundCfg::SoundCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	t_config *p_config = config_get_ptr();

	setVisible ( false );
	setCaption ("Sound Settings");

	/*
	 * Setup all of our widgets
	 */
	mProfileDec.setCaption ( "Profile" );
	mDescriptionLabel.setCaption ("Description");
	mRename.setCaption ("Rename");
	mSaveAs.setCaption ("Save As");
	mDefault.setCaption ("Use as Default");
	mProfile.setListModel ( &mProfileList );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	mSoundDec.setCaption ( "" );
	mSoundEnabled.setCaption ("Enable Sound");
	mFilterEnabled.setCaption ("Enable Filter");
	mFreqLabel.setCaption ("Frequency");
	mFreq.setListModel ( &mFreqList );
	mSamplesLabel.setCaption("Samples");
	mSamples.setListModel ( &mSamplesList );

	mBass.setScale (0, 123);
	mTreble.setScale (-200,5);

	mBassLabel.setCaption ("Bass (Hz)");
	mTrebleLabel.setCaption ("Treble (db)");

	mBassValue.setCaption ("0");
	mTrebleValue.setCaption ("0");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mDescriptionBox.addMouseListener ( this );
	mProfile.addActionListener ( this );
	mRename.addActionListener ( this );
	mSaveAs.addActionListener ( this );
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mDefault.addActionListener ( this );
	mSoundEnabled.addActionListener ( this );
	mFilterEnabled.addActionListener ( this );
	mFreq.addActionListener ( this );
	mSamples.addActionListener ( this );

	mRename.setActionEventId ( "mRename" );
	mSaveAs.setActionEventId ( "mSaveAs" );
	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );
	mDefault.setActionEventId ( "mDirty" );
	mSoundEnabled.setActionEventId ( "mDirty" );
	mFilterEnabled.setActionEventId ( "mFilterEnabled" );
	mFreq.setActionEventId ( "mDirty" );
	mSamples.setActionEventId ( "mDirty" );
	mProfile.setActionEventId ( "mProfile" );

	mBass.addActionListener ( this );
	mTreble.addActionListener ( this );

	mBass.setActionEventId ( "mBass" );
	mTreble.setActionEventId ( "mTreble" );

	/*
	 * Size and place our widgets
	 */
	add ( &mProfileDec );
	add ( &mSoundDec );
	add ( &mProfile );
	add ( &mDescriptionLabel );
	add ( &mDescriptionBox );
	add ( &mRename );
	add ( &mSaveAs );

	add ( &mOk );
	add ( &mCancel );

	add ( &mDefault );
	add ( &mSoundEnabled );
	add ( &mFilterEnabled );
	add ( &mFreq );
	add ( &mFreqLabel );
	add ( &mSamples );
	add ( &mSamplesLabel );

	add ( &mBass );
	add ( &mTreble );

	add ( &mBassLabel );
	add ( &mTrebleLabel );

	add ( &mBassValue );
	add ( &mTrebleValue );

	setDimension ( mGuiMain->getRectangle(3,5,73,15) );

	mProfile.setDimension ( mGuiMain->getRectangle(1,1.2,50,4) );
	mDescriptionLabel.setDimension ( mGuiMain->getRectangle(1,2.8,50,4) );
	mDescriptionBox.setDimension ( mGuiMain->getRectangle(1,3.8,50,4) );
	mDefault.setDimension ( mGuiMain->getRectangle(53,3.8,14,1) );
	mRename.setDimension ( mGuiMain->getRectangle(53,1.2,3,1) );
	mSaveAs.setDimension ( mGuiMain->getRectangle(61.5,1.2,3,1) );
	mProfileDec.setDimension ( mGuiMain->getRectangle(0.5,0.0,71,5.5) );

	mOk.setDimension ( mGuiMain->getRectangle(30,12,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(37,12,6,1) );

	mSoundDec.setDimension ( mGuiMain->getRectangle(0.5,5.5,71,6) );
	mSoundEnabled.setDimension ( mGuiMain->getRectangle(5,6.5,6,1) );
	mFilterEnabled.setDimension ( mGuiMain->getRectangle(33,6.5,6,1) );
	mFreqLabel.setDimension ( mGuiMain->getRectangle(5,8.5,12,1) );
	mFreq.setDimension ( mGuiMain->getRectangle(16,8.5,10,1) );
	mSamplesLabel.setDimension ( mGuiMain->getRectangle(5,10,12,1) );
	mSamples.setDimension ( mGuiMain->getRectangle(16,10,10,1) );

	mBassLabel.setDimension ( mGuiMain->getRectangle(33,8.5,4,1) );
	mBass.setDimension ( mGuiMain->getRectangle(44,8.5,14,1) );
	mBassValue.setDimension ( mGuiMain->getRectangle(58.5,8.5,4,1) );
	mTrebleLabel.setDimension ( mGuiMain->getRectangle(33,10,6,1) );
	mTreble.setDimension ( mGuiMain->getRectangle(44,10,14,1) );
	mTrebleValue.setDimension ( mGuiMain->getRectangle(58.5,10,6,1) );

	mProfile.adjustHeight();
	mDescriptionLabel.adjustSize();
	mDescriptionBox.adjustHeight();
	mRename.adjustSize();
	mSaveAs.adjustSize();
	mOk.adjustSize();
	mCancel.adjustSize();

	mDefault.adjustSize();
	mSoundEnabled.adjustSize();
	mFilterEnabled.adjustSize();
	mFreqLabel.adjustSize();
	mSamplesLabel.adjustSize();

	mBassLabel.adjustSize();
	mTrebleLabel.adjustSize();

	mBassValue.adjustSize();
	mTrebleValue.adjustSize();

	/*
	 * Initialize the list
	 */
	initGroupControls();
	initProfileList();

	/*
	 * FIXME: Enable this when we have a sound filter in place
	 */
	mFilterEnabled.setVisible ( false );
}

SoundCfg::~SoundCfg ()
{
}

void SoundCfg::show ( )
{
	t_config *p_config = config_get_ptr();
	std::string profile_name;

	mSettingsDirty = false;

	if ( (p_config->machine_type == media_get_ptr()->machine_type) ||
	     (p_config->machine_type != MACHINE_TYPE_5200 && 
	     media_get_ptr()->machine_type != MACHINE_TYPE_5200) )
		sound_set_profile ( media_get_ptr()->sound_profile );

	mSound = g_sound;

	profile_name = mSound.name;

	if ( getParent () )
		getParent()->moveToTop (this);

	changeGroup ( profile_name );
	setVisible ( true );
}

void SoundCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void SoundCfg::initProfileList ( )
{
	struct generic_node *nodeptr = 0;

	mProfileList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Sound", &nodeptr );
	mProfileList.setList ( nodeptr );
}

void SoundCfg::setSoundCaptions ( )
{
	char tmp_string[100];

	sprintf ( tmp_string, "%+#03.1lf", pow(mBass.getValue(),2) );
	mBassValue.setCaption( tmp_string );
	sprintf ( tmp_string, "%+#03.1lf", mTreble.getValue() );
	mTrebleValue.setCaption( tmp_string );

	mBassValue.adjustSize();
	mTrebleValue.adjustSize();
}

void SoundCfg::setFilterEnabled ( bool enabled )
{
	mBass.setVisible( enabled );
	mTreble.setVisible( enabled );
	mBassValue.setVisible( enabled );
	mTrebleValue.setVisible( enabled );
	mBassLabel.setVisible( enabled );
	mTrebleLabel.setVisible( enabled );
}

void SoundCfg::initGroupControls ( )
{
	mFreqList.deleteAll();
	mFreqList.addElement ( "11025", 11025, 0 );
	mFreqList.addElement ( "15960", 15960, 0 );
	mFreqList.addElement ( "15960", 15960, 0 );
	mFreqList.addElement ( "22050", 22050, 0 );
	mFreqList.addElement ( "33075", 33075, 0 );
	mFreqList.addElement ( "44100", 44100, 0 );

	mSamplesList.deleteAll();
	mSamplesList.addElement ( "512", 512, 0 );
	mSamplesList.addElement ( "1024", 1024, 0 );
	mSamplesList.addElement ( "2048", 2048, 0 );
	mSamplesList.addElement ( "4096", 4096, 0 );
	mSamplesList.addElement ( "8192", 8192, 0 );
}

void SoundCfg::saveSettings ( std::string profile_name )
{
	t_config *p_config = config_get_ptr();
	t_atari_sound *p_sound = &mSound;

	strcpy ( p_sound->description, mDescriptionBox.getText().c_str() );

	if ( mSoundEnabled.isSelected() == true )
		p_sound->on = 1;
	else
		p_sound->on = 0;

	p_sound->freq = mFreqList.getValueFromIndex( mFreq.getSelected() );
	p_sound->samples = mSamplesList.getValueFromIndex( mSamples.getSelected() );

	if ( mFilterEnabled.isSelected() == true )
		p_sound->filter_on = 1;
	else
		p_sound->filter_on = 0;

	p_sound->bass = (int)pow(mBass.getValue(),2);
	p_sound->treble = (int)mTreble.getValue();

	if ( !sound_save_profile ( (char *)profile_name.c_str(), p_sound ) ) {

		strcpy ( p_sound->name, profile_name.c_str() );

		initProfileList();
		mProfile.setSelected ( mProfileList.getIndexFromLabel(p_sound->name));

		if ( mDefault.isSelected() == true ) {
			strcpy ( p_config->default_sound_profile, p_sound->name );
			config_save();
		}
	}
}

void SoundCfg::changeGroup ( std::string profile_name )
{

	t_config *p_config = config_get_ptr();
	t_atari_sound *p_sound = &mSound;

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
	if ( sound_load_profile ( profile_name.c_str(), p_sound ) ) {

		mGuiMain->showError(ERR_PROFILE_LOAD);

		sound_load_profile ( p_config->default_sound_profile, &mSound );
		profile_name = p_config->default_sound_profile;
	}

	mProfile.setSelected ( mProfileList.getIndexFromLabel(profile_name.c_str()) );

	mDescriptionBox.setText ( p_sound->description );

	/*
	 * Set values on the GUI
	 */
	if ( !strcmp(profile_name.c_str(), p_config->default_sound_profile) )
		mDefault.setSelected ( true );
	else
		mDefault.setSelected ( false );

	if ( p_sound->on )
		mSoundEnabled.setSelected ( true );
	else
		mSoundEnabled.setSelected ( false );

	if ( p_sound->filter_on )
		mFilterEnabled.setSelected ( true );
	else
		mFilterEnabled.setSelected ( false );

	mFreq.setSelected ( mFreqList.getIndexFromValue(p_sound->freq) );
	mSamples.setSelected ( mSamplesList.getIndexFromValue(p_sound->samples) );

	mBass.setValue(sqrt((double)p_sound->bass));
	mTreble.setValue(p_sound->treble);

	setSoundCaptions ( );
	setFilterEnabled ( mFilterEnabled.isSelected() );
}

void SoundCfg::action(const gcn::ActionEvent &actionEvent)
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
	else if ( actionEvent.getId() == "mProfile" ) {
		if ( mSettingsDirty == true ) {
			saveSettings( mSound.name );
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
				sprintf ( tmp_string, "UPDATE Sound SET Name='%s' WHERE Name='%s'", 
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
		mGuiMain->getProfilePopup()->show( "Choose Profile or Create New", "SELECT Name FROM Sound", 
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
	else if ( actionEvent.getId() == "mFilterEnabled" ) {
		mSettingsDirty = true;
		setFilterEnabled ( mFilterEnabled.isSelected() );
	}
	else if ( actionEvent.getId() == "mBass" ) {
		mSettingsDirty = true;
		setSoundCaptions();
	}
	else if ( actionEvent.getId() == "mTreble" ) {
		mSettingsDirty = true;
		setSoundCaptions();
	}
}

void SoundCfg::mousePressed(gcn::MouseEvent& mouseEvent)
{
	bool comp_has_mouse = false;

	if ( mouseEvent.getSource() == &mDescriptionBox )
		comp_has_mouse = true;

    if ((comp_has_mouse == true) && mouseEvent.getButton() == gcn::MouseInput::LEFT)
    {
		mSettingsDirty = true;
    }

	gcn::Window::mousePressed ( mouseEvent );
}
