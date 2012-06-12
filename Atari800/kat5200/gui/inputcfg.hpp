/******************************************************************************
*
* FILENAME: inputcfg.hpp
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
******************************************************************************/
#ifndef inputcfg_hpp
#define inputcfg_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "tabbedwindow.hpp"
#include "guiinputbutton.hpp"
#include "window.hpp"
#include "inputautolistener.hpp"
#include "decoration.hpp"
#include "guiobject.hpp"

extern "C" {
	#include "../interface/input.h"
	#include "../interface/kconfig.h"
	#include "../interface/util.h"
}

class InputCfg : public gcn::GuiWindow,
                 public GuiObject,
                 public gcn::ActionListener
{
	public:
		InputCfg( GuiMain *gMain );
		~InputCfg();

		void show (); 
		void hide ();

		void initGroupControls ();
		void changeGroup ( std::string profile_name );
		void setMachine ( e_machine_type type );
		void saveSettings ( std::string profile_name );
		void getInput( int player, t_atari_common *ctrl );
		void setInput( int player, t_atari_common *ctrl );
		void action(const gcn::ActionEvent &actionEvent);

		void mousePressed(gcn::MouseEvent& mouseEvent);

	private:
		double setRow( double row );
		double setColumn( double column );
		std::string getKeyTitle(int key);
		void set800KeyDimensions();
		void initProfileList ( );
		void setupInputButton ( GuiInputButton *btn, std::string caption, 
		                         std::string grp, gcn::ActionListener *listener, 
		                         gcn::Window *parent, t_atari_common *ctrl );

		Decoration mProfileDec;
		gcn::DropDown mProfile;
		GenericList mProfileList;
		gcn::TextField mDescriptionBox;
		gcn::Button mRename;
		gcn::Button mSaveAs;

		gcn::Button mOk;
		gcn::Button mCancel;

		gcn::CheckBox mDefault;

		gcn::Label mMouseLabel;
		gcn::DropDown mMouse;
		GenericList mMouseList;

		TabbedWindow mPlayerWindow;
		gcn::Window mKeyboardTab;
		gcn::Window mPlayerTab[4];

		Decoration mSettingsDec[4];
		gcn::Label mSensitivityLabel[4];
		gcn::Slider mSensitivity[4];
		gcn::Label mSensitivityValue[4];
		gcn::Label mDeadzoneLabel[4];
		gcn::Slider mDeadzone[4];
		gcn::Label mDeadzoneValue[4];
		gcn::CheckBox mSimulateAnalog[4];

		Decoration mTypeDec[4];
		gcn::DropDown mPluggedIn[4];
		GenericList mPluggedInList[4];

		Decoration mPCDec[5];
		gcn::Label mDeviceLabel[5];
		gcn::DropDown mDevice[5];
		GenericList mDeviceList[5];
		gcn::Label mDeviceNumLabel[5];
		gcn::TextField mDeviceNum[5];
		gcn::Label mPartTypeLabel[5];
		gcn::DropDown mPartType[5];
		GenericList mPartTypeList[5];
		gcn::Label mPartNumLabel[5];
		gcn::TextField mPartNum[5];
		gcn::Label mDirectionLabel[5];
		gcn::DropDown mDirection[5];
		GenericList mDirectionList[5];
		gcn::Button mAutoDetect[5];
		InputAutoListener mAutoListener;

		Decoration mStickDec[4];
		Decoration mPaddlesDec[4];
		Decoration mKeypadDec[4];
		Decoration mA5200Dec[4];

		Decoration mKeysDec;
		Decoration mControlsDec;
		GuiInputButton mKey[10][4];
		GuiInputButton mStarKey[4];
		GuiInputButton mPoundKey[4];
		GuiInputButton mStartKey[4];
		GuiInputButton mPauseKey[4];
		GuiInputButton mResetKey[4];
		GuiInputButton mTopButton[4];
		GuiInputButton mBottomButton[4];
		GuiInputButton m800Button[4];
		GuiInputButton m800Key[64];
		GuiInputButton m800SelectKey;
		GuiInputButton m800StartKey;
		GuiInputButton m800OptionKey;
		GuiInputButton m800ShiftKey;
		GuiInputButton m800CtrlKey;
		GuiInputButton m800BrkKey;
		GuiInputButton m800ResetKey;

		GuiInputButton mStickDirection[4][4];
		GuiInputButton mPaddleDirection[4][4];
		gcn::Label mPaddle1Label[4];
		gcn::Label mPaddle2Label[4];

		t_atari_common *mCurrentCtrl;

		t_atari_input mInput;
		bool mSettingsDirty;
		bool mKeyboardShowing;
		int mAutoDetectAct;
		int mCurrentGrp;
};

#endif
