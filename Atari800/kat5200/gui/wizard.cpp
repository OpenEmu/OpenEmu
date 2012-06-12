/******************************************************************************
*
* FILENAME: wizard.cpp
*
* DESCRIPTION:  This class will show the configuration wizard
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   07/12/07  bberlin      Creation
******************************************************************************/
#include "wizard.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/media.h"
	#include "../interface/db_if.h"

	extern t_atari_video g_video;
	extern t_atari_input g_input;
}

Wizard::Wizard ( GuiMain *gMain ) : GuiObject(gMain)
{
	t_config *p_config = config_get_ptr();

	setVisible ( false );
	setCaption ("Configuration Wizard");

	/*
	 * Setup all of our widgets
	 */
	mSysDec.setCaption ( "Video" );
	mVidLabel.setCaption ("Video System");
	mVid.setListModel (&mVidList);
	mFullscreen.setCaption ("Fullscreen");
	mZoomLabel.setCaption ("Video Zoom");
	mZoom.setListModel (&mZoomList);

	mRomDec.setCaption ( "ROM Image Directories (Check to Scan)" );
	m5200Rom.setCaption ("Atari 5200 Image Directory");
	m5200Rom.setSelected ( true );
	m5200Browse.setCaption ("Browse");
	m5200ScanSubs.setCaption ("Scan Sub-directories");
	m5200ScanSubs.setSelected (true);
	m5200Bios.setCaption ("Scan for BIOS?");
	m5200Bios.setSelected (true); 
	m8bitRom.setCaption ("Atari 8-bit Image Directory");
	m8bitRom.setSelected (true);
	m8bitBrowse.setCaption ("Browse");
	m8bitScanSubs.setCaption ("Scan Sub-directories");
	m8bitScanSubs.setSelected (true);
	m8bitBios.setCaption ("Scan for BIOS?");
	m8bitBios.setSelected (true); 
	mBiosRom.setCaption ("BIOS/BASIC Directory");
	mBiosBrowse.setCaption ("Browse");
	mBiosScanSubs.setCaption ("Scan Sub-directories");
	mBiosScanSubs.setSelected (true);

	mInput.setListModel (&mInputList);
	mAutoDetect.setCaption ("Auto-Detect");
	mInputSelect.setCaption ("");
	mInputSelect.setSelected (true);
	mInputMessage.setCaption ("Use desired Stick or D-Pad if Auto-Detecting Joystick");
	mInputDec.setCaption ( "Primary Input Device (Check to Setup)" );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	m5200Browse.addActionListener ( this );
	m8bitBrowse.addActionListener ( this );
	mBiosBrowse.addActionListener ( this );
	mAutoDetect.addActionListener ( this );
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );

	m5200Browse.setActionEventId ( "m5200Browse" );
	m8bitBrowse.setActionEventId ( "m8bitBrowse" );
	mBiosBrowse.setActionEventId ( "mBiosBrowse" );
	mAutoDetect.setActionEventId ( "mAutoDetect" );
	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );

	/*
	 * Size and place our widgets
	 */
	add ( &mMessage );
	add ( &mMessage1 );

	add ( &mSysDec );
	add ( &mVidLabel );
	add ( &mVid );
	add ( &mFullscreen );
	add ( &mZoom );
	add ( &mZoomLabel );

	add ( &mRomDec );
	add ( &m5200Rom);
	add ( &m5200Box );
	add ( &m5200Browse );
	add ( &m5200ScanSubs );
	add ( &m5200Bios );
	add ( &m8bitRom);
	add ( &m8bitBox );
	add ( &m8bitBrowse );
	add ( &m8bitScanSubs );
	add ( &m8bitBios );
	add ( &mBiosRom);
	add ( &mBiosBox );
	add ( &mBiosBrowse );
	add ( &mBiosScanSubs );

	add ( &mInputDec );
	add ( &mInput );
	add ( &mInputMessage );
	add ( &mInputSelect );
	add ( &mAutoDetect );

	add ( &mOk );
	add ( &mCancel );

	setDimension ( mGuiMain->getRectangle(3,1.5,73,26) );

	mMessage.setDimension ( mGuiMain->getRectangle(0.5,0,71,1.0) );
	mMessage1.setDimension ( mGuiMain->getRectangle(0.5,1.0,71,1.0) );

	mVidLabel.setDimension ( mGuiMain->getRectangle(1,4.0,8,2) );
	mVid.setDimension ( mGuiMain->getRectangle(14,4.0,15,2) );
	mZoomLabel.setDimension ( mGuiMain->getRectangle(31,4.0,12,1) );
	mZoom.setDimension ( mGuiMain->getRectangle(42,4.0,10,1) );
	mFullscreen.setDimension ( mGuiMain->getRectangle(54,4.0,8,2) );
	mSysDec.setDimension ( mGuiMain->getRectangle(0.5,2.5,71,3.0) );

	m5200Rom.setDimension ( mGuiMain->getRectangle(1,7.2,8,2) );
	m5200Box.setDimension ( mGuiMain->getRectangle(1,8.4,60,2) );
	m5200Browse.setDimension ( mGuiMain->getRectangle(62,8.4,7,2) );
	m5200ScanSubs.setDimension ( mGuiMain->getRectangle(47,7.2,7,2) );
	m5200Bios.setDimension ( mGuiMain->getRectangle(30,7.2,7,2) );
	m8bitRom.setDimension ( mGuiMain->getRectangle(1,10.8,8,2) );
	m8bitBox.setDimension ( mGuiMain->getRectangle(1,12.0,60,2) );
	m8bitBrowse.setDimension ( mGuiMain->getRectangle(62,12.0,7,2) );
	m8bitScanSubs.setDimension ( mGuiMain->getRectangle(47,10.8,7,2) );
	m8bitBios.setDimension ( mGuiMain->getRectangle(30,10.8,7,2) );
	mBiosRom.setDimension ( mGuiMain->getRectangle(1,14.5,8,2) );
	mBiosBox.setDimension ( mGuiMain->getRectangle(1,15.7,60,2) );
	mBiosBrowse.setDimension ( mGuiMain->getRectangle(62,15.7,7,2) );
	mBiosScanSubs.setDimension ( mGuiMain->getRectangle(47,14.5,7,2) );
	mRomDec.setDimension ( mGuiMain->getRectangle(0.5,6.0,71,11.5) );

	mInputSelect.setDimension ( mGuiMain->getRectangle(1,19.5,2,2) );
	mInput.setDimension ( mGuiMain->getRectangle(4,19.5,51,2) );
	mInputMessage.setDimension ( mGuiMain->getRectangle(4,20.8,51,1) );
	mAutoDetect.setDimension ( mGuiMain->getRectangle(57,19.5,12,2) );
	mInputDec.setDimension ( mGuiMain->getRectangle(0.5,18.0,71,4.5) );

	mOk.setDimension ( mGuiMain->getRectangle(30,22.9,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(37,22.9,6,1) );

	mVid.adjustHeight();
	mVidLabel.adjustSize();
	mZoomLabel.adjustSize();
	mFullscreen.adjustSize();

	m5200Rom.adjustSize();
	m8bitRom.adjustSize();
	mBiosRom.adjustSize();
	m5200Box.adjustHeight();
	m8bitBox.adjustHeight();
	mBiosBox.adjustHeight();
	m5200Browse.adjustSize();
	m8bitBrowse.adjustSize();
	mBiosBrowse.adjustSize();
	m5200ScanSubs.adjustSize();
	m8bitScanSubs.adjustSize();
	mBiosScanSubs.adjustSize();
	m5200Bios.adjustSize();
	m8bitBios.adjustSize();

	mInputSelect.adjustSize();
	mInput.adjustHeight();
	mZoom.adjustHeight();
	mAutoDetect.adjustSize();

	mOk.adjustSize();
	mCancel.adjustSize();
}

