/******************************************************************************
*
* FILENAME: katmenu.hpp
*
* DESCRIPTION:  This class will create a menu bar for the gui
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/29/06  bberlin      Creation
******************************************************************************/
#ifndef katmenu_hpp
#define katmenu_hpp

extern "C" {
	#include "../interface/kconfig.h"
}

#include <guichan.hpp>
#include "guiobject.hpp"
#include "menubar.hpp"

class KatMenuListener : public gcn::ActionListener,
                        public GuiObject
{
public:
	void action(const gcn::ActionEvent &actionEvent);
};

class KatMenu : public MenuBar,
                public GuiObject,
                public gcn::ActionListener
{
	public:
		KatMenu( GuiMain *gMain );
		~KatMenu();
		void checkSaveStateExistance();
		void setMachine(e_machine_type machine);
		void setRam(e_machine_type machine, int ram);
		void action(const gcn::ActionEvent &actionEvent);

	private:
		KatMenuListener mMenuListener;
};

#endif
