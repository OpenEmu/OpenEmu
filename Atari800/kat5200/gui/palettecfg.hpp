/******************************************************************************
*
* FILENAME: palettecfg.hpp
*
* DESCRIPTION:  This class will show the palette configuration
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/24/06  bberlin      Creation
******************************************************************************/
#ifndef palettecfg_hpp
#define palettecfg_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"
#include "genericlist.hpp"
#include "decoration.hpp"

class PaletteCfg : public gcn::GuiWindow,
                   public GuiObject,
                   public gcn::ActionListener
{
	public:
		PaletteCfg( GuiMain *gMain );
		~PaletteCfg();

		void show (); 
		void hide ();

		void initProfileList ();
		void initGroupControls ();
		void changeGroup ( std::string profile_name );
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

		Decoration mPalDec;
		gcn::Label mIndexLabel;
		gcn::TextField mIndex;
		gcn::Label mValueLabel;
		gcn::TextField mValue;

		std::string mCurrentName;
		bool mSettingsDirty;
		int mCurrentIndex;
		int mPal[256];
};

#endif
