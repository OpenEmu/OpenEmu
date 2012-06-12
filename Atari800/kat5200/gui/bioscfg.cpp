/******************************************************************************
*
* FILENAME: bioscfg.cpp
*
* DESCRIPTION:  This class will show the BiosCfg
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/22/06  bberlin      Creation
******************************************************************************/
#include "bioscfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
}

BiosCfg::BiosCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );
	setCaption ( "BIOS Files" );

	/*
	 * Init Captions
	 */
	m5200Label.setCaption ( "Atari 5200 BIOS" );
	m5200Browse.setCaption ( "Browse" );

	m800Label.setCaption ( "Atari 800 BIOS" );
	m800Browse.setCaption ( "Browse" );

	mXLLabel.setCaption ( "Atari XL/XE BIOS" );
	mXLBrowse.setCaption ( "Browse" );

	mBasicLabel.setCaption ( "Atari BASIC Cart" );
	mBasicBrowse.setCaption ( "Browse" );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mOk.addActionListener ( this );
	mOk.setActionEventId ( "mOk" );

	mCancel.addActionListener ( this );
	mCancel.setActionEventId ( "mCancel" );

	m5200Browse.addActionListener ( this );
	m5200Browse.setActionEventId ( "m5200Browse" );

	m800Browse.addActionListener ( this );
	m800Browse.setActionEventId ( "m800Browse" );

	mXLBrowse.addActionListener ( this );
	mXLBrowse.setActionEventId ( "mXLBrowse" );

	mBasicBrowse.addActionListener ( this );
	mBasicBrowse.setActionEventId ( "mBasicBrowse" );

	m5200File.addMouseListener ( this );
	m800File.addMouseListener ( this );
	mXLFile.addMouseListener ( this );
	mBasicFile.addMouseListener ( this );

	/*
	 * Size and place our widgets
	 */
	add ( &m5200Label );
	add ( &m5200File );
	add ( &m5200Browse );

	add ( &m800Label );
	add ( &m800File );
	add ( &m800Browse );

	add ( &mXLLabel );
	add ( &mXLFile );
	add ( &mXLBrowse );

	add ( &mBasicLabel );
	add ( &mBasicFile );
	add ( &mBasicBrowse );

	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(2,8,76,12) );

	m5200Label.setDimension ( mGuiMain->getRectangle(1,1,30,1) );
	m5200File.setDimension ( mGuiMain->getRectangle(16,1,50,1) );
	m5200Browse.setDimension ( mGuiMain->getRectangle(67,1,6,1) );

	m800Label.setDimension ( mGuiMain->getRectangle(1,3,30,1) );
	m800File.setDimension ( mGuiMain->getRectangle(16,3,50,1) );
	m800Browse.setDimension ( mGuiMain->getRectangle(67,3,6,1) );

	mXLLabel.setDimension ( mGuiMain->getRectangle(1,5,30,1) );
	mXLFile.setDimension ( mGuiMain->getRectangle(16,5,50,1) );
	mXLBrowse.setDimension ( mGuiMain->getRectangle(67,5,6,1) );

	mBasicLabel.setDimension ( mGuiMain->getRectangle(1,7,30,1) );
	mBasicFile.setDimension ( mGuiMain->getRectangle(16,7,50,1) );
	mBasicBrowse.setDimension ( mGuiMain->getRectangle(67,7,6,1) );

	mOk.setDimension ( mGuiMain->getRectangle(32,9,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(37,9,3,1) );

	m5200Label.adjustSize();
	m800Label.adjustSize();
	mXLLabel.adjustSize();
	mBasicLabel.adjustSize();
	m5200Browse.adjustSize();
	m800Browse.adjustSize();
	mXLBrowse.adjustSize();
	mBasicBrowse.adjustSize();
	m5200File.adjustHeight();
	m800File.adjustHeight();
	mXLFile.adjustHeight();
	mBasicFile.adjustHeight();

	mOk.adjustSize();
	mCancel.adjustSize();
}

BiosCfg::~BiosCfg ()
{

}

void BiosCfg::show ( )
{
	t_config *p_config = config_get_ptr();

	util_set_file_to_program_dir ( p_config->os_file_5200 );
	util_set_file_to_program_dir ( p_config->os_file_800 );
	util_set_file_to_program_dir ( p_config->os_file_xl );
	util_set_file_to_program_dir ( p_config->os_file_basic );

	m5200File.setText( p_config->os_file_5200 );
	m800File.setText( p_config->os_file_800 );
	mXLFile.setText( p_config->os_file_xl );
	mBasicFile.setText( p_config->os_file_basic );

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void BiosCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void BiosCfg::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();

	if ( actionEvent.getId() == "mOk" ) {
		strcpy ( p_config->os_file_5200, m5200File.getText().c_str() );
		strcpy ( p_config->os_file_800, m800File.getText().c_str() );
		strcpy ( p_config->os_file_xl, mXLFile.getText().c_str() );
		strcpy ( p_config->os_file_basic, mBasicFile.getText().c_str() );
		config_save();
		hide();
	}

	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}

	else if ( actionEvent.getId() == "m5200Browse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "File5200" );
		mGuiMain->getFileBrowser()->show("Load 5200 BIOS", p_config->os_file_5200, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "m800Browse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "File800" );
		mGuiMain->getFileBrowser()->show("Load 800 BIOS", p_config->os_file_800, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "mXLBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FileXL" );
		mGuiMain->getFileBrowser()->show("Load XL/XE BIOS", p_config->os_file_xl, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "mBasicBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FileBasic" );
		mGuiMain->getFileBrowser()->show("Load BASIC", p_config->os_file_basic, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "File5200" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			m5200File.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
		}
	}
	else if ( actionEvent.getId() == "File800" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			m800File.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
		}
	}
	else if ( actionEvent.getId() == "FileXL" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mXLFile.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
		}
	}
	else if ( actionEvent.getId() == "FileBasic" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mBasicFile.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
		}
	}
}
