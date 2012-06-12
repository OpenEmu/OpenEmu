/******************************************************************************
*
* FILENAME: convert.cpp
*
* DESCRIPTION:  This class will show the Convert
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   12/22/06  bberlin      Creation
******************************************************************************/
#include "convert.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/palette.h"
	#include "../interface/video.h"
	#include "../interface/media.h"
	#include "../interface/db_if.h"

	extern t_atari_video g_video;
}

Convert::Convert ( GuiMain *gMain ) : GuiObject(gMain)
{
	setVisible ( false );
	setCaption ( "Convert" );

	/*
	 * Initialize Captions
	 */
	mLabel.setCaption ( "File" );
	mLabel2.setCaption ( "Name" );
	mNameLabel.setCaption ( "Palette" );
	mBrowse.setCaption ( "Browse" );
	mOk.setCaption ("Convert");
	mCancel.setCaption ("Cancel");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mOk.addActionListener ( this );
	mOk.setActionEventId ( "mOk" );

	mCancel.addActionListener ( this );
	mCancel.setActionEventId ( "mCancel" );

	mBrowse.addActionListener ( this );
	mBrowse.setActionEventId ( "mBrowse" );

	mName.setListModel ( &mNameList );

	/*
	 * Size and place our widgets
	 */
	add ( &mLabel );
	add ( &mLabel2 );
	add ( &mNameLabel );
	add ( &mFile );
	add ( &mFile2 );
	add ( &mName );
	add ( &mBrowse );
	add ( &mOption );
	add ( &mOption2 );

	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(5,9,70,11.5) );

	mLabel.setDimension ( mGuiMain->getRectangle(1,1,30,1) );
	mLabel2.setDimension ( mGuiMain->getRectangle(1,3,30,1) );
	mFile.setDimension ( mGuiMain->getRectangle(7,1,52,1) );
	mFile2.setDimension ( mGuiMain->getRectangle(7,3,52,1) );
	mBrowse.setDimension ( mGuiMain->getRectangle(61,1,6,1) );
	mOption.setDimension ( mGuiMain->getRectangle(7,5,50,1) );
	mOption2.setDimension ( mGuiMain->getRectangle(7,6.3,50,1) );
	mNameLabel.setDimension ( mGuiMain->getRectangle(1,1,30,1) );
	mName.setDimension ( mGuiMain->getRectangle(8,1,51,1) );

	mOk.setDimension ( mGuiMain->getRectangle(24,8,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(37,8,3,1) );

	mLabel.adjustSize();
	mLabel2.adjustSize();
	mNameLabel.adjustSize();
	mBrowse.adjustSize();
	mFile.adjustHeight();
	mFile2.adjustHeight();
	mName.adjustHeight();

	mOk.adjustSize();
	mCancel.adjustSize();
}

Convert::~Convert ()
{

}

