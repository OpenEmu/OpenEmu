/******************************************************************************
*
* FILENAME: infomessage.cpp
*
* DESCRIPTION:  This class will show a message pop-up with one buttons
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   08/01/07  bberlin      Creation
******************************************************************************/
#include "infomessage.hpp"
#include "guimain.hpp"

InfoMessage::InfoMessage ( GuiMain *gMain ) : GenericMessage(gMain)
{
	setVisible ( false );

	setCaption ( "User Message" );
	setDimension ( mGuiMain->getRectangle(10,8,60,9) );
	mMessage.setDimension ( mGuiMain->getRectangle(2,1,58,1.2) );
	mButton[0].setDimension ( mGuiMain->getRectangle(28,6,3,1) );
	mButton[0].setCaption ( "Ok" );
	mButton[0].adjustSize();

	mMessage.setVisible ( true );
	mButton[0].setVisible ( true );
	mButton[1].setVisible ( false );
	mButton[2].setVisible ( false );
	mInput.setVisible ( false );
}

InfoMessage::~InfoMessage ()
{
}

void InfoMessage::show ( std::string message )
{
	int diff;

	mMessage.setCaption ( message );

	diff = 60 - (int)message.size();
	if ( diff < 0 )
		mMessage.setDimension ( mGuiMain->getRectangle(0,1,60,1.2) );
	else
		mMessage.setDimension ( mGuiMain->getRectangle(diff/2,1,58,1.2) );
	mMessage.adjustSize ();

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
	requestModalFocus ();
}

void InfoMessage::action(const gcn::ActionEvent &actionEvent)
{
	distributeActionEvent();

	hide();

} /* end action */
