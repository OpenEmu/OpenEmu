/******************************************************************************
*
* FILENAME: guiobject.hpp
*
* DESCRIPTION:  Header to describe a class used to access GuiMain.  All our
*                 objects that need access to GuiMain should derive from here
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/12/07  bberlin      Creation
******************************************************************************/
#ifndef guiobject_hpp
#define guiobject_hpp

class GuiMain;

class GuiObject
{
protected:
	GuiMain *mGuiMain;

public:
	GuiObject ( GuiMain *gMain=0 ) {
		mGuiMain = gMain;
	}

	~GuiObject ( ) {
	}

	GuiMain *getGuiMain ( void )
	{
		return mGuiMain;
	}

	void setGuiMain ( GuiMain *gMain )
	{
		mGuiMain = gMain;
	}
};

#endif
