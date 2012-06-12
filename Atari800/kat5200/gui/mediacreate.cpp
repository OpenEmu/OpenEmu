/******************************************************************************
*
* FILENAME: mediacreate.cpp
*
* DESCRIPTION:  This class will show the Media Creation window
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   06/25/08  bberlin      Creation
******************************************************************************/
#include "mediacreate.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/media.h"
	#include "../core/sio.h"

	extern t_sio_interface sio;
	extern t_cart cart;
}

MediaCreate::MediaCreate ( GuiMain *gMain ) : GuiObject(gMain)
{
	int i;

	setVisible ( false );
	setCaption ( "Media Creation" );

	/*
	 * Init Captions
	 */
	mCreateCasLabel.setCaption ( "Cassette" );
	mCreateCasDescLabel.setCaption ( "Description" );
	mCreateCasBrowse.setCaption ( "Create" );

	mCreateDiskLabel.setCaption ( "Disk" );
	mCreateDiskTypeLabel.setCaption ( "Sector Size / Boot Type" );
	mCreateDiskSectorsLabel.setCaption ( "Sectors" );
	mCreateDiskSectors.setText ( "720" );
	mCreateDiskBrowse.setCaption ( "Create" );
	mCreateDiskType.setListModel ( &mCreateDiskList );

	mCreateDiskList.deleteAll();
	mCreateDiskList.addElement ( "128 / 128", SIO_DSK_128, 0 );
	mCreateDiskList.addElement ( "256 / 128", SIO_DSK_256_BOOT_128, 0 );
	mCreateDiskList.addElement ( "256 / 256", SIO_DSK_256_BOOT_256, 0 );
	mCreateDiskList.addElement ( "256 / SIOPC", SIO_DSK_256_BOOT_SIO2PC, 0 );

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

	mCreateCasBrowse.addActionListener ( this );
	mCreateCasBrowse.setActionEventId ( "mCreateCasBrowse" );
	mCreateDiskBrowse.addActionListener ( this );
	mCreateDiskBrowse.setActionEventId ( "mCreateDiskBrowse" );

	/*
	 * Size and place our widgets
	 */
	add ( &mCreateCasLabel );
	add ( &mCreateCasDescLabel );
	add ( &mCreateCasDesc );
	add ( &mCreateCasBrowse );

	add ( &mCreateDiskLabel );
	add ( &mCreateDiskTypeLabel );
	add ( &mCreateDiskType );
	add ( &mCreateDiskSectorsLabel );
	add ( &mCreateDiskSectors );
	add ( &mCreateDiskBrowse );

	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(3,5.8,64,12.5) );

	i = -3;
	mCreateCasLabel.setDimension ( mGuiMain->getRectangle(1,2.7+1.7*(i+5),5,1) );
	mCreateCasDescLabel.setDimension ( mGuiMain->getRectangle(10,2.7+1.7*(i+4.2),5,1) );
	mCreateCasDesc.setDimension ( mGuiMain->getRectangle(10,2.7+1.7*(i+5),38,1) );
	mCreateCasBrowse.setDimension ( mGuiMain->getRectangle(50,2.7+1.7*(i+5),6,1) );

	mCreateDiskLabel.setDimension ( mGuiMain->getRectangle(1,2.7+1.7*(i+3.0),5,1) );
	mCreateDiskTypeLabel.setDimension ( mGuiMain->getRectangle(10,2.7+1.7*(i+2.2),5,1) );
	mCreateDiskType.setDimension ( mGuiMain->getRectangle(10,2.7+1.7*(i+3.0),14,1) );
	mCreateDiskSectorsLabel.setDimension ( mGuiMain->getRectangle(40,2.7+1.7*(i+2.2),7,1) );
	mCreateDiskSectors.setDimension ( mGuiMain->getRectangle(40,2.7+1.7*(i+3.0),8,1) );
	mCreateDiskBrowse.setDimension ( mGuiMain->getRectangle(50,2.7+1.7*(i+3.0),6,1) );

	mOk.setDimension ( mGuiMain->getRectangle(26,9,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(31,9,3,1) );

	mCreateCasLabel.adjustSize();
	mCreateCasDescLabel.adjustSize();
	mCreateCasDesc.adjustHeight();
	mCreateCasBrowse.adjustSize();

	mCreateDiskLabel.adjustSize();
	mCreateDiskTypeLabel.adjustSize();
	mCreateDiskType.adjustHeight();
	mCreateDiskSectorsLabel.adjustSize();
	mCreateDiskSectors.adjustHeight();
	mCreateDiskBrowse.adjustSize();

	mOk.adjustSize();
	mCancel.adjustSize();
}

MediaCreate::~MediaCreate ()
{

}

void MediaCreate::show ( )
{
	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void MediaCreate::hide ( )
{
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void MediaCreate::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();
	static int current_disk = 0;

	if ( actionEvent.getId() == "mOk" ) {
		hide();
	}

	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}

	else if ( actionEvent.getId() == "mCreateCasBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "CreateCas" );
		mGuiMain->getFileBrowser()->show("Save Cassette (Use .cas extension)", "", FileBrowser::SAVE);
	}
	else if ( actionEvent.getId() == "mCreateDiskBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId ( "CreateDisk" );
		mGuiMain->getFileBrowser()->show("Save Disk (Use .atr extension)", "", FileBrowser::SAVE);
	}
	else if ( actionEvent.getId() == "CreateCas" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			sio_create_cassette ( mGuiMain->getFileBrowser()->getResult().c_str(), 
			                      mCreateCasDesc.getText().c_str());
			mGuiMain->getFileBrowser()->hide();
		}
	}
	else if ( actionEvent.getId() == "CreateDisk" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			sio_create_disk ( mGuiMain->getFileBrowser()->getResult().c_str(), 
			                  atoi(mCreateDiskSectors.getText().c_str()),
			                  mCreateDiskList.getValueFromIndex(mCreateDiskType.getSelected()));
			mGuiMain->getFileBrowser()->hide();
		}
	}
}
