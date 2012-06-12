/******************************************************************************
*
* FILENAME: bioscfg.hpp
*
* DESCRIPTION:  This class will show the BiosCfg adjustment
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/22/06  bberlin      Creation
******************************************************************************/
#ifndef bioscfg_hpp
#define bioscfg_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"

class BiosCfg : public gcn::GuiWindow,
                public GuiObject,
                public gcn::ActionListener
{
	public:
		BiosCfg( GuiMain *gMain );
		~BiosCfg();

		void show (); 
		void hide ();

		void action(const gcn::ActionEvent &actionEvent);

	private:
		gcn::Label m5200Label;
		gcn::TextField m5200File;
		gcn::Button m5200Browse;

		gcn::Label m800Label;
		gcn::TextField m800File;
		gcn::Button m800Browse;

		gcn::Label mXLLabel;
		gcn::TextField mXLFile;
		gcn::Button mXLBrowse;

		gcn::Label mBasicLabel;
		gcn::TextField mBasicFile;
		gcn::Button mBasicBrowse;

		gcn::Button mOk;
		gcn::Button mCancel;
};

#endif
