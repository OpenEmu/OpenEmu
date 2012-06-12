/******************************************************************************
*
* FILENAME: profilechooser.hpp
*
* DESCRIPTION:  This class will show a pop-up with profile list
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/13/07  bberlin      Creation
******************************************************************************/
#ifndef profilechooser_hpp
#define profilechooser_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"
#include "genericlist.hpp"

class ProfileChooser : public gcn::GuiWindow,
                       public GuiObject,
                       public gcn::ActionListener
{
	public:
		ProfileChooser( GuiMain *gMain );
		~ProfileChooser();

		void show ( std::string title, std::string sql, std::string current_profile, bool show_new );
		void hide ( );
		int getButton ( );
		void deleteActions ( );
		bool getCancelPressed ( );
		std::string getProfile();
		void action(const gcn::ActionEvent &actionEvent);

	private:
		gcn::Button mOk;
		gcn::Button mCancel;
		gcn::Label mProfileLabel;
		gcn::Label mNewProfileLabel;
		gcn::DropDown mProfile;
		GenericList mProfileList;
		gcn::TextField mNewProfile;
		gcn::CheckBox mCreateNew;

		bool mCancelPressed;
};

#endif
