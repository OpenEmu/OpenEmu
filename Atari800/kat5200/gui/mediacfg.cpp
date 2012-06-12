/******************************************************************************
*
* FILENAME: mediacfg.cpp
*
* DESCRIPTION:  This class will show the MediaCfg
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   01/12/08  bberlin      Creation
******************************************************************************/
#include "mediacfg.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/media.h"
	#include "../core/sio.h"

	extern t_sio_interface sio;
	extern t_cart cart;
}

MediaCfg::MediaCfg ( GuiMain *gMain ) : GuiObject(gMain)
{
	int i;
	char tmp_string[10];

	setVisible ( false );
	setCaption ( "Media Files" );

	/*
	 * Init Captions
	 */
	mCartLabel.setCaption ( "Cart" );
	mCartBrowse.setCaption ( "Browse" );
	mCartClear.setCaption ( "Clear" );

	mCasLabel.setCaption ( "Cassette" );
	mCasBrowse.setCaption ( "Browse" );
	mCasClear.setCaption ( "Clear" );

	mRecord.setCaption ( "Rec" );
	mPlay.setCaption ( "Play" );
	mRewind.setCaption ( "RW" );
	mFastFoward.setCaption ( "FF" );
	mStop.setCaption ( "Stop" );

	mRecord.setGroup ( "Rec" );
	mPlay.setGroup ( "Play" );

	for ( i = 0; i < 8; ++i )
	{
		sprintf ( tmp_string, "Disk %d", i+1 );
		mDiskLabel[i].setCaption ( tmp_string );
		mDiskBrowse[i].setCaption ( "Browse" );
		mDiskClear[i].setCaption ( "Clear" );
	}

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

	mCartBrowse.addActionListener ( this );
	mCartBrowse.setActionEventId ( "mCartBrowse" );
	mCartClear.addActionListener ( this );
	mCartClear.setActionEventId ( "mCartClear" );

	mCasBrowse.addActionListener ( this );
	mCasBrowse.setActionEventId ( "mCasBrowse" );
	mCasClear.addActionListener ( this );
	mCasClear.setActionEventId ( "mCasClear" );

	mRewind.addActionListener ( this );
	mRewind.setActionEventId ( "mRewind" );
	mFastFoward.addActionListener ( this );
	mFastFoward.setActionEventId ( "mFastFoward" );
	mStop.addActionListener ( this );
	mStop.setActionEventId ( "mStop" );

	for ( i = 0; i < 8; ++i )
	{
		mDiskBrowse[i].addActionListener ( this );
		mDiskBrowse[i].setActionEventId ( "mDiskBrowse" );
		mDiskClear[i].addActionListener ( this );
		mDiskClear[i].setActionEventId ( "mDiskClear" );
		mDiskFile[i].addMouseListener ( this );
	}

	mCartFile.addMouseListener ( this );
	mCasFile.addMouseListener ( this );

	/*
	 * Size and place our widgets
	 */
	add ( &mCartLabel );
	add ( &mCartFile );
	add ( &mCartBrowse );
	add ( &mCartClear );

	add ( &mCasLabel );
	add ( &mCasFile );
	add ( &mCasBrowse );
	add ( &mCasClear );

	add ( &mRecord );
	add ( &mPlay );
	add ( &mRewind );
	add ( &mFastFoward );
	add ( &mStop );

	for ( i = 0; i < 8; ++i )
	{
		add ( &mDiskLabel[i] );
		add ( &mDiskFile[i] );
		add ( &mDiskBrowse[i] );
		add ( &mDiskClear[i] );
	}

	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(3,1.8,74,24.5) );

	mCartLabel.setDimension ( mGuiMain->getRectangle(1,1,30,1) );
	mCartFile.setDimension ( mGuiMain->getRectangle(10,1,46,1) );
	mCartBrowse.setDimension ( mGuiMain->getRectangle(57,1,6,1) );
	mCartClear.setDimension ( mGuiMain->getRectangle(66,1,6,1) );

	for ( i = 0; i < 8; ++i )
	{
		mDiskLabel[i].setDimension ( mGuiMain->getRectangle(1,2.7+1.7*i,30,1) );
		mDiskFile[i].setDimension ( mGuiMain->getRectangle(10,2.7+1.7*i,46,1) );
		mDiskBrowse[i].setDimension ( mGuiMain->getRectangle(57,2.7+1.7*i,6,1) );
		mDiskClear[i].setDimension ( mGuiMain->getRectangle(66,2.7+1.7*i,6,1) );
	}

	mCasLabel.setDimension ( mGuiMain->getRectangle(1,2.7+1.7*i,30,1) );
	mCasFile.setDimension ( mGuiMain->getRectangle(10,2.7+1.7*i,46,1) );
	mCasBrowse.setDimension ( mGuiMain->getRectangle(57,2.7+1.7*i,6,1) );
	mCasClear.setDimension ( mGuiMain->getRectangle(66,2.7+1.7*i,6,1) );

	mRecord.setDimension ( mGuiMain->getRectangle(10,2.7+1.7*(i+1),5,1) );
	mPlay.setDimension ( mGuiMain->getRectangle(15,2.7+1.7*(i+1),5,1) );
	mRewind.setDimension ( mGuiMain->getRectangle(21,2.7+1.7*(i+1),5,1) );
	mFastFoward.setDimension ( mGuiMain->getRectangle(25,2.7+1.7*(i+1),5,1) );
	mStop.setDimension ( mGuiMain->getRectangle(29,2.7+1.7*(i+1),5,1) );

	mOk.setDimension ( mGuiMain->getRectangle(32,21,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(37,21,3,1) );

	mCartLabel.adjustSize();
	mCartBrowse.adjustSize();
	mCartClear.adjustSize();
	mCartFile.adjustHeight();

	mCasLabel.adjustSize();
	mCasBrowse.adjustSize();
	mCasClear.adjustSize();
	mCasFile.adjustHeight();

	mRecord.adjustSize();
	mPlay.adjustSize();
	mRewind.adjustSize();
	mFastFoward.adjustSize();
	mStop.adjustSize();

	for ( i = 0; i < 8; ++i )
	{
		mDiskLabel[i].adjustSize();
		mDiskBrowse[i].adjustSize();
		mDiskClear[i].adjustSize();
		mDiskFile[i].adjustHeight();
	}

	mOk.adjustSize();
	mCancel.adjustSize();
}

