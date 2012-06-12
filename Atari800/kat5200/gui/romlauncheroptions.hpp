/******************************************************************************
*
* FILENAME: romlauncheroptions.hpp
*
* DESCRIPTION:  This class will show the configuration romlauncheroptions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   01/12/08  bberlin      Creation
******************************************************************************/
#ifndef romlauncheroptions_hpp
#define romlauncheroptions_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "window.hpp"
#include "inputautolistener.hpp"
#include "decoration.hpp"
#include "guiobject.hpp"

class RomLauncherOptions : public gcn::GuiWindow,
               public GuiObject,
               public gcn::ActionListener
{
	public:
		RomLauncherOptions( GuiMain *gMain );
		~RomLauncherOptions();

		void show (); 
		void hide ();

		void action(const gcn::ActionEvent &actionEvent);

	private:
		Decoration mMachineDec;
		GenericList mMachineList;
		gcn::DropDown mMachine;

		Decoration mTabbedDec;
		GenericList mTabbedList;
		gcn::DropDown mTabbed;

		gcn::Button mOk;
		gcn::Button mCancel;

		bool mSettingsDirty;
};

#endif
