/******************************************************************************
*
* FILENAME: romlaunchermenu.hpp
*
* DESCRIPTION:  This class will create a menu bar for the launcher window
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   07/31/07  bberlin      Creation
******************************************************************************/
#ifndef romlaunchermenu_hpp
#define romlaunchermenu_hpp

#include <guichan.hpp>
#include "menubar.hpp"
#include "guiobject.hpp"

class RomLauncher;
class RomLauncherMenu : public MenuBar,
                        public GuiObject,
                        public gcn::ActionListener
{
	public:
		RomLauncherMenu( RomLauncher *launcher=0 );
		~RomLauncherMenu();
		void setGroupChecked ( std::string id );
		void setLauncher ( RomLauncher *launcher );
		void action(const gcn::ActionEvent &actionEvent);

	private:
		RomLauncher *romLauncher;
};

#endif
