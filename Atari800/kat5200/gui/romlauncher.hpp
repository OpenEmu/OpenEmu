/******************************************************************************
*
* FILENAME: romlauncher.hpp
*
* DESCRIPTION:  This class will show a window containing the list of roms
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   10/03/06  bberlin      Creation
******************************************************************************/
#ifndef romlauncher_hpp
#define romlauncher_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "decoration.hpp"
#include "guiobject.hpp"
#include "window.hpp"
#include "tabbedwindow.hpp"
#include "romlaunchermenu.hpp"

extern "C" {
	#include "../interface/media.h"
}

class RomLauncherListener : public gcn::ActionListener,
                            public GuiObject
{
private:

public:
	void action(const gcn::ActionEvent &actionEvent);
};

class RomLauncherTab : public gcn::Window,
                       public GuiObject
{
	public:
		RomLauncherTab ( GuiMain *gMain, std::string caption, gcn::ActionListener *al, 
		                       gcn::KeyListener *kl );
		gcn::ListBox mRomBox;
		gcn::ScrollArea mRomScroller;
		GenericList mRomList;
};

class RomLauncher : public gcn::GuiWindow,
                    public GuiObject,
                    public gcn::ActionListener,
					public gcn::KeyListener
{
	public:
		RomLauncher( GuiMain *gMain );
		~RomLauncher();

		void show (); 
		void hide ();

		void initRomList ( RomLauncherTab *pTab, char *sClause );
		void initGroupControls ();
		void changeTabView ();
		void changeRomGroup ( unsigned long value );
		unsigned long getSelectedCRC();
		int getSelectedGroup();
		void changeRom ( int index );
		void setMachine ( e_machine_type );
		void launch ( int load_flag );
		void load ( e_cart_type type, int slot );
		void actOnFavorites ();

		void action(const gcn::ActionEvent &actionEvent);
		void mousePressed (gcn::MouseEvent &mouseEvent);
		void keyPressed (gcn::KeyEvent &keyEvent);

		RomLauncherListener mRomListener;

	private:
		RomLauncherMenu mMenu;

		TabbedWindow mRomWindow;

		gcn::Button mOk;
		gcn::Button mCancel;

		TabbedWindow mImageWindow;
		gcn::Window mImageNameTab;
		gcn::Window mImageFileTab;
		gcn::Window mImageProfilesTab;
		gcn::Window mImageSettingsTab;

		gcn::Label mCRC;
		gcn::Label mMD5;
		gcn::Label mSHA1;
		gcn::Label mRomSize;
		gcn::Label mRomType;
		gcn::DropDown mMachine;
		gcn::TextField mFile;
		gcn::TextField mTitle;

		gcn::DropDown mMapping;
		gcn::DropDown mRam;
		gcn::DropDown mBasic;
		gcn::DropDown mSIO;

		gcn::Label mDefaultLabel;
		gcn::DropDown  mInputBox;
		gcn::DropDown  mVideoBox;
		gcn::DropDown  mSoundBox;
		GenericList    mInputList;
		GenericList    mVideoList;
		GenericList    mSoundList;
		gcn::CheckBox  mInputDefault;
		gcn::CheckBox  mVideoDefault;
		gcn::CheckBox  mSoundDefault;
		gcn::Button mProfileOk;
		gcn::Button mProfileCancel;

		GenericList mMappingList;
		GenericList mMachineList;
		GenericList mRamList;
		GenericList mBasicList;
		GenericList mSIOList;

		Decoration mDetailsDec;

		gcn::Label mCRCLabel;
		gcn::Label mMD5Label;
		gcn::Label mSHA1Label;
		gcn::Label mRomSizeLabel;
		gcn::Label mRomTypeLabel;
		gcn::Label mMachineLabel;
		gcn::Label mMachineFavLabel;
		gcn::Label mMappingLabel;
		gcn::Label mRamLabel;
		gcn::Label mBasicLabel;
		gcn::Label mSIOLabel;
		gcn::Label mInputLabel;
		gcn::Label mVideoLabel;
		gcn::Label mSoundLabel;

		std::vector<RomLauncherTab *> mTabList;
		typedef std::vector<RomLauncherTab *>::iterator TabIterator;

		unsigned long mCurrentCRC;
		unsigned long mCurrentGroup;
		t_media mCurrentMedia;
		bool mRomDirty;

		clock_t mClickTime;
		clock_t mKeyTime;
		char mKeySave[4];

		int mKeyLevel;

		void saveRom();
};

#endif
