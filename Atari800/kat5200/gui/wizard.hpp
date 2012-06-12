/******************************************************************************
*
* FILENAME: wizard.hpp
*
* DESCRIPTION:  This class will show the configuration wizard
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   07/12/07  bberlin      Creation
******************************************************************************/
#ifndef wizard_hpp
#define wizard_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "window.hpp"
#include "inputautolistener.hpp"
#include "decoration.hpp"
#include "guiobject.hpp"

class Wizard : public gcn::GuiWindow,
               public GuiObject,
               public gcn::ActionListener
{
	public:
		Wizard( GuiMain *gMain );
		~Wizard();

		void show ( std::string message ); 
		void hide ();
		int updateInputList ( );
		int scanImages ( );

		void action(const gcn::ActionEvent &actionEvent);

		gcn::CheckBox m5200Rom;
		gcn::TextField m5200Box;
		gcn::CheckBox m5200ScanSubs;
		gcn::CheckBox m5200Bios;
		gcn::CheckBox m8bitRom;
		gcn::TextField m8bitBox;
		gcn::CheckBox m8bitScanSubs;
		gcn::CheckBox m8bitBios;
		gcn::CheckBox mBiosRom;
		gcn::TextField mBiosBox;
		gcn::CheckBox mBiosScanSubs;

	private:

		gcn::Label mMessage;
		gcn::Label mMessage1;

		gcn::Button m5200Browse;
		gcn::Button m8bitBrowse;
		gcn::Button mBiosBrowse;

		Decoration mSysDec;
		gcn::Label mVidLabel;
		GenericList mVidList;
		gcn::DropDown mVid;
		gcn::CheckBox mFullscreen;
		gcn::Label mZoomLabel;
		GenericList mZoomList;
		gcn::DropDown mZoom;

		Decoration mRomDec;


		Decoration mInputDec;
		GenericList mInputList;
		gcn::DropDown mInput;
		gcn::CheckBox mInputSelect;
		gcn::Label mInputMessage;
		gcn::Button mAutoDetect;
		InputAutoListener mAutoListener;

		gcn::Button mOk;
		gcn::Button mCancel;

		int mAutoDetectAct;
};

#endif
