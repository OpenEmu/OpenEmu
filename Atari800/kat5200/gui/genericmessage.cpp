/******************************************************************************
*
* FILENAME: genericmessage.cpp
*
* DESCRIPTION:  This class will show a message pop-up with optional buttons
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/06/06  bberlin      Creation
******************************************************************************/
#include "genericmessage.hpp"
#include "guimain.hpp"

GenericMessage::GenericMessage ( GuiMain *gMain ) : GuiObject(gMain)
{
	int i;

	setVisible ( false );

	/*
	 * Create Callbacks and assign
	 */
	for ( i = 0; i < 3; ++i )
		mButton[i].addActionListener ( this );

	mButton[0].setActionEventId ( "Event0" );
	mButton[1].setActionEventId ( "Event1" );
	mButton[2].setActionEventId ( "Event2" );

	/*
	 * Size and place our widgets
	 */
	add ( &mMessage );
	add ( &mInput );
	for ( i = 0; i < 3; ++i )
		add ( &mButton[i] );

	setCaption ( "User Message" );
	setDimension ( mGuiMain->getRectangle(10,8,60,9) );
	mMessage.setDimension ( mGuiMain->getRectangle(2,1,58,1.2) );
	mInput.setDimension ( mGuiMain->getRectangle(2,2.5,48,1.2) );
	mButton[0].setDimension ( mGuiMain->getRectangle(15,6,3,1) );
	mButton[1].setDimension ( mGuiMain->getRectangle(30,6,3,1) );
	mButton[2].setDimension ( mGuiMain->getRectangle(45,6,3,1) );
}

GenericMessage::~GenericMessage ()
{
}

void GenericMessage::show ( std::string message, std::string button0, bool show0, 
		                                         std::string button1, bool show1,
		                                         std::string button2, bool show2, 
												 bool showinput )
{
	int diff;

	mMessage.setCaption ( message );
	mMessage.setVisible ( true );
	diff = 60 - (int)message.size();
	if ( diff < 0 )
		mMessage.setDimension ( mGuiMain->getRectangle(0,1,60,1.2) );
	else
		mMessage.setDimension ( mGuiMain->getRectangle(diff/2,1,58,1.2) );
	mMessage.adjustSize ();

	mInput.setVisible ( showinput );
	mInput.setText("");

	mButton[0].setCaption ( button0 );
	mButton[1].setCaption ( button1 );
	mButton[2].setCaption ( button2 );

	mButton[0].setVisible ( false );
	mButton[1].setVisible ( false );
	mButton[2].setVisible ( false );

	if ( show0 == true ) {
		mButton[0].setVisible ( true );
		mButton[0].adjustSize ();
		mButton[0].setPosition ( mGuiMain->getPosition(18,6).x, mGuiMain->getPosition(20,6).y );
	}

	if ( show1 == true ) {
		mButton[1].setVisible ( true );
		mButton[1].adjustSize ();
		mButton[1].setPosition ( mGuiMain->getPosition(33,6).x, mGuiMain->getPosition(40,6).y );
	}

	if ( show2 == true ) {
		mButton[2].setVisible ( true );
		mButton[2].adjustSize ();
		mButton[2].setPosition ( mGuiMain->getPosition(48,6).x, mGuiMain->getPosition(60,6).y );
	}

	if ( show0 && !show1 && !show2 ) {
		mButton[0].setPosition ( mGuiMain->getPosition(28,6).x, mGuiMain->getPosition(40,6).y );
	}

	requestModalFocus ();
	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void GenericMessage::hide ( )
{
	releaseModalFocus ();
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

int GenericMessage::getButton ( )
{
	return mButtonIndex;
}

void GenericMessage::setButton ( int button )
{
	mButtonIndex = button;
}

void GenericMessage::deleteActions ( )
{
	mActionListeners.clear();
}

void GenericMessage::setInput( std::string input )
{
		mInput.setText( input );
}

std::string GenericMessage::getInput()
{
	return mInput.getText();
}

void GenericMessage::action(const gcn::ActionEvent &actionEvent)
{
	/*
	 * Check which button was pressed
	 */
	if ( actionEvent.getId() == "Event0" )
		setButton ( 0 );
	else if ( actionEvent.getId() == "Event1" )
		setButton ( 1 );
	else if ( actionEvent.getId() == "Event2" )
		setButton ( 2 );

	distributeActionEvent();

	/*
	for ( i = 0; i < 3; ++i ) {
		if ( mButton[i]->isVisible() )
			buttons++;
	}

	if ( buttons < 2 )
		hide();
	*/

} /* end action */
