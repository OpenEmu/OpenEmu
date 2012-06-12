/******************************************************************************
*
* FILENAME: mediacreate.hpp
*
* DESCRIPTION:  This class will show the Media Creation window
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   06/25/08  bberlin      Creation
******************************************************************************/
#ifndef mediacreate_hpp
#define mediacreate_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"
#include "guiinputbutton.hpp"
#include "genericlist.hpp"

class MediaCreate : public gcn::GuiWindow,
                    public GuiObject,
                    public gcn::ActionListener
{
	public:
		MediaCreate( GuiMain *gMain );
		~MediaCreate();

		void show (); 
		void hide ();

		void action(const gcn::ActionEvent &actionEvent);

	private:
		gcn::Label mCreateCasLabel;
		gcn::Label mCreateCasDescLabel;
		gcn::TextField mCreateCasDesc;
		gcn::Button mCreateCasBrowse;

		gcn::Label mCreateDiskLabel;
		gcn::Label mCreateDiskTypeLabel;
		gcn::DropDown mCreateDiskType;
		GenericList mCreateDiskList;
		gcn::Label mCreateDiskSectorsLabel;
		gcn::TextField mCreateDiskSectors;
		gcn::Button mCreateDiskBrowse;

		gcn::Button mOk;
		gcn::Button mCancel;
};

#endif
