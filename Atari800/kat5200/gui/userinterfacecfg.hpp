/******************************************************************************
*
* FILENAME: userinterfacecfg.hpp
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
#ifndef userinterfacecfg_hpp
#define userinterfacecfg_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "guiinputbutton.hpp"
#include "window.hpp"
#include "inputautolistener.hpp"
#include "decoration.hpp"
#include "guiobject.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/ui.h"
}

class UserInterfaceCfg : public gcn::GuiWindow,
                         public GuiObject,
                         public gcn::ActionListener
{
	public:
		UserInterfaceCfg( GuiMain *gMain );
		~UserInterfaceCfg();

		void setupInputButton (GuiInputButton *btn, std::string caption, 
		                         std::string grp, t_atari_common *ctrl );
		void show (); 
		void hide ();
		void initProfileList ( );
		void initGroupControls ();
		void saveSettings ( std::string profile_name );
		void changeGroup ( std::string profile_name );
		void getInput( t_atari_common *ctrl );
		void setInput( t_atari_common *ctrl );
		void action(const gcn::ActionEvent &actionEvent);

		void mousePressed(gcn::MouseEvent &mouseEvent);

	private:
		gcn::DropDown mProfile;
		GenericList mProfileList;
		gcn::CheckBox mDefault;
		gcn::Button mRename;
		gcn::Button mSaveAs;
		gcn::TextField mDescriptionBox;
		gcn::Label mDescriptionLabel;
		Decoration mProfileDec;

		gcn::Button mOk;
		gcn::Button mCancel;

		Decoration mFontDec;
		gcn::TextField mFontFile;
		gcn::Button mFontBrowse;

		Decoration mStartDec;
		gcn::CheckBox mStartGui;
		gcn::CheckBox mStartLauncher;

		Decoration mPCDec;
		gcn::Label mDeviceLabel;
		gcn::DropDown mDevice;
		GenericList mDeviceList;
		gcn::Label mDeviceNumLabel;
		gcn::TextField mDeviceNum;
		gcn::Label mPartTypeLabel;
		gcn::DropDown mPartType;
		GenericList mPartTypeList;
		gcn::Label mPartNumLabel;
		gcn::TextField mPartNum;
		gcn::Label mDirectionLabel;
		gcn::DropDown mDirection;
		GenericList mDirectionList;
		gcn::Button mAutoDetect;
		InputAutoListener mAutoListener;

		Decoration mUIDec;
		GuiInputButton mExit;
		GuiInputButton mBack;
		GuiInputButton mFullScreen;
		GuiInputButton mScreenShot;

		Decoration mStateDec;
		Decoration mStateDec2;
		GuiInputButton mLoadState[9];
		GuiInputButton mSaveState[9];

		t_atari_common *mCurrentCtrl;

		t_config mConfig;
		t_ui mUI;
		bool mSettingsDirty;
		int mAutoDetectAct;
};

#endif