void Convert::show ( int type )
{
	Type = type;
	t_config *p_config = config_get_ptr();
	struct generic_node *nodeptr = 0;

	mNameList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Palette", &nodeptr );
	mNameList.setList ( nodeptr );

	switch ( type )
	{
		case CONVERT_KAT5200:
			setCaption ( "Export kat5200 Palette" );
			if ( p_config->system_type == PAL )
				mName.setSelected( mNameList.getIndexFromLabel(g_video.pal_palette) );
			else
				mName.setSelected( mNameList.getIndexFromLabel(g_video.ntsc_palette) );
			mBrowse.setVisible ( false );
			mLabel.setVisible( false );
			mLabel2.setVisible( false );
			mFile.setVisible( false );
			mFile2.setVisible( false );
			mName.setVisible ( true );
			mNameLabel.setVisible ( true );
			mOption.setVisible ( false );
			mOption2.setVisible ( false );
			mOk.setCaption ( "Export" );
			mOk.adjustSize();
		break;
		case CONVERT_A800:
			setCaption ( "Import Atari800 Palette" );
			mBrowse.setVisible ( true );
			mFile.setText( "" );
			mFile2.setText( "" );
			mFile.setVisible( true );
			mFile2.setVisible( true );
			mLabel2.setVisible( true );
			mName.setVisible ( false );
			mNameLabel.setVisible ( false );
			mOption.setVisible ( false );
			mOption2.setVisible ( false );
			mOk.setCaption ( "Import" );
			mOk.adjustSize();
		break;
		case CONVERT_DIR:
			setCaption ( "Choose CartDB file to create or update" );
			mBrowse.setVisible ( true );
			mFile.setText( "" );
			mFile2.setVisible( false );
			mLabel2.setVisible( false );
			mName.setVisible ( false );
			mName.setVisible ( false );
			mNameLabel.setVisible ( false );
			mOption.setVisible ( true );
			mOption.setSelected ( false );
			mOption.setCaption ( "Overwrite Existing File? (Otherwise will update)" );
			mOption2.setVisible ( true );
			mOption2.setSelected ( false );
			mOption2.setCaption ( "Scan subdirectories as well?" );
			mOk.setCaption ( "Scan" );
			mOk.adjustSize();
		break;
	}

	mOption.setDimension ( mGuiMain->getRectangle(7,3,50,1) );
	mOption2.setDimension ( mGuiMain->getRectangle(7,4.3,50,1) );

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void Convert::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void Convert::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();

	if ( actionEvent.getId() == "mOk" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FileSave" );
		if ( Type == CONVERT_DIR )
			mGuiMain->getFileBrowser()->show("Navigate to Directory to Scan and Press Select",
			                                p_config->rom_file, FileBrowser::DIR_ONLY);
		else if ( Type == CONVERT_A800 )
		{
			if ( convert_palette ( (char *)mFile.getText().c_str(), 
			                  (char *)mFile2.getText().c_str() ) )
			{
				mGuiMain->getMessagePopup()->deleteActions();
				mGuiMain->getMessagePopup()->show ( "Conversion Failed.  See Log for details", 
				         "Ok", true, "", false, "", false, false );
			}
		}
		else
			mGuiMain->getFileBrowser()->show("Save to File", "", FileBrowser::SAVE);
		hide();
	}

	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}

	else if ( actionEvent.getId() == "mBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "File" );
		switch ( Type ) {
			case CONVERT_KAT5200:
			case CONVERT_A800:
				mGuiMain->getFileBrowser()->show("Load File to Convert", "", FileBrowser::LOAD);
				break;
			case CONVERT_DIR:
				mGuiMain->getFileBrowser()->setShowOverwrite( false );
				mGuiMain->getFileBrowser()->show("Create or choose file", "", FileBrowser::SAVE);
				break;
		}
	}
	else if ( actionEvent.getId() == "File" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mFile.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->setShowOverwrite( true );
			mGuiMain->getFileBrowser()->hide();
		}
	}
	else if ( actionEvent.getId() == "FileSave" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mGuiMain->getFileBrowser()->hide();
			switch ( Type )
			{
				case CONVERT_KAT5200:
					if ( convert_kat_palette ( (mNameList.getLabelFromIndex(mName.getSelected())), 
					                      (char *)mGuiMain->getFileBrowser()->getResult().c_str() ) )
					{
						mGuiMain->getMessagePopup()->deleteActions();
						mGuiMain->getMessagePopup()->show ( "Conversion Failed.  See Log for details", 
						         "Ok", true, "", false, "", false, false );
					}
				break;
				case CONVERT_DIR:
					mGuiMain->getMessagePopup()->show ( "Scanning...","",false,"",false,"",false, false );
					if ( media_create_from_directory ( (char *)mFile.getText().c_str(), 
					                  (char *)mGuiMain->getFileBrowser()->getResult().c_str(),
					                  mOption2.isSelected() ? 1:0, mOption.isSelected() ? 1:0 ) )
					{
						mGuiMain->getMessagePopup()->hide();
						mGuiMain->getMessagePopup()->deleteActions();
						mGuiMain->getMessagePopup()->show ( "Creation/Update Failed.  See Log for details", 
						         "Ok", true, "", false, "", false, false );
					}
					else 
					{
						mGuiMain->getMessagePopup()->hide();
					}
				break;
			}
		}
	}
}
