/******************************************************************************
*
* FILENAME: guimain.hpp
*
* DESCRIPTION:  This is the main class where everything is created and run
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/02/06  bberlin      Creation
******************************************************************************/
#ifndef guimain_hpp
#define guimain_hpp

#include <SDL/SDL.h>
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "katgui.hpp"
#include "sdlguiinput.hpp"
#include "decoration.hpp"
#include "filebrowser.hpp"
#include "genericmessage.hpp"
#include "infomessage.hpp"
#include "profilechooser.hpp"
#include "throttle.hpp"
#include "bioscfg.hpp"
#include "mediacfg.hpp"
#include "convert.hpp"
#include "romlauncher.hpp"
#include "romlauncheroptions.hpp"
#include "videocfg.hpp"
#include "soundcfg.hpp"
#include "inputcfg.hpp"
#include "palettecfg.hpp"
#include "userinterfacecfg.hpp"
#include "mediacreate.hpp"
#include "log.hpp"
#include "katmenu.hpp"
#include "window.hpp"
#include "wizard.hpp"

struct guimessage {
	int message;
	void *data;
};

class GuiMain : public gcn::KeyListener,
                public gcn::JoystickListener,	
                public gcn::ActionListener
    {
    public:
		GuiMain(int error, int debug_flag);
		~GuiMain();
		gcn::Rectangle getRectangle ( double x, double y, double w, double h );
		gcn::Rectangle getPosition ( double x, double y );
		void launch( int state_flag, int load_flag );
		void showError( int error );
		void exit();
		bool getExit();
		bool getStateLoad();
		bool getDebugMode();
		void run();
		FileBrowser * getFileBrowser();
		GenericMessage * getMessagePopup();
		InfoMessage * getInfoPopup();
		ProfileChooser * getProfilePopup();
		gcn::GuiWindow * getAutoWindow();
		gcn::KatGui * getKatGui();
		void setAutoMessage( std::string msg );
		void showAutoWindow( std::string caption, std::string msg );
		void hideAutoWindow();
		void showThrottle();
		void showBiosCfg();
		void showMediaCfg();
		void showConvert( int type );
		void showLauncher();
		void showLauncherOptions();
		void showVideoCfg();
		void showSoundCfg();
		void showInputCfg();
		void showPaletteCfg();
		void showWizard( std::string message );
		void showUserInterfaceCfg();
		void showMediaCreate();
		void showLog();
		void Message ( int message, void *data );
		unsigned long getLauncherSelected();
		void updateLauncherList ();
		int convertKeyCharacter(gcn::KeyEvent &keyEvent);
		void performUIAction ( int ui_key );

		void action(const gcn::ActionEvent &actionEvent);
		void joyEvent(gcn::JoystickEvent& ji);
		void keyPressed (gcn::KeyEvent &keyEvent);
		void keyReleased (gcn::KeyEvent &keyEvent);

		enum {
			MSG_SCAN = 0,
			MSG_SCAN_WIZARD,
			MSG_NO_SCAN,
			MSG_JOY_CHECK,
			MSG_NO_JOY_CHECK,
			MSG_JOY_CHECK_ONCE
		};
    private:
		void setRomInfo ( void );
		void setSysInfo ( void );
		void checkUIKey (gcn::KeyEvent &keyEvent, e_direction direction);
		bool mIgnoreUIKeys;
		bool mRunning;
		bool mExit;
		bool mStateLoad;
		bool mDebugMode;

		SDL_Surface* mScreen;
		gcn::SDLGraphics* mGraphics;
		//gcn::SDLInput* mInput;
		gcn::SDLGuiInput* mInput;
		gcn::SDLImageLoader* mImageLoader;
		//gcn::Gui* mGui;
		gcn::KatGui* mGui;
		gcn::GuiWindow mAutoWindow;
		gcn::Label mAutoMessage;

		gcn::Container *mTop;

		FileBrowser *mFileBrowser;
		GenericMessage *mMessagePopup;
		InfoMessage *mInfoPopup;
		ProfileChooser *mProfilePopup;

		Throttle *mThrottle;
		BiosCfg *mBiosCfg;
		MediaCfg *mMediaCfg;
		Convert *mConvert;
		RomLauncher *mLauncher;
		RomLauncherOptions *mLauncherOptions;
		VideoCfg *mVideoCfg;
		SoundCfg *mSoundCfg;
		InputCfg *mInputCfg;
		PaletteCfg *mPaletteCfg;
		Wizard *mWizard;
		UserInterfaceCfg *mUserInterfaceCfg;
		MediaCreate *mMediaCreate;
		Log *mLog;
		KatMenu *mMenu;

		Decoration mRomDec;
		gcn::Label mRomTitle;
		gcn::Label mRomFilename;
		gcn::Label mRomCRC;
		gcn::Label mRomType;

		Decoration mSysDec;
		gcn::Label mSysMachine;
		gcn::Label mSysVideo;
		gcn::Label mSysJack[MAX_CONTROLLER];

		gcn::ImageFont *mFont;

		std::queue<struct guimessage> mMessageQueue;
    };

#endif