MediaCfg::~MediaCfg ()
{

}

void MediaCfg::show ( )
{
	int i;
	t_config *p_config = config_get_ptr();

	mSettingsDirty = false;

	if ( cart.loaded )
		mCartFile.setText( cart.filename );
	else
		mCartFile.setText( "" );

	if ( sio.cassette.dev.f_buffer )
		mCasFile.setText( sio.cassette.dev.filename );
	else
		mCasFile.setText( "" );

	for ( i = 0; i < 8; ++i )
	{
		if ( sio.drives[i].dev.f_buffer )
			mDiskFile[i].setText( sio.drives[i].dev.filename );
		else
			mDiskFile[i].setText( "" );
	}

	mRecord.setSelected ( true );
	mPlay.setSelected ( true );

	setMachine();

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void MediaCfg::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void MediaCfg::setMachine ( )
{
	t_config *p_config = config_get_ptr();
	bool enable;
	int i;

	if ( p_config->machine_type != MACHINE_TYPE_5200 )
		enable = true;
	else
		enable = false;

	mCasLabel.setVisible(enable);
	mCasFile.setVisible(enable);
	mCasBrowse.setVisible(enable);
	mCasClear.setVisible(enable);

	for ( i = 0; i < MAX_SIO_DEVICES; ++i )
	{
		mDiskLabel[i].setVisible(enable);
		mDiskFile[i].setVisible(enable);
		mDiskBrowse[i].setVisible(enable);
		mDiskClear[i].setVisible(enable);
	}

	mRecord.setVisible(enable);
	mPlay.setVisible(enable);
	mRewind.setVisible(enable);
	mFastFoward.setVisible(enable);
	mStop.setVisible(enable);
}

void MediaCfg::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();
	static int current_disk = 0;
	char file[1100];
	int i;

	for ( i = 0; i < 8; ++i )
	{
		if ( actionEvent.getSource() == &mDiskBrowse[i] ||
		     actionEvent.getSource() == &mDiskClear[i] )
			current_disk = i;
	}

	if ( actionEvent.getId() == "mOk" ) {
		if ( mSettingsDirty )
		{
			for ( i = 0; i < 8; ++i )
			{
				if ( mDiskFile[i].getText().size() ) 
				{
					strcpy ( file, mDiskFile[i].getText().c_str() );
					util_strip_to_file ( file );
					if ( !strstr ( sio.drives[i].dev.filename, file ) )
					{
						strcpy ( file, mDiskFile[i].getText().c_str() );
						media_open_rom ( file, CART_FLOPPY, i, 0 );
					}
				}
				else
					sio_unmount_image ( SIO_TYPE_FLOPPY, i );
			}

			if ( mCasFile.getText().size() ) 
			{
				strcpy ( file, mCasFile.getText().c_str() );
				util_strip_to_file ( file );
				if ( !strstr ( sio.cassette.dev.filename, file ) )
				{
					strcpy ( file, mCasFile.getText().c_str() );
					media_open_rom ( file, CART_CASSETTE, i, 0 );
				}
			}
			else
				sio_unmount_image ( SIO_TYPE_CASSETTE, 0 );

			if ( mCartFile.getText().size() ) 
			{
				strcpy ( file, mCartFile.getText().c_str() );
				util_strip_to_file ( file );
				if ( !strstr ( cart.filename, file ) )
				{
					strcpy ( file, mCartFile.getText().c_str() );
					media_open_rom ( file, CART_CART, i, 0 );
				}
			}
			else {
				strcpy ( cart.filename, "" );
				cart.loaded = 0;
			}
		}
		hide();
	}

	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}

	else if ( actionEvent.getId() == "mCartBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FileCart" );
		mGuiMain->getFileBrowser()->show("Load Cart", p_config->os_file_5200, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "mCasBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FileCas" );
		mGuiMain->getFileBrowser()->show("Load Cassette", p_config->os_file_5200, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "mDiskBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "FileDisk" );
		mGuiMain->getFileBrowser()->show("Load Disk", p_config->os_file_800, FileBrowser::LOAD);
	}
	else if ( actionEvent.getId() == "FileCart" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mCartFile.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
			mSettingsDirty = true;
		}
	}
	else if ( actionEvent.getId() == "FileCas" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mCasFile.setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
			mSettingsDirty = true;
		}
	}
	else if ( actionEvent.getId() == "FileDisk" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mDiskFile[current_disk].setText ( mGuiMain->getFileBrowser()->getResult() );
			mGuiMain->getFileBrowser()->hide();
			mSettingsDirty = true;
		}
	}
	else if ( actionEvent.getId() == "mCartClear" ) {
		mCartFile.setText ( "" );
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mCasClear" ) {
		mCasFile.setText ( "" );
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mDiskClear" ) {
		mDiskFile[current_disk].setText ( "" );
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mRewind" ) {
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mFastFoward" ) {
		mSettingsDirty = true;
	}
	else if ( actionEvent.getId() == "mStop" ) {
		mSettingsDirty = true;
		mRecord.setSelected ( false );
		mPlay.setSelected ( false );
	}
}
