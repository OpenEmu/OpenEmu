/******************************************************************************
*
* FILENAME: convert.hpp
*
* DESCRIPTION:  This class will show the Convert adjustment
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/24/06  bberlin      Creation
******************************************************************************/
#ifndef convert_hpp
#define convert_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"
#include "genericlist.hpp"

class Convert : public gcn::GuiWindow,
                public GuiObject,
                public gcn::ActionListener
{
	public:
		Convert( GuiMain *gMain );
		~Convert();

		void show ( int type ); 
		void hide ();

		void action(const gcn::ActionEvent &actionEvent);

        enum
        {
            CONVERT_KAT5200 = 0,
            CONVERT_A800,
            CONVERT_DIR
        };
	private:
		gcn::Label mLabel;
		gcn::Label mLabel2;
		gcn::TextField mFile;
		gcn::TextField mFile2;
		gcn::DropDown mName;
		GenericList mNameList;
		gcn::Label mNameLabel;
		gcn::Button mBrowse;
		gcn::CheckBox mOption;
		gcn::CheckBox mOption2;

		gcn::Button mOk;
		gcn::Button mCancel;
		int Type;
};

#endif
