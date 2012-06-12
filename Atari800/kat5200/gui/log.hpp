/******************************************************************************
*
* FILENAME: log.hpp
*
* DESCRIPTION:  This class will show the Log
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   11/30/06  bberlin      Creation
******************************************************************************/
#ifndef log_hpp
#define log_hpp

#include <guichan.hpp>
#include "guiobject.hpp"
#include "window.hpp"

class Log : public gcn::GuiWindow,
            public GuiObject,
            public gcn::ActionListener
{
	public:
		Log( GuiMain *gMain );
		~Log();

		void show (); 
		void hide ();

		void action(const gcn::ActionEvent &actionEvent);

	private:
		gcn::ScrollArea mLogScroller;
		gcn::TextBox mLogBox;

		gcn::Button mOk;
};

#endif