Wizard::~Wizard ()
{
}

int wizard_callback ( void *wizard, int argc, char **argv, char **azColName ) {

	int i;
	char name[100];
	std::string text;
	Wizard *mWiz;

	mWiz = (Wizard *) wizard;

	if ( argc < 4 )
		return -1;

	if ( argv[i=0] ) strcpy ( name, argv[i] );
	if ( strstr ( name, "5200" ) )
	{
		if ( argv[++i] ) mWiz->m5200Rom.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->m5200ScanSubs.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->m5200Bios.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->m5200Box.setText ( text = argv[i] );
	}
	if ( strstr ( name, "800" ) )
	{
		if ( argv[++i] ) mWiz->m8bitRom.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->m8bitScanSubs.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->m8bitBios.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->m8bitBox.setText ( text = argv[i] );
	}
	if ( strstr ( name, "Bios" ) )
	{
		if ( argv[++i] ) mWiz->mBiosRom.setSelected ( atoi(argv[i]) ? true : false );
		if ( argv[++i] ) mWiz->mBiosScanSubs.setSelected ( atoi(argv[i]) ? true : false );
		++i;
		if ( argv[++i] ) mWiz->mBiosBox.setText ( text = argv[i] );
	}

	return 0;

} /* end wizard_callback */

void Wizard::show ( std::string message )
{
	int total;
	char statement[50];
	std::string tmp_string;
	t_config *p_config = config_get_ptr();

	mAutoDetectAct = -1;

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);

	tmp_string = message;
	if ( message.size() > 70 )
		message.erase( 69 );
	mMessage.setCaption ( message );
	if ( tmp_string.size() > 68 ) {
		tmp_string.erase(0,69);
		mMessage1.setCaption ( tmp_string );
	}

	/*
	 * Create Lists
	 */
	mVidList.deleteAll();
	mVidList.addElement ( "NTSC", NTSC, 0 );
	mVidList.addElement ( "PAL", PAL, 0 );

	mZoomList.deleteAll();
	mZoomList.addElement ( "1x", 0, 0 );
	mZoomList.addElement ( "2x", 1, 0 );
	mZoomList.addElement ( "3x", 2, 0 );

	total = updateInputList();

	/*
	 * Set Selected to Current
	 */
	mVid.setSelected(mVidList.getIndexFromValue(p_config->system_type));
	mInput.setSelected(mInputList.getIndexFromValue(0));
	mFullscreen.setSelected( g_video.fullscreen ? true : false );
	mZoom.setSelected( mZoomList.getIndexFromValue(g_video.zoom) );
	if ( g_input.players[0].stick.direction[0].device == DEV_JOYSTICK )
		mInput.setSelected(mVidList.getIndexFromValue(g_input.players[0].stick.direction[0].device_num+1));

	/*
	 * Set options stored in database
	 */
	sprintf ( statement, "SELECT * FROM Wizard WHERE Name='5200'" );
	db_if_exec_sql ( statement, wizard_callback, this );
	sprintf ( statement, "SELECT * FROM Wizard WHERE Name='800'" );
	db_if_exec_sql ( statement, wizard_callback, this );
	sprintf ( statement, "SELECT * FROM Wizard WHERE Name='Bios'" );
	db_if_exec_sql ( statement, wizard_callback, this );

	//mGuiMain->Message ( GuiMain::MSG_JOY_CHECK, &total );
}

