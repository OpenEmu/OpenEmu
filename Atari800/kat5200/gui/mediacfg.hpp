/******************************************************************************
*
* FILENAME: mediacfg.hpp
*
* DESCRIPTION:  This class will show the MediaCfg adjustment
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   01/12/08  bberlin      Creation
******************************************************************************/
#ifndef mediacfg_hpp
#define mediacfg_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"
#include "guiinputbutton.hpp"
#include "genericlist.hpp"

class MediaCfg : public gcn::GuiWindow,
                 public GuiObject,
                 public gcn::ActionListener
{
	public:
		MediaCfg( GuiMain *gMain );
		~MediaCfg();

		void show (); 
		void hide ();

		void setMachine ( );
		void action(const gcn::ActionEvent &actionEvent);

	private:
		gcn::Label mCartLabel;
		gcn::TextField mCartFile;
		gcn::Button mCartBrowse;
		gcn::Button mCartClear;

		gcn::Label mCasLabel;
		gcn::TextField mCasFile;
		gcn::Button mCasBrowse;
		gcn::Button mCasClear;

		gcn::Label mDiskLabel[8];
		gcn::TextField mDiskFile[8];
		gcn::Button mDiskBrowse[8];
		gcn::Button mDiskClear[8];

		GuiInputButton mRecord;
		GuiInputButton mPlay;
		gcn::Button mRewind;
		gcn::Button mFastFoward;
		gcn::Button mStop;

		gcn::Button mOk;
		gcn::Button mCancel;

		bool mSettingsDirty;
};

#endif
