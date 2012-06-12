/******************************************************************************
*
* FILENAME: soundcfg.hpp
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
#ifndef soundcfg_hpp
#define soundcfg_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"
#include "genericlist.hpp"
#include "decoration.hpp"

extern "C" {
	#include "../interface/sound.h"
}

class SoundCfg : public gcn::GuiWindow,
                 public GuiObject,
                 public gcn::ActionListener
{
	public:
		SoundCfg( GuiMain *gMain );
		~SoundCfg();

		void show (); 
		void hide ();

		void initProfileList ();
		void initGroupControls ();
		void changeGroup ( std::string profile_name );
		void setSoundCaptions ();
		void setFilterEnabled ( bool enabled );
		void saveSettings ( std::string profile_name );

		void action(const gcn::ActionEvent &actionEvent);
		void mousePressed(gcn::MouseEvent& mouseEvent);

	private:
		gcn::DropDown mProfile;
		GenericList mProfileList;
		gcn::Label mDescriptionLabel;
		gcn::TextField mDescriptionBox;
		gcn::Button mRename;
		gcn::Button mSaveAs;
		Decoration mProfileDec;

		gcn::Button mOk;
		gcn::Button mCancel;

		Decoration mSoundDec;
		gcn::CheckBox mDefault;
		gcn::CheckBox mSoundEnabled;
		gcn::CheckBox mFilterEnabled;
		gcn::Label mFreqLabel;
		gcn::DropDown mFreq;
		GenericList mFreqList;
		gcn::Label mSamplesLabel;
		gcn::DropDown mSamples;
		GenericList mSamplesList;

		gcn::Slider mBass;
		gcn::Slider mTreble;

		gcn::Label mBassLabel;
		gcn::Label mTrebleLabel;

		gcn::Label mBassValue;
		gcn::Label mTrebleValue;

		t_atari_sound mSound;

		bool mSettingsDirty;
		int mCurrentGrp;
};

#endif