void Wizard::hide ( )
{
	int total = 0;

	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);

	//mGuiMain->Message ( GuiMain::MSG_NO_JOY_CHECK, &total );
}

int Wizard::updateInputList ( )
{
	int selected, i, total;
	char joyname[260];

	selected = mInput.getSelected();

	total = pc_detect_and_open_joysticks ();

	mInputList.deleteAll();
	mInputList.addElement ( "Keyboard", 0, 0 );
	for ( i = 0; i < total; ++i ) {
		sprintf ( joyname, "J%d - %s", i, pc_get_joystick_name(i) );
		mInputList.addElement ( joyname, i+1, 0 );
	}

	mInput.setSelected( selected );

	return total;
}

int Wizard::scanImages ( )
{
	int scan_subs;
	int bios_check;
	char dir[1100];
	char statement[1200];
	char *p_tmp;

	/*
	 * Check for 5200 images to scan for
	 */
	scan_subs = m5200ScanSubs.isSelected() ? 1 : 0;
	bios_check = m5200Bios.isSelected() ? 1 : 0;
	strcpy ( dir, m5200Box.getText().c_str() );
	if ( m5200Rom.isSelected() )
	{
		media_clear_filenames ( MACHINE_TYPE_5200 );
		media_scan_directory ( MACHINE_TYPE_5200, dir, scan_subs, bios_check );
	}
	sprintf ( statement, "INSERT OR REPLACE INTO Wizard VALUES ( '5200', %d, %d, %d, '%s' )",
	            m5200Rom.isSelected() ? 1 : 0, scan_subs, bios_check, 
	            p_tmp=util_replace_and_copy(dir, "'", "''" ) ); 
	free ( p_tmp );
	db_if_exec_sql ( statement, 0, 0 );

	/*
	 * Check for 8bit images to scan for
	 */
	scan_subs = m8bitScanSubs.isSelected() ? 1 : 0;
	bios_check = m8bitBios.isSelected() ? 1 : 0;
	strcpy ( dir, m8bitBox.getText().c_str() );
	if ( m8bitRom.isSelected() )
	{
		media_clear_filenames ( MACHINE_TYPE_MIN );
		media_scan_directory ( MACHINE_TYPE_MIN, dir, scan_subs, bios_check );
	}
	sprintf ( statement, "INSERT OR REPLACE INTO Wizard VALUES ( '800', %d, %d, %d, '%s' )",
	            m8bitRom.isSelected() ? 1 : 0, scan_subs, bios_check, 
	            p_tmp=util_replace_and_copy(dir, "'", "''")  ); 
	free ( p_tmp );
	db_if_exec_sql ( statement, 0, 0 );

	/*
	 * Check for Bios images to scan for
	 */
	scan_subs = mBiosScanSubs.isSelected() ? 1 : 0;
	strcpy ( dir, mBiosBox.getText().c_str() );
	if ( mBiosRom.isSelected() ) {
		media_bios_check ( 1, 0, "" );
		media_scan_for_bios ( dir, scan_subs );
		config_save();
	}
	sprintf ( statement, "INSERT OR REPLACE INTO Wizard VALUES ( 'Bios', %d, %d, %d, '%s' )",
	            mBiosRom.isSelected() ? 1 : 0, scan_subs, 1, 
	            p_tmp=util_replace_and_copy(dir, "'", "''") ); 
	free ( p_tmp );
	db_if_exec_sql ( statement, 0, 0 );

	return 0;
}

