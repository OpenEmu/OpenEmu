/******************************************************************************
*
* FILENAME: romlauncheroptions.cpp
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
#include "romlauncheroptions.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
}

RomLauncherOptions::RomLauncherOptions ( GuiMain *gMain ) : GuiObject(gMain)
{
	t_config *p_config = config_get_ptr();

	setVisible ( false );
	setCaption ("Launcher Options");

	/*
	 * Setup all of our widgets
	 */
	mMachineDec.setCaption ( "Media by Machine" );
	mTabbedDec.setCaption ( "Tabbed View Options" );

	mMachine.setListModel (&mMachineList);
	mTabbed.setListModel (&mTabbedList);

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mMachine.addActionListener ( this );
	mTabbed.addActionListener ( this );
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );

	mMachine.setActionEventId ( "mDirty" );
	mTabbed.setActionEventId ( "mDirty" );
	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );

	/*
	 * Size and place our widgets
	 */
	add ( &mMachineDec );
	add ( &mTabbedDec );

	add ( &mMachine );
	add ( &mTabbed );

	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(12,6.5,53,12) );

	mMachine.setDimension ( mGuiMain->getRectangle(1,2.0,32,2) );
	mTabbed.setDimension ( mGuiMain->getRectangle(1,6.0,32,2) );

	mMachineDec.setDimension ( mGuiMain->getRectangle(0.5,0.5,40,3.0) );
	mTabbedDec.setDimension ( mGuiMain->getRectangle(0.5,4.5,40,3.0) );

	mOk.setDimension ( mGuiMain->getRectangle(20,9.0,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(27,9.0,6,1) );

	mMachine.adjustHeight();
	mTabbed.adjustHeight();

	mOk.adjustSize();
	mCancel.adjustSize();

	mMachineList.deleteAll();
	mMachineList.addElement ( "Show all Media", 1, 0 );
	mMachineList.addElement ( "Show Media for Current Machine", 0, 0 );

	mTabbedList.deleteAll();
	mTabbedList.addElement ( "No Tab", 0, 0 );
	mTabbedList.addElement ( "Tabbed by TOSEC designation", 1, 0 );
	mTabbedList.addElement ( "Tabbed by Media type", 2, 0 );
	mTabbedList.addElement ( "Tabbed Alphabetically", 3, 0 );
}

RomLauncherOptions::~RomLauncherOptions ()
{
}

void RomLauncherOptions::show ( )
{
	t_config *p_config = config_get_ptr();

	mSettingsDirty = false;

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);

	/*
	 * Set Selected to Current
	 */
	mMachine.setSelected(mMachineList.getIndexFromValue(p_config->launcher_show_all));
	mTabbed.setSelected(mTabbedList.getIndexFromValue(p_config->launcher_tabbed_view));
}

void RomLauncherOptions::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void RomLauncherOptions::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();

	if ( actionEvent.getId() == "mOk" ) {
		if ( mSettingsDirty == true )
		{
			p_config->launcher_show_all = mMachineList.getValueFromIndex(mMachine.getSelected());
			p_config->launcher_tabbed_view = mTabbedList.getValueFromIndex(mTabbed.getSelected());
			config_save();
			mGuiMain->updateLauncherList ();
		}
		hide();
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}
	else if ( actionEvent.getId() == "mDirty" ) {
		mSettingsDirty = true;
	}
}
