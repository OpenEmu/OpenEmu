/******************************************************************************
*
* FILENAME: filebrowser.hpp
*
* DESCRIPTION:  This class will show a pop-up for loading / saving files
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   09/22/06  bberlin      Creation
******************************************************************************/
#ifndef filebrowser_hpp
#define filebrowser_hpp

#include <guichan.hpp>
#include "genericlist.hpp"
#include "guiobject.hpp"
#include "window.hpp"

class FileBrowser : public gcn::GuiWindow,
                    public GuiObject,
                    public gcn::ActionListener,
					public gcn::KeyListener
{
	public:
		FileBrowser( GuiMain *gMain );
		~FileBrowser();

		void show ( std::string title, std::string dir, int type );
		void hide ( );
		void setOkText ( std::string text );
		void setCancelPressed ( bool pressed );
		bool getCancelPressed ( );
		void setShowOverwrite ( bool show );
		void setDirOnly ( bool setting );
		void setResult ( std::string result );
		std::string getResult ( );
		void deleteActions ( );
		void fillFileList ( std::string dir, std::string substring );
		char getSeperatorChar ( );
		void action(const gcn::ActionEvent &actionEvent);
		void keyPressed (gcn::KeyEvent &keyEvent);
		void keyReleased (gcn::KeyEvent &keyEvent);

        enum
        {
            EMPTY = 0,
            LOAD,
            SAVE,
            DIR_ONLY,
            SELECT,
            CANCEL
        };

	private:
		std::string mDirText;
		std::string mResult;
		gcn::Button mOk;
		gcn::Button mCancel;
		gcn::Button mUp;
		gcn::TextField mFile;
		gcn::TextField mDir;
		gcn::ListBox mDirListBox;
		gcn::ScrollArea mListScroller;
		GenericList mFileList;
		bool mShowDirOnly;
		bool mCancelPressed;
		bool mShowOverwrite;
		int mType;
		unsigned long mClickTime;
		unsigned long mKeyTime;
		int mKeyLevel;
		int mKeySearchType;
		int mCurrentIndex;
		char mKeySave[4];
};

#endif
