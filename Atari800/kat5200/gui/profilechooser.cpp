/******************************************************************************
*
* FILENAME: genericmessage.cpp
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
#include "profilechooser.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/db_if.h"
}

ProfileChooser::ProfileChooser ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );

	mOk.setCaption ( "Ok" );
	mCancel.setCaption ( "Cancel" );

	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mCreateNew.addActionListener ( this );

	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );
	mCreateNew.setActionEventId ( "mCreateNew" );

	mProfileLabel.setCaption ( "Profile" );
	mNewProfileLabel.setCaption ( "Profile" );
	mCreateNew.setCaption ( "Create New" );

	mProfile.setListModel ( &mProfileList );

	/*
	 * Size and place our widgets
	 */
	add ( &mProfileLabel );
	add ( &mNewProfileLabel );
	add ( &mProfile );
	add ( &mNewProfile );
	add ( &mCreateNew );
	add ( &mOk );
	add ( &mCancel );

	setCaption ( "Choose Profile or Create New" );
	setDimension ( mGuiMain->getRectangle(10,8,60,10) );
	mProfileLabel.setDimension ( mGuiMain->getRectangle(2,1,8,1.2) );
	mProfile.setDimension ( mGuiMain->getRectangle(10,1,45,1.2) );
	mCreateNew.setDimension ( mGuiMain->getRectangle(2,3,10,1.2) );
	mNewProfileLabel.setDimension ( mGuiMain->getRectangle(2,5,8,1.2) );
	mNewProfile.setDimension ( mGuiMain->getRectangle(10,5,45,1.2) );
	mOk.setDimension ( mGuiMain->getRectangle(21,7,4,1.2) );
	mCancel.setDimension ( mGuiMain->getRectangle(31,7,4,1.2) );

	mProfileLabel.adjustSize();
	mProfile.adjustHeight();
	mCreateNew.adjustSize();
	mNewProfile.adjustHeight();
	mOk.adjustSize();
	mCancel.adjustSize();
}

ProfileChooser::~ProfileChooser ()
{

}

void ProfileChooser::show ( std::string title, std::string sql, std::string current_message, bool show_new )
{
	struct generic_node *nodeptr = 0;

	mCancelPressed = false;

	mProfileList.deleteAll();
	db_if_get_profile_names ( sql.c_str(), &nodeptr );
	mProfileList.setList ( nodeptr );

	mProfile.setSelected ( mProfileList.getIndexFromLabel(current_message.c_str()) );

	mCreateNew.setSelected ( false );
	mNewProfile.setText ( "" );

	mCreateNew.setVisible ( show_new );
	mNewProfile.setVisible ( false );
	mNewProfileLabel.setVisible ( false );

	requestModalFocus ();
	setCaption ( title );
	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void ProfileChooser::hide ( )
{
	releaseModalFocus ();
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void ProfileChooser::deleteActions ( )
{
	mActionListeners.clear();
}

bool ProfileChooser::getCancelPressed ( )
{
	return mCancelPressed;
}

std::string ProfileChooser::getProfile()
{
	if ( mCreateNew.isSelected() == true )
		return mNewProfile.getText();
	else
		return mProfileList.getLabelFromIndex(mProfile.getSelected());
}

void ProfileChooser::action(const gcn::ActionEvent &actionEvent)
{
	std::string tmp_string;
	int buttons = 0;

	/*
	 * Check which button was pressed
	 */
	if ( actionEvent.getId() == "mOk" ) {
		hide ();
		mCancelPressed = false;
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
		mCancelPressed = true;
	}
	else if ( actionEvent.getId() == "mCreateNew" ) {
		mNewProfile.setVisible ( mCreateNew.isSelected() );
		mNewProfileLabel.setVisible ( mCreateNew.isSelected() );
		return;
	}

	distributeActionEvent();

} /* end action */