void Wizard::action(const gcn::ActionEvent &actionEvent)
{
	int input_value;
	int i;
	int total = 0;
	char joyname[260];
	char name[50];
	t_config *p_config = config_get_ptr();
	static t_atari_common stick;
	static int first_axis_event = 0;
	t_atari_input j_input = g_input;
	e_machine_type tmp_machine;

	if ( actionEvent.getId() == "mOk" ) {
		p_config->system_type = mVidList.getValueFromIndex(mVid.getSelected());
		config_save();
		g_video.fullscreen = mFullscreen.isSelected() ? 1 : 0;
		g_video.zoom = mZoomList.getValueFromIndex( mZoom.getSelected() );
		video_save_profile ( p_config->default_video_profile, &g_video );

		/*
		 * Tell the GUI to go scan after showing the "Wait" message
		 */
		mGuiMain->getMessagePopup()->show( 
		            "Scanning... (Application may appear to freeze)","",
		             false,"",false,"",false, false );
		mGuiMain->Message ( GuiMain::MSG_SCAN_WIZARD, &total );

		/*
		 * Now adjust input for selected device and save
		 */
		if ( mInputSelect.isSelected() ) {

		input_value = mInputList.getValueFromIndex ( mInput.getSelected() );

		if ( input_value > 0 ) {
			for ( i = 0; i < MAX_DIR; ++i ) {
				j_input.players[0].stick.direction[i].device = DEV_JOYSTICK;
				j_input.players[0].stick.direction[i].device_num = input_value-1;
				j_input.players[0].stick.direction[i].part_num = 0;
				if ( stick.device == DEV_JOYSTICK && stick.part_type == PART_TYPE_HAT )
					j_input.players[0].stick.direction[i].part_type = PART_TYPE_HAT;
				else
					j_input.players[0].stick.direction[i].part_type = PART_TYPE_AXIS;
			}
			if ( j_input.players[0].stick.direction[0].part_type == PART_TYPE_AXIS ) {
				j_input.players[0].stick.direction[DIR_LEFT].direction = DIR_MINUS;
				j_input.players[0].stick.direction[DIR_RIGHT].direction = DIR_PLUS;
				j_input.players[0].stick.direction[DIR_UP].part_num = 1;
				j_input.players[0].stick.direction[DIR_UP].direction = DIR_MINUS;
				j_input.players[0].stick.direction[DIR_DOWN].part_num = 1;
				j_input.players[0].stick.direction[DIR_DOWN].direction = DIR_PLUS;
			}
			else {
				j_input.players[0].stick.direction[DIR_LEFT].direction = DIR_LEFT;
				j_input.players[0].stick.direction[DIR_RIGHT].direction = DIR_RIGHT;
				j_input.players[0].stick.direction[DIR_UP].direction = DIR_UP;
				j_input.players[0].stick.direction[DIR_DOWN].direction = DIR_DOWN;
			}

			for ( i = 0; i < MAX_DIR; ++i ) {
				j_input.players[0].paddles.direction[i].device = 
				        j_input.players[0].stick.direction[i].device;
				j_input.players[0].paddles.direction[i].device_num = 
				        j_input.players[0].stick.direction[i].device_num;
				j_input.players[0].paddles.direction[i].part_num = 
				        j_input.players[0].stick.direction[i].part_num;
				j_input.players[0].paddles.direction[i].part_type = 
				        j_input.players[0].stick.direction[i].part_type;
				j_input.players[0].paddles.direction[i].direction = 
				        j_input.players[0].stick.direction[i].direction;
			}

			j_input.players[0].bottom_button.device = DEV_JOYSTICK;
			j_input.players[0].bottom_button.device_num = input_value-1;
			j_input.players[0].bottom_button.part_type = PART_TYPE_BUTTON;
			j_input.players[0].bottom_button.part_num = 0;
			j_input.players[0].bottom_button.direction = DIR_PRESS;

			j_input.players[0].top_button.device = DEV_JOYSTICK;
			j_input.players[0].top_button.device_num = input_value-1;
			j_input.players[0].top_button.part_type = PART_TYPE_BUTTON;
			j_input.players[0].top_button.part_num = 1;
			j_input.players[0].top_button.direction = DIR_PRESS;

			sprintf ( j_input.description, "J%d - %s", input_value-1, 
			                  pc_get_joystick_name(input_value-1) );
			tmp_machine = p_config->machine_type;

			p_config->machine_type = j_input.machine_type = MACHINE_TYPE_5200;
			input_save_profile ( "Joystick Input", &j_input );
			strcpy ( p_config->default_input_profile, "Joystick Input" );

			p_config->machine_type = j_input.machine_type = MACHINE_TYPE_XL;
			input_save_profile ( "Joystick 800 Input", &j_input );
			strcpy ( p_config->default_800input_profile, "Joystick 800 Input" );

			p_config->machine_type = tmp_machine;
			config_save();
		}
		else {
			tmp_machine = p_config->machine_type;
			strcpy ( j_input.description, "Keyboard" );

			for ( i = 0; i < 2; ++i ) {
				if ( i == 0 ) {
					p_config->machine_type = j_input.machine_type = MACHINE_TYPE_5200;
					sprintf ( name, "Keyboard Input" );
				}
				else {
					p_config->machine_type = j_input.machine_type = MACHINE_TYPE_XL;
					sprintf ( name, "Keyboard 800 Input" );
				}
				input_set_defaults();
				j_input = g_input;
				input_save_profile ( name, &j_input );
				if ( i == 0 )
					strcpy ( p_config->default_input_profile, name );
				else
					strcpy ( p_config->default_800input_profile, name );
			}

			p_config->machine_type = tmp_machine;
			config_save();
		}

		} /* end if Input Selected */

		hide();
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}
	else if ( actionEvent.getId() == "m5200Browse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId("Load5200Browser");
		mGuiMain->getFileBrowser()->show("Navigate to Directory to Scan and Press Select",
		                                m5200Box.getText(), FileBrowser::DIR_ONLY);
	}
	else if ( actionEvent.getId() == "Load5200Browser" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mGuiMain->getFileBrowser()->hide();
			m5200Box.setText ( mGuiMain->getFileBrowser()->getResult() );
		}
	}
	else if ( actionEvent.getId() == "m8bitBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId("Load8bitBrowser");
		mGuiMain->getFileBrowser()->show("Navigate to Directory to Scan and Press Select",
		                                m8bitBox.getText(), FileBrowser::DIR_ONLY);
	}
	else if ( actionEvent.getId() == "Load8bitBrowser" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mGuiMain->getFileBrowser()->hide();
			m8bitBox.setText ( mGuiMain->getFileBrowser()->getResult() );
		}
	}
	else if ( actionEvent.getId() == "mBiosBrowse" ) {
		mGuiMain->getFileBrowser()->deleteActions();
		mGuiMain->getFileBrowser()->addActionListener(this);
		mGuiMain->getFileBrowser()->setActionEventId("LoadBiosBrowser");
		mGuiMain->getFileBrowser()->show("Navigate to Directory to Scan and Press Select",
		                                m8bitBox.getText(), FileBrowser::DIR_ONLY);
	}
	else if ( actionEvent.getId() == "LoadBiosBrowser" ) {
		if ( mGuiMain->getFileBrowser()->getCancelPressed() == false ) {
			mGuiMain->getFileBrowser()->hide();
			mBiosBox.setText ( mGuiMain->getFileBrowser()->getResult() );
		}
	}
	else if ( actionEvent.getId() == "mAutoDetect" ) {
		if ( mAutoDetectAct == -1 ) 
		{
			//mGuiMain->Message ( GuiMain::MSG_NO_JOY_CHECK, &total );
			total = pc_detect_and_open_joysticks ();
			first_axis_event = 1;

			mAutoListener.mCountModKey = true;
			mAutoListener.setWindow ( mGuiMain->getAutoWindow() );

			mGuiMain->getAutoWindow()->deleteAllListeners ( );
			mGuiMain->getAutoWindow()->addActionListener ( this );
			mGuiMain->getAutoWindow()->addKeyListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->addMouseListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->addJoystickListener ( &mAutoListener );
			mGuiMain->getAutoWindow()->setActionEventId ( "mAutoWindow" );
			mGuiMain->getAutoWindow()->setDimension ( mGuiMain->getRectangle(9,18.0,60,7) );

			mGuiMain->showAutoWindow( "User Action Required", "Perform action on desired Input Device" );

			mGuiMain->getKatGui()->setJoystickEnable ( false );
			mInputList.deleteAll();
			mInputList.addElement ( "Keyboard", 0, 0 );
			for ( i = 0; i < total; ++i ) {
				sprintf ( joyname, "J%d - %s", i, pc_get_joystick_name(i) );
				mInputList.addElement ( joyname, i+1, 0 );
			}
		}
		else {
			mAutoDetectAct--;
		}
	}
	else if ( actionEvent.getId() == "mAutoWindow" ) {
		mAutoListener.getInput ( 0, &stick.device, 
		                            &stick.device_num,
		                            &stick.part_type, 
		                            &stick.part_num,
		                            &stick.direction );
		if ( stick.device != DEV_JOYSTICK || 
		     stick.part_type != PART_TYPE_AXIS || !first_axis_event )
		{
			mGuiMain->hideAutoWindow();
			//mGuiMain->Message ( GuiMain::MSG_JOY_CHECK, &total );
			if ( stick.device == DEV_MOUSE && 
			     stick.part_type == PART_TYPE_BUTTON )
				mAutoDetectAct = 0;

			if ( stick.device == DEV_KEYBOARD )
				mInput.setSelected ( 0 );
			else if ( stick.device == DEV_JOYSTICK )
				mInput.setSelected ( stick.device_num + 1 );
		}
		first_axis_event = 0;
	}
}

