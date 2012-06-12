/******************************************************************************
*
* FILENAME: guimain.cpp
*
* DESCRIPTION: This is the main class where everything is created and run 
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/02/06  bberlin      Creation
******************************************************************************/
#include "guimain.hpp"

#include <sys/stat.h>
#include <zlib.h>

extern "C" {
	#include "default_font.h"
	#include "../interface/sdl_if.h"
	#include "../interface/media.h"
	#include "../interface/kconfig.h"
	#include "../interface/util.h"
	#include "../interface/states.h"
	#include "../interface/input.h"
	#include "../interface/ui.h"
	#include "../interface/video.h"
	#include "../interface/sound.h"
	#include "../interface/logger.h"
	#include "../core/gtia.h"
	#include "../core/memory.h"
	#include "../core/console.h"
	#include "../core/pia.h"
	#include "../kat5200.h"

	extern t_atari_sound g_sound;
	extern t_atari_input g_input;
	extern t_ui g_ui;
}

GuiMain::GuiMain(int error, int debug_flag)
{
	t_config *p_config = config_get_ptr ();
	FILE *fp;
	gzFile zip_fp;
	struct stat file_status;
	char msg[30];
	unsigned char *new_buffer;
	int i;
	char string[1100];
	char *tmp;

	/*
	 * Setup the initial screen and input stuff
	 */
	SDL_CloseAudio();
	sprintf ( msg, "kat5200 %s", VERSION );
	strcpy ( string, "kat5200.bmp" );
	util_set_file_to_program_dir ( string );
	SDL_WM_SetIcon(SDL_LoadBMP(string), NULL);
	SDL_WM_SetCaption(msg, "");
	SDL_WM_GrabInput(SDL_GRAB_OFF);

	mScreen = (SDL_Surface *) pc_set_gui_video ();

	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_SetModState(KMOD_NONE);

	mImageLoader = new gcn::SDLImageLoader();
	gcn::Image::setImageLoader(mImageLoader); 
	mGraphics = new gcn::SDLGraphics();
	mGraphics->setTarget(mScreen);
	//mInput = new gcn::SDLInput();
	mInput = new gcn::SDLGuiInput();
	
	mTop = new gcn::Container();    
	mTop->setDimension(gcn::Rectangle(0, 0, mScreen->w, mScreen->h));

	//mGui = new gcn::Gui();
	mGui = new gcn::KatGui();
	mGui->setGraphics(mGraphics);
	mGui->setInput(mInput); 
	mGui->setTop(mTop);
	mGui->addGlobalKeyListener ( this );
	mGui->addGlobalJoystickListener ( this );
	pc_detect_and_open_joysticks ();

	/*
	 * Check for Font file, if not there create
	 */
	strcpy ( string, p_config->font_file );
	util_set_file_to_program_dir ( string );
	if ( stat ( string, &file_status ) ) {
		sprintf ( string, "GuiMain: Font File \"%s\" ... does not exist!", p_config->font_file );
		logger_log_message ( LOG_ERROR, string, "" );
		strcpy ( string, "GuiMain: Attempting to use internal default..." );
		logger_log_message ( LOG_INFO, string, "" );
		
		/*
		 * Use the internal default font
		 */
		strcpy ( string, "tmp_font.bmp.gz" );
		util_set_file_to_program_dir ( string );

		fp = fopen ( string, "wb" );
		if ( fp ) {
			fwrite ( default_font, 1, default_font_length, fp );
			fclose ( fp );
		}

		new_buffer = (unsigned char *)malloc(55000);

		zip_fp = gzopen ( string, "rb" );
		if ( zip_fp ) {
			gzread ( zip_fp, new_buffer, 54582 );
		}
		gzclose (zip_fp);

		tmp = strstr ( string, ".gz" );
		*tmp = '\0';
			
		fp = fopen ( string, "wb" );
		if ( fp ) {
			for ( i = 0; i < 54582; ++i )
				fputc ( new_buffer[i], fp );
			fclose ( fp );
		}
		free ( new_buffer );
		mFont = new gcn::ImageFont(string, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&`'*#=[]\"<>{}^~|_@$\\");
	}
	else {
		mFont = new gcn::ImageFont(string, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!?-+/():;%&`'*#=[]\"<>{}^~|_@$\\");
	}
	gcn::Widget::setGlobalFont(mFont); 

	/*
	 * Setup the Auto Input Window
	 */
	mAutoWindow.setCaption ( "User Action Required" );
	mAutoWindow.add ( &mAutoMessage );
	mAutoMessage.setDimension ( getRectangle(5,1,35,1) );
	mAutoMessage.setFocusable( false );
	mTop->add(&mAutoWindow);
	mGui->addJoystickWindow(&mAutoWindow);

	/*
	 * Set us as GuiMain for objects that need it
	 */
	mBiosCfg = new BiosCfg(this);
	mMediaCfg = new MediaCfg(this);
	mConvert = new Convert(this);
	mFileBrowser = new FileBrowser(this);
	mMessagePopup = new GenericMessage(this);
	mInfoPopup = new InfoMessage(this);
	mProfilePopup = new ProfileChooser(this);
	mInputCfg = new InputCfg(this);
	mPaletteCfg = new PaletteCfg(this);
	mMediaCreate = new MediaCreate(this);
	mLog = new Log(this);
	mLauncher = new RomLauncher(this);
	mLauncherOptions = new RomLauncherOptions(this);
	mSoundCfg = new SoundCfg(this);
	mUserInterfaceCfg = new UserInterfaceCfg(this);
	mWizard = new Wizard(this);
	mThrottle = new Throttle(this);
	mVideoCfg = new VideoCfg(this);

	mMenu = new KatMenu(this);

	/*
	 * Now create all windows and widgets
	 */
	for ( i = 0; i < MAX_CONTROLLER; ++i ) {
		mTop->add(&mSysJack[i]);
	}
	mTop->add(mThrottle);
	mTop->add(mBiosCfg);
	mTop->add(mMediaCfg);
	mTop->add(mConvert);
	mTop->add(mLauncher);
	mTop->add(mLauncherOptions);
	mTop->add(mVideoCfg);
	mTop->add(mSoundCfg);
	mTop->add(mInputCfg);
	mTop->add(mPaletteCfg);
	mTop->add(mWizard);
	mTop->add(mUserInterfaceCfg);
	mTop->add(mMediaCreate);
	mTop->add(mLog);
	mTop->add(mFileBrowser);
	mTop->add(mMessagePopup);
	mTop->add(mInfoPopup);
	mTop->add(mProfilePopup);

	mTop->add(&mRomDec);
	mTop->add(&mRomTitle);
	mTop->add(&mRomFilename);
	mTop->add(&mRomCRC);
	mTop->add(&mRomType);
	mTop->add(&mSysDec);
	mTop->add(&mSysMachine);
	mTop->add(&mSysVideo);

	mMenu->setContainer (mTop);

	mSysDec.setDimension ( getRectangle(2,5,75,9) );
	mSysMachine.setDimension ( getRectangle(2.5,6.5,75,1) );
	mSysVideo.setDimension ( getRectangle(2.5,7.5,75,1) );
	for ( i = 0; i < MAX_CONTROLLER; ++i )
		mSysJack[i].setDimension ( getRectangle(2.5,8.5+i,75,1) );
	setSysInfo();

	mRomDec.setDimension ( getRectangle(2,15,75,6) );
	mRomTitle.setDimension ( getRectangle(2.5,16.5,75,1) );
	mRomFilename.setDimension ( getRectangle(2.5,17.5,75,1) );
	mRomCRC.setDimension ( getRectangle(2.5,18.5,75,1) );
	mRomType.setDimension ( getRectangle(2.5,19.5,75,1) );
	setRomInfo();

	mMenu->checkSaveStateExistance();

	if ( p_config->start_gui_with_launcher )
		mLauncher->show();

	if ( config_get_first_use() ) 
	{
		strcpy ( string, "It appears this is the first use of kat5200.  You may use" );
		strcat ( string, " this wizard to perform configuration or press Cancel to exit" );
		showWizard ( string );
	}
	else 
	{
		showError ( error );
	}

	mDebugMode = false;
	mIgnoreUIKeys = false;
	if ( debug_flag == 1 )
   	{
		mMessagePopup->deleteActions();
		mMessagePopup->show( "CPU Crashed.  Enter Debugger?","Yes",true,"No",true,"",false, false );
		mMessagePopup->addActionListener ( this );
	}
}

GuiMain::~GuiMain()
{
	delete mThrottle;
	delete mBiosCfg;
	delete mMediaCfg;
	delete mConvert;
	delete mLauncher;
	delete mLauncherOptions;
	delete mVideoCfg;
	delete mSoundCfg;
	delete mInputCfg;
	delete mPaletteCfg;
	delete mWizard;
	delete mUserInterfaceCfg;
	delete mMediaCreate;
	delete mLog;
	delete mMenu;

	/*
	 * Delete top level stuff
	 */
	delete mFont;
	delete mTop;
	delete mGui;
	
	delete mInput;
	delete mGraphics;
	delete mImageLoader;
}

gcn::Rectangle GuiMain::getRectangle ( double x, double y, double w, double h )
{
	gcn::Rectangle NewRect;

	NewRect.x = (int)(x * (double)mFont->getWidth("A"));
	NewRect.y = (int)(y * (double)mFont->getHeight());
	NewRect.width = (int)(w * (double)mFont->getWidth("A"));
	NewRect.height = (int)(h * (double)mFont->getHeight());

	return NewRect;
}

gcn::Rectangle GuiMain::getPosition ( double x, double y )
{
	gcn::Rectangle NewRect;

	NewRect.x = (int)(x * (double)mFont->getWidth("A"));
	NewRect.y = (int)(y * (double)mFont->getHeight());

	return NewRect;
}

void GuiMain::setRomInfo ( void )
{
	char tmp1_string[50];
	std::string tmpString;

	mRomTitle.setCaption ( (tmpString="Title: ") + media_get_ptr()->title );
	mRomFilename.setCaption ( (tmpString="File : ") + media_get_ptr()->filename );

	sprintf ( tmp1_string, "%x", media_get_ptr()->crc );
	mRomCRC.setCaption ( (tmpString="CRC  : ") + tmp1_string );

	mRomType.setCaption ( (tmpString="Type : ") + cart_get_mapping_string(media_get_ptr()->mapping,tmp1_string) );

	mRomTitle.adjustSize();
	mRomFilename.adjustSize();
	mRomCRC.adjustSize();
	mRomType.adjustSize();
}

void GuiMain::setSysInfo ( void )
{
	int i;
	char tmp_string[50];

	switch ( config_get_ptr()->machine_type ) {
		case MACHINE_TYPE_5200: 
			mSysMachine.setCaption ( "Machine: Atari 5200" );
			break;
		case MACHINE_TYPE_800: 
			mSysMachine.setCaption ( "Machine: Atari 800" );
			break;
		case MACHINE_TYPE_XL: 
			mSysMachine.setCaption ( "Machine: Atari 800 XL/XE" );
			break;
		default: 
			mSysMachine.setCaption ( "Machine: Unknown" );
			break;
	}

	if ( config_get_ptr()->system_type == NTSC )
		mSysVideo.setCaption ( "Video  : NTSC" );
	else
		mSysVideo.setCaption ( "Video  : PAL" );

	for ( i = 0; i < MAX_CONTROLLER; ++i ) {
		switch ( g_input.players[i].control_type ) {
			case CTRLR_TYPE_JOYSTICK: 
				sprintf ( tmp_string, "Port %d : %s", i+1, "Joystick" );
				break;
			case CTRLR_TYPE_TRACKBALL: 
				sprintf ( tmp_string, "Port %d : %s", i+1, "Trackball" );
				break;
			case CTRLR_TYPE_PADDLES: 
				sprintf ( tmp_string, "Port %d : %s", i+1, "Paddles" );
				break;
			case CTRLR_TYPE_KEYPAD: 
				sprintf ( tmp_string, "Port %d : %s", i+1, "Keypad" );
				break;
			default:
				sprintf ( tmp_string, "Port %d : %s", i+1, "None" );
				break;
		}
		mSysJack[i].setCaption ( tmp_string );
		mSysJack[i].adjustSize();
	}

	mSysMachine.adjustSize();
	mSysVideo.adjustSize();
}

void GuiMain::launch( int state_flag, int load_flag )
{
	t_media tmp_cart;
	t_config *p_config;
	e_cart_type tmp_mapping;
	int status;
	int ram_size = 64;
	struct atari *p_console;

	mStateLoad = false;

	/*
	 * Get Configuration and machine status
	 */
	config_save();
	p_config = config_get_ptr ();
	p_console = console_get_ptr ();
	gtia_set_mode_2_draw_ptr ( );	

	/*
	 * Check for state file
	 */
	if ( state_flag ) {
		input_set_profile ( media_get_ptr()->input_profile );
		video_set_profile ( media_get_ptr()->video_profile );
		sound_set_profile ( media_get_ptr()->sound_profile );
		ui_set_profile( p_config->default_ui_profile );
		mem_assign_ops ( p_console->cpu );

		mStateLoad = true;
		mRunning = false;
		mExit = false;
		return;
	}

	/*
	 * Check for new ROM or mapping
	 */
	tmp_cart.crc = util_get_crc(p_config->rom_file);
	media_get_settings ( &tmp_cart );

	if ( load_flag ) {
		if ( status=media_open_rom (p_config->rom_file, CART_MIN, 0, 1) ) {
			showError ( status );
			return;
		}
	}

	else if ( (tmp_cart.crc != media_get_ptr()->crc) || (tmp_cart.crc == 0) ||
	     ((tmp_cart.mapping != media_get_ptr()->mapping) && tmp_cart.mapping != CART_MIN) ||
	     (tmp_cart.ram_size != media_get_ptr()->ram_size) ||
	     (tmp_cart.machine_type != media_get_ptr()->machine_type) ||
		 (p_config->machine_type != console_get_machine_type() ) ) {
		if ( status=media_open_rom (p_config->rom_file, CART_MIN, 0, 1) ) {
			showError ( status );
			return;
		}
		if ( status=load_os ( config_get_os_file (), p_config->machine_type ) ) {
			showError ( status );
			return;
		}

		input_set_profile ( media_get_ptr()->input_profile );
		video_set_profile ( media_get_ptr()->video_profile );
		sound_set_profile ( media_get_ptr()->sound_profile );
		ui_set_profile( p_config->default_ui_profile );

		if ( media_get_ptr()->ram_size )
			ram_size = media_get_ptr()->ram_size;
		else
			ram_size = p_config->ram_size[media_get_ptr()->machine_type];
		console_reset( p_config->machine_type, p_config->system_type, ram_size, g_sound.freq );
	}

	/*
	 * Else check for new profiles
	 */
	else {
		tmp_mapping = media_get_ptr()->mapping;
		media_get_settings ( media_get_ptr() );
		media_get_ptr()->mapping = tmp_mapping;
		input_set_profile ( media_get_ptr()->input_profile );
		video_set_profile ( media_get_ptr()->video_profile );
		sound_set_profile ( media_get_ptr()->sound_profile );
		ui_set_profile( p_config->default_ui_profile );
	}

	/*
	 * Debugger Entry?
	 * FIXME: Do I handle that here?
	 */

	mRunning = false;
	mExit = false;
}

void GuiMain::showError( int error )
{
	if ( !error )
		return;

	mInfoPopup->deleteActions();
	switch ( error )
	{
		case ERR_ROM_LOAD: 
			mInfoPopup->show( "Error Loading ROM Image" );
			break;
		case ERR_BIOS_LOAD: 
			mInfoPopup->show( "Error Loading BIOS Image" );
			break;
		case ERR_BASIC_LOAD: 
			mInfoPopup->show( "Error Loading BASIC Image" );
			break;
		case ERR_PROFILE_LOAD: 
			mInfoPopup->show( "Error Loading Profile" );
			break;
	}
}

void GuiMain::exit( )
{
	mStateLoad = false;
	mDebugMode = false;
	mRunning = false;
	mExit = true;
}

bool GuiMain::getExit( )
{
	return mExit;
}

bool GuiMain::getStateLoad( )
{
	return mStateLoad;
}

bool GuiMain::getDebugMode( )
{
	return mDebugMode;
}

FileBrowser * GuiMain::getFileBrowser( )
{
	return mFileBrowser;
}

GenericMessage * GuiMain::getMessagePopup( )
{
	return mMessagePopup;
}

InfoMessage * GuiMain::getInfoPopup( )
{
	return mInfoPopup;
}

ProfileChooser * GuiMain::getProfilePopup( )
{
	return mProfilePopup;
}

gcn::GuiWindow * GuiMain::getAutoWindow( )
{
	return &mAutoWindow;
}

gcn::KatGui * GuiMain::getKatGui( )
{
	return mGui;
}

void GuiMain::setAutoMessage( std::string msg )
{
	mAutoMessage.setCaption ( msg );
	mAutoMessage.adjustSize();
}

void GuiMain::showAutoWindow( std::string caption, std::string msg )
{
	mIgnoreUIKeys = true;

	mAutoMessage.setCaption ( msg );
	mAutoMessage.adjustSize();

	mAutoWindow.setFocusable ( true );
	mAutoWindow.setCaption ( caption );
	mAutoWindow.requestModalFocus();
	mAutoWindow.setVisible(true);
	mAutoWindow.requestFocus();
	mTop->moveToTop(&mAutoWindow);
}

void GuiMain::hideAutoWindow()
{
	mIgnoreUIKeys = false;

	mAutoWindow.releaseModalFocus();
	mAutoWindow.setVisible(false);
	mTop->moveToBottom(&mAutoWindow);
}

void GuiMain::showThrottle ()
{
	mThrottle->show();
}

void GuiMain::showBiosCfg ()
{
	mBiosCfg->show();
}

void GuiMain::showMediaCfg ()
{
	mMediaCfg->show();
}

void GuiMain::showConvert ( int type )
{
	mConvert->show( type );
}

void GuiMain::showLauncher ()
{
	mLauncher->show();
}

void GuiMain::showLauncherOptions ()
{
	mLauncherOptions->show();
}

void GuiMain::showVideoCfg ()
{
	mVideoCfg->show();
}

void GuiMain::showSoundCfg ()
{
	mSoundCfg->show();
}

void GuiMain::showInputCfg ()
{
	mInputCfg->show();
}

void GuiMain::showPaletteCfg ()
{
	mPaletteCfg->show();
}

void GuiMain::showWizard ( std::string message )
{
	mWizard->show( message );
}

void GuiMain::showUserInterfaceCfg ()
{
	mUserInterfaceCfg->show();
}

void GuiMain::showMediaCreate ()
{
	mMediaCreate->show();
}

void GuiMain::showLog ()
{
	mLog->show();
}

unsigned long GuiMain::getLauncherSelected ()
{
	return mLauncher->getSelectedCRC();
}

void GuiMain::updateLauncherList ()
{
	mLauncher->changeRomGroup( mLauncher->getSelectedGroup() );
}

void GuiMain::Message ( int message, void *data )
{
	struct guimessage msg;

	msg.message = message;
	msg.data = data;

	mMessageQueue.push ( msg );
}

void GuiMain::run()
{
	SDL_Event event;
	bool mJoystickCheck = false;
	struct guimessage msg;
	char dir[1100];
	int *data;
	int total;
	int new_total;

	mRunning = true;
	mExit = false;
	mStateLoad = false;

	while ( mRunning ) 
	{
		if ( mJoystickCheck == true ) 
		{
			new_total = pc_detect_joysticks();
			if ( new_total != total ) 
			{
				mWizard->updateInputList();
				total = new_total;
			}
		}

		while ( !mMessageQueue.empty() )
		{
			msg = mMessageQueue.front();	
			mMessageQueue.pop();

			switch ( msg.message )
			{
				case MSG_SCAN:
					data = (int *)msg.data;
					strcpy ( dir, getFileBrowser()->getResult().c_str() );
					media_scan_directory ( (e_machine_type)data[0], dir, data[1], data[2] );
					mMessagePopup->hide();
					updateLauncherList();
					break;
				case MSG_SCAN_WIZARD:
					mWizard->scanImages();
					mMessagePopup->hide();
					break;
				case MSG_JOY_CHECK:
					mJoystickCheck = true;
					total = *((int *)msg.data);
					break;
				case MSG_NO_JOY_CHECK:
					mJoystickCheck = false;
					break;
				case MSG_JOY_CHECK_ONCE:
					total = pc_detect_and_open_joysticks();
					break;
			}
		}

		while ( SDL_PollEvent ( &event ) ) 
		{
			if (event.type == SDL_QUIT)
			{
				this->exit();
			}
			mInput->pushInput ( event );
		}

	    mGui->logic();
		mGui->draw();

		SDL_Flip(mScreen);

		if ( SDL_WaitEvent ( &event ) ) 
		{
			if (event.type == SDL_QUIT)
			{
				this->exit();
			}
			mInput->pushInput ( event );
		}
	}
}

void GuiMain::action(const gcn::ActionEvent &actionEvent)
{
	GenericMessage *pop = (GenericMessage *) actionEvent.getSource();

	if ( pop && pop->getButton() == 0 ) {
		pop->hide();
		mDebugMode = true;
		launch(0,0);
	}
	else if ( pop ) {
		pop->hide();
	}
}

void GuiMain::joyEvent(gcn::JoystickEvent& ji)
{
	e_part_type tmp_part_type;
	e_direction tmp_direction;
	t_config *p_config = config_get_ptr();
	int i;

	switch ( ji.getType() ) {
		case gcn::JoystickInput::AXIS:
			if ( abs(ji.getValue()) < 16000 )
				return;
			if ( ji.getValue() < 0 )
				tmp_direction = DIR_MINUS;
			else
				tmp_direction = DIR_PLUS;
			tmp_part_type = PART_TYPE_AXIS;
			break;
		case gcn::JoystickInput::BALL:
			if ( ji.getValue() < 0 )
				tmp_direction = DIR_MINUS;
			else
				tmp_direction = DIR_PLUS;
			tmp_part_type = PART_TYPE_BALL;
			break;
		case gcn::JoystickInput::BUTTON:
			tmp_part_type = PART_TYPE_BUTTON;
			if ( ji.getValue() == gcn::JoystickEvent::PRESS )
				tmp_direction = DIR_PRESS;
			else
				tmp_direction = DIR_RELEASE;
			break;
		case gcn::JoystickInput::HAT:
			if ( ji.getValue() & SDL_HAT_LEFT )
				tmp_direction = DIR_LEFT;
			else if ( ji.getValue() & SDL_HAT_RIGHT )
				tmp_direction = DIR_RIGHT;
			else if ( ji.getValue() & SDL_HAT_UP )
				tmp_direction = DIR_UP;
			else if ( ji.getValue() & SDL_HAT_DOWN )
				tmp_direction = DIR_DOWN;
			tmp_part_type = PART_TYPE_HAT;
			break;
	}

	for ( i = 0; i < UI_KEY_MAX; ++i )
	{
		if ( g_ui.keys[i].device == DEV_JOYSTICK &&
		     g_ui.keys[i].device_num == ji.getDevice() &&
		     g_ui.keys[i].part_type == tmp_part_type &&
		     g_ui.keys[i].part_num == ji.getPart() &&
		     g_ui.keys[i].direction == tmp_direction )
		{
			performUIAction ( i );
		}
	}
}

void GuiMain::keyPressed (gcn::KeyEvent &keyEvent)
{
	if ( mIgnoreUIKeys == false )
		checkUIKey ( keyEvent, DIR_PRESS );
}

void GuiMain::keyReleased (gcn::KeyEvent &keyEvent)
{
	if ( mIgnoreUIKeys == false )
		checkUIKey ( keyEvent, DIR_RELEASE );
}

void GuiMain::checkUIKey (gcn::KeyEvent &keyEvent, e_direction direction)
{
	int i,tmp_dev_num;
	t_config *p_config = config_get_ptr();

	if ( keyEvent.isAltPressed() == true )
		tmp_dev_num = KMOD_ALT;
	else if ( keyEvent.isControlPressed() == true )
		tmp_dev_num = KMOD_CTRL;
	else if ( keyEvent.isShiftPressed() == true )
		tmp_dev_num = KMOD_SHIFT;
	else if ( keyEvent.isMetaPressed() == true )
		tmp_dev_num = KMOD_META;
	else
		tmp_dev_num = 0;

	for ( i = 0; i < UI_KEY_MAX; ++i )
	{
		if ( g_ui.keys[i].device == DEV_KEYBOARD &&
		     g_ui.keys[i].device_num == tmp_dev_num &&
		     g_ui.keys[i].part_num == convertKeyCharacter(keyEvent) &&
		     g_ui.keys[i].direction == direction )
		{
			performUIAction ( i );
		}
	}
}

void GuiMain::performUIAction ( int ui_key )
{
	switch ( ui_key )
	{
		case UI_KEY_EXIT:
			this->exit();
		break;
		case UI_KEY_BACK:
			launch( 0, 0 );
		break;
		case UI_KEY_FULLSCREEN:
			mScreen = pc_toggle_gui_fullscreen ( mScreen );
			mGraphics->setTarget(mScreen);
		break;

	} /* end switch key action */
}

int GuiMain::convertKeyCharacter(gcn::KeyEvent &keyEvent)
{
    int value = 0;

    if ( keyEvent.isNumericPad() == true )
    {
		if ( (keyEvent.getKey().getValue() >= '1') && (keyEvent.getKey().getValue() <= '9') )
			return SDLK_KP0 + (keyEvent.getKey().getValue() - '1');
		if ( keyEvent.getKey().getValue() == '.' )
			return SDLK_KP_PERIOD;
		if ( keyEvent.getKey().getValue() == '/' )
			return SDLK_KP_DIVIDE;
		if ( keyEvent.getKey().getValue() == '*' )
			return SDLK_KP_MULTIPLY;
		if ( keyEvent.getKey().getValue() == '-' )
			return SDLK_KP_MINUS;
		if ( keyEvent.getKey().getValue() == '+' )
			return SDLK_KP_PLUS;
		if ( keyEvent.getKey().getValue() == '=' )
			return SDLK_KP_EQUALS;
		if ( keyEvent.getKey().getValue() == gcn::Key::ENTER )
			return SDLK_KP_ENTER;

		switch ( keyEvent.getKey().getValue() )
		{
              case gcn::Key::INSERT:
                  return SDLK_KP0;
                  break;
              case gcn::Key::END:
                  return SDLK_KP1;
                  break;
              case gcn::Key::DOWN:
                  return SDLK_KP2;
                  break;
              case gcn::Key::PAGE_DOWN:
                  return SDLK_KP3;
                  break;
              case gcn::Key::LEFT:
                  return SDLK_KP4;
                  break;
              case 0:
                  return SDLK_KP5;
                  break;
              case gcn::Key::RIGHT:
                  return SDLK_KP6;
                  break;
              case gcn::Key::HOME:
                  return SDLK_KP7;
                  break;
              case gcn::Key::UP:
                  return SDLK_KP8;
                  break;
              case gcn::Key::PAGE_UP:
                  return SDLK_KP9;
                  break;
		}
    }

    switch (keyEvent.getKey().getValue())
    {
	  case gcn::Key::TAB:
          return SDLK_TAB;
          break;
	  case gcn::Key::LEFT_ALT:
      	  return SDLK_LALT;
          break;
	  case gcn::Key::RIGHT_ALT:
          return SDLK_RALT;
          break;
      case gcn::Key::LEFT_SHIFT:
          return SDLK_LSHIFT;
          break;
      case gcn::Key::RIGHT_SHIFT:
          return SDLK_RSHIFT;
          break;
      case gcn::Key::LEFT_CONTROL:
          return SDLK_LCTRL;
          break;
      case gcn::Key::RIGHT_CONTROL:
          return SDLK_RCTRL;
          break;
      case gcn::Key::BACKSPACE:
          return SDLK_BACKSPACE;
          break;
      case gcn::Key::PAUSE:
          return SDLK_PAUSE;
          break;
      case gcn::Key::SPACE:
          return SDLK_SPACE;
          break;
      case gcn::Key::ESCAPE:
          return SDLK_ESCAPE;
          break;
      case gcn::Key::DELETE:
          return SDLK_DELETE;
          break;
      case gcn::Key::INSERT:
          return SDLK_INSERT;
          break;
      case gcn::Key::HOME:
          return SDLK_HOME;
          break;
      case gcn::Key::END:
          return SDLK_END;
          break;
      case gcn::Key::PAGE_UP:
          return SDLK_PAGEUP;
          break;
      case gcn::Key::PRINT_SCREEN:
          return SDLK_PRINT;
          break;
      case gcn::Key::PAGE_DOWN:
          return SDLK_PAGEDOWN;
          break;
      case gcn::Key::F1:
          return SDLK_F1;
          break;
      case gcn::Key::F2:
          return SDLK_F2;
          break;
      case gcn::Key::F3:
          return SDLK_F3;
          break;
      case gcn::Key::F4:
          return SDLK_F4;
          break;
      case gcn::Key::F5:
          return SDLK_F5;
          break;
      case gcn::Key::F6:
          return SDLK_F6;
          break;
      case gcn::Key::F7:
          return SDLK_F7;
          break;
      case gcn::Key::F8:
          return SDLK_F8;
          break;
      case gcn::Key::F9:
          return SDLK_F9;
          break;
      case gcn::Key::F10:
          return SDLK_F10;
          break;
      case gcn::Key::F11:
          return SDLK_F11;
          break;
      case gcn::Key::F12:
          return SDLK_F12;
          break;
      case gcn::Key::F13:
          return SDLK_F13;
          break;
      case gcn::Key::F14:
          return SDLK_F14;
          break;
      case gcn::Key::F15:
          return SDLK_F15;
          break;
      case gcn::Key::NUM_LOCK:
          return SDLK_NUMLOCK;
          break;
      case gcn::Key::CAPS_LOCK:
          return SDLK_CAPSLOCK;
          break;
      case gcn::Key::SCROLL_LOCK:
          return SDLK_SCROLLOCK;
          break;
      case gcn::Key::RIGHT_META:
          return SDLK_RMETA;
          break;
      case gcn::Key::LEFT_META:
          return SDLK_LMETA;
          break;
      case gcn::Key::LEFT_SUPER:
          return SDLK_LSUPER;
          break;
      case gcn::Key::RIGHT_SUPER:
          return SDLK_RSUPER;
          break;
      case gcn::Key::ALT_GR:
          return SDLK_MODE;
          break;
      case gcn::Key::UP:
          return SDLK_UP;
          break;
      case gcn::Key::DOWN:
          return SDLK_DOWN;
          break;
      case gcn::Key::LEFT:
          return SDLK_LEFT;
          break;
      case gcn::Key::RIGHT:
          return SDLK_RIGHT;
          break;
      case gcn::Key::ENTER:
          return SDLK_RETURN;
          break;
    }

	return keyEvent.getKey().getValue();
}
