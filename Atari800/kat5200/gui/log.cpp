/******************************************************************************
*
* FILENAME: log.cpp
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
#include "log.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/logger.h"
}

Log::Log ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );
	setCaption ( "Log" );

	/*
	 * Setup all of our widgets
	 */
	mLogScroller.setContent(&mLogBox);
	mOk.setCaption("Ok");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mOk.addActionListener ( this );
	mOk.setActionEventId ( "mOk" );

	/*
	 * Size and place our widgets
	 */
	add ( &mLogScroller );
	add ( &mOk );

	setDimension ( mGuiMain->getRectangle(2,2,74,26) );

	mLogScroller.setDimension ( mGuiMain->getRectangle(1,1,72,21) );
	mLogBox.setDimension ( mGuiMain->getRectangle(0,0,72,21) );
	mOk.setDimension ( mGuiMain->getRectangle(34,22.8,3,1) );

	mOk.adjustSize();
}

Log::~Log ()
{

}

void Log::show ( )
{
	char *g_messages = 0;

	mLogBox.setText ( g_messages = logger_get_buffer () );

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void Log::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void Log::action(const gcn::ActionEvent &actionEvent)
{
	if ( actionEvent.getId() == "mOk" ) {
		hide();
	}
}
