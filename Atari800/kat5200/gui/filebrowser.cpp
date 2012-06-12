/******************************************************************************
*
* FILENAME: filebrowser.cpp
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
#include "filebrowser.hpp"
#include "guimain.hpp"

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#include <time.h>
#endif
#include <dirent.h>
#include <sys/stat.h>

FileBrowser::FileBrowser ( GuiMain *gMain ) : GuiObject(gMain)
{

	mCurrentIndex = 0;
	mShowDirOnly = false;
	mShowOverwrite = true;
	setVisible ( false );

	/*
	 * Create all of our widgets
	 */
	mOk.setCaption ( "Ok" );
	mCancel.setCaption ( "Cancel" );
	mUp.setCaption ( "Up" );
	mFileList.setSortMethod ( GLIST_SORT_FILE_ALPHA );
	mDirListBox.setListModel ( &mFileList );
	mListScroller.setContent ( &mDirListBox );

	/*
	 * Create Callbacks and assign
	 */
	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mUp.addActionListener ( this );

	mFile.addActionListener ( this );
	mDir.addActionListener ( this );
	mDirListBox.addActionListener ( this );
	mDirListBox.addKeyListener ( this );

	mOk.setActionEventId ( "Ok" );
	mCancel.setActionEventId ( "Cancel" );
	mUp.setActionEventId ( "Up" );

	mFile.setActionEventId ( "File" );
	mDir.setActionEventId ( "Dir" );
	mDirListBox.setActionEventId ( "List" );

	/*
	 * Size and place our widgets
	 */
	add ( &mOk );
	add ( &mCancel );
	add ( &mUp );
	add ( &mFile);
	add ( &mDir );
	add ( &mListScroller );

	setDimension ( mGuiMain->getRectangle(5,2,74,26) );
	mOk.setDimension ( mGuiMain->getRectangle(25,23.0,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(45,23.0,6,1) );
	mUp.setDimension ( mGuiMain->getRectangle(67,1,3,1) );
	mFile.setDimension ( mGuiMain->getRectangle(4,21,60,1) );
	mDir.setDimension ( mGuiMain->getRectangle(4,1,60,1) );
	mDirListBox.setDimension ( mGuiMain->getRectangle(0,0,60,16.5) );
	mListScroller.setDimension ( mGuiMain->getRectangle(4,3,60,17) );

	mOk.adjustSize();
	mCancel.adjustSize();
	mUp.adjustSize();

	mFile.adjustHeight();
	mDir.adjustHeight();

	mKeyTime = clock();
	mKeyLevel = 0;
	mKeySearchType = 0;
}

FileBrowser::~FileBrowser ()
{
}

void FileBrowser::show ( std::string title, std::string dir, int type )
{
	std::string selected_file;
	std::string::size_type index = 0;
	char *directory;
	struct stat file_status;

	mShowDirOnly = false;
	mCancelPressed = true;
	setCaption ( title );

	mFile.setVisible ( true );

	switch ( type ) {
		case LOAD:
			mOk.setCaption ( "Load" );
			break;
		case SAVE:
			mOk.setCaption ( "Save" );
			break;
		case DIR_ONLY:
			mOk.setCaption ( "Select" );
			mShowDirOnly = true;
			mFile.setVisible ( false );
			break;
	}

	mOk.adjustSize();
	mType = type;

	/*
	 * Check if this is a relative directory
	 */
	if ( (dir.c_str()[0] != '/') && (dir.c_str()[1] != ':') ) {
		directory = (char *)malloc ( sizeof(char)*1100 );
		getcwd ( directory, 1000 );
		selected_file = dir;
		//dir = directory + getSeperatorChar() + selected_file;
		dir = directory;
		dir += getSeperatorChar();
		dir += selected_file;
		free ( directory );
	}

	/*
	 * Check if directory is a file,
	 *   and if so truncate it
	 */
	selected_file = "";
	if ( !stat ( dir.c_str(), &file_status ) ) {
		if ( !(file_status.st_mode & S_IFDIR) ) {
			index = dir.rfind ( '\\' );
			if ( index == std::string::npos )
				index = dir.rfind ( '/' );
			if ( index != std::string::npos ) {
				if ( index+1 != std::string::npos ) {
					selected_file = dir.substr ( index+1 );
				}
				dir.erase ( index );
			}
		}
	}

	/*
	 * If file doesn't exist, try to get last directory
	 */
	else {
		index = dir.rfind ( '\\' );
		if ( index == std::string::npos )
			index = dir.rfind ( '/' );
		if ( index != std::string::npos ) {
			dir.erase ( index );
		}
	}

	/*
	 * Fill in List and show
	 */
	fillFileList ( dir, "" );
	mDirListBox.adjustSize();
	requestModalFocus ();
	setPosition ( mGuiMain->getPosition(3,2).x, mGuiMain->getPosition(3,2).y );
	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);

	/*
	 * Set Current File (if entered)
	 */
	if ( selected_file.size() ) {
		mDirListBox.setSelected ( mFileList.getIndexFromLabel(selected_file.c_str()));
	}
}

void FileBrowser::hide ( )
{
	releaseModalFocus ();
	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void FileBrowser::setCancelPressed ( bool pressed )
{
	mCancelPressed = pressed;
}

bool FileBrowser::getCancelPressed ( )
{
	return mCancelPressed;
}

void FileBrowser::setShowOverwrite ( bool show )
{
	mShowOverwrite = show;
}

void FileBrowser::setResult ( std::string result )
{
	mResult = result;
}

std::string FileBrowser::getResult (  )
{
	return mResult;
}

void FileBrowser::setOkText ( std::string text )
{
	mOk.setCaption ( text );
}

void FileBrowser::setDirOnly ( bool setting )
{
	mShowDirOnly = setting;
}

void FileBrowser::deleteActions ( )
{
	mActionListeners.clear();
}

void FileBrowser::fillFileList ( std::string dir, std::string substring )
{
	DIR *pdir;
	struct dirent *entry;
	int value;
	struct stat file_status;
	char current_dir[1001];
	std::string new_file;

	/* 
	 * Check for directory existance and use current if can't open
	 */
	pdir = opendir ( dir.c_str() );
	if ( !pdir ) {
		pdir = opendir ( mDirText.c_str() );
		dir = mDirText;
	}
	if ( !pdir ) {
		pdir = opendir ( getcwd( current_dir, 1000 ) );
		dir = current_dir;
	}

	mDirText = dir;
	
	mDir.setText ( mDirText );
	mFile.setText ( "" );
	mFileList.deleteAll();

	/*
	 * Go through the entries and put then in list indicating file or 
	 * directory for each one
	 */
	while ( entry=readdir(pdir) ) {
		new_file = dir + getSeperatorChar() + entry->d_name;
		stat ( new_file.c_str(), &file_status );
		if ( file_status.st_mode & S_IFDIR ) {
			value = 1;
			strcat ( entry->d_name,"/" );
			mFileList.addElement(entry->d_name, value, 0);
		}
		else if ( mShowDirOnly == false ) {
			if ( substring.empty()  || mType !=FileBrowser::LOAD ||
			     ((mType == FileBrowser::LOAD) && 
			      strstr(entry->d_name, substring.c_str())) ) {
				value = 0;
				mFileList.addElement(entry->d_name, value, 0);
			}
		}

	} /* end while entry exists in directory list */

	closedir ( pdir );
}

char FileBrowser::getSeperatorChar ( ) {

	char *filename;
	char tmp;

	filename = (char *)malloc ( sizeof(char)*1024 );

	getcwd ( filename, 1023 );

	if ( strrchr(filename,'\\') )
			tmp = '\\';
	else
			tmp = '/';

	free ( filename );

	return tmp;

} /* end getSeperatorChar */

void FileBrowser::action(const gcn::ActionEvent &actionEvent)
{
	std::string tmp_string;
	int index, new_index;
	struct stat file_status;

	/*
	 * Ok, Load, Save Button
	 */
	if ( actionEvent.getId() == "Ok" ) {
		if ( mShowDirOnly ) {
			setResult ( mDirText );
		}
		else {
			if ( mType == FileBrowser::LOAD ) {
				tmp_string = mFileList.getElementAt(mDirListBox.getSelected()).c_str();
				if ( !strstr(tmp_string.c_str(), "/") )
					setResult ( mDirText + getSeperatorChar() + tmp_string );
				else {
					tmp_string.erase ( tmp_string.end()-1 );
					fillFileList ( mDirText + getSeperatorChar() + tmp_string, "" );
				}
			}
			else {
				setResult ( mDirText + getSeperatorChar() + mFile.getText() );
				if ( !stat ( getResult().c_str(), &file_status ) && mShowOverwrite ) {
					releaseModalFocus();
					mGuiMain->getMessagePopup()->deleteActions();
					mGuiMain->getMessagePopup()->setActionEventId("Overwrite");
					mGuiMain->getMessagePopup()->addActionListener(this);
					mGuiMain->getMessagePopup()->show( "This will overwrite existing file. Are you sure?","Yes",true,"No",true,"",false, false );
					return;
				}
			}

		} /* end else LOAD or SAVE box */

		setCancelPressed ( false );
		distributeActionEvent();
		hide ( );

	} /* end if Ok button */

	/*
	 * Cancel button
	 */
	else if ( actionEvent.getId() == "Cancel" ) {
		setCancelPressed ( true );
		distributeActionEvent();
		hide ( );
	}

	/*
	 * Up button
	 */
	else if ( actionEvent.getId() == "Up" ) {
		tmp_string = mDirText;
		if ( (index = (int)tmp_string.rfind ( '\\' )) != std::string::npos ) {
			if ( (index > 0) && *(tmp_string.end()-1) != ':' && *(tmp_string.end()-2) != ':' ) {
				tmp_string.erase ( index );
				if ( *(tmp_string.end()-1) == ':' )
					tmp_string += '\\';
			}
		}
		else if ( (index = (int)tmp_string.rfind ( '/' )) != std::string::npos ) {
			if ( index > 0 ) {
				new_index = (int)tmp_string.rfind ( '/', index-1 );
				if ( new_index != std::string::npos )
					tmp_string.erase ( index );
			}
			else {
				tmp_string.erase ( index + 1 );
			}
		}

		fillFileList ( tmp_string, "" );
	}

	/*
	 * Enter pressed in File box
	 */
	else if ( actionEvent.getId() == "File" ) {
		if ( mType == FileBrowser::LOAD ) {
			tmp_string = mFile.getText();
			fillFileList ( mDirText, tmp_string );
		}
		else {
			setResult ( mDirText + getSeperatorChar() + mFile.getText() );
			if ( !stat ( getResult().c_str(), &file_status ) && mShowOverwrite ) {
				releaseModalFocus();
				mGuiMain->getMessagePopup()->deleteActions();
				mGuiMain->getMessagePopup()->setActionEventId("Overwrite");
				mGuiMain->getMessagePopup()->addActionListener(this);
				mGuiMain->getMessagePopup()->show( "This will overwrite existing file. Are you sure?","Yes",true,"No",true,"",false, false );
				return;
			}
			setCancelPressed ( false );
			distributeActionEvent();
			hide ( );
		}
	}

	/*
	 * Enter pressed in Dir box
	 */
	else if ( actionEvent.getId() == "Dir" ) {
		mDirText = mDir.getText ();
		tmp_string = mDirText;
		if ( *(tmp_string.end()-1) == '\\' ) {
			if ( tmp_string.size() > 2 ) {
				if ( *(tmp_string.end()-2) != ':' ) {
					tmp_string.erase ( tmp_string.end()-1 );
				}
			}
			else {
				tmp_string = mDirText;
			}
		}
		else if ( *(tmp_string.end()-1) == '/' ) {
			if ( tmp_string.size() > 1 )
				tmp_string.erase ( tmp_string.end()-1 );
		}
		else if ( *(tmp_string.end()-1) == ':' ) {
			tmp_string += '\\';
		}

		fillFileList ( tmp_string, "" );
	}

	/*
	 * Click or Double-click in the File List
	 */
	else if ( actionEvent.getId() == "List" ) {

		/*
		 * Double Click, select the file and say goodbye
		 *   or if directory, change to that directory
		 */
		if ( ((clock() - mClickTime) < (CLOCKS_PER_SEC*0.7)) && mCurrentIndex == mDirListBox.getSelected() ) {
			mClickTime = 0;
			tmp_string = mFileList.getElementAt(mDirListBox.getSelected()).c_str();
			if ( strstr(tmp_string.c_str(), "/") ) {
				tmp_string.erase ( tmp_string.end()-1 );

				if ( tmp_string == ".." ) {
					tmp_string = mDirText;
					if ( (index = (int)tmp_string.rfind ( '\\' )) != std::string::npos ) {
						if ( (index > 0) && *(tmp_string.end()-1) != ':' && *(tmp_string.end()-2) != ':' ) {
							tmp_string.erase ( index );
							if ( *(tmp_string.end()-1) == ':' )
								tmp_string += '\\';
						}
					}
					else if ( (index = (int)tmp_string.rfind ( '/' )) != std::string::npos ) {
						if ( index > 0 ) {
							new_index = (int)tmp_string.rfind ( '/', index-1 );
							if ( new_index != std::string::npos )
								tmp_string.erase ( index );
						}
						else {
							tmp_string.erase ( index + 1 );
						}
					}
					fillFileList ( tmp_string, "" );
				}
				else if ( tmp_string != "." ) {
					if ( *(mDirText.end()-1) == '\\' ||
					     *(mDirText.end()-1) == '/' )
						setResult ( mDirText + tmp_string );
					else
						setResult ( mDirText + getSeperatorChar() + tmp_string );
					fillFileList ( getResult(), "" );
				}
			}
			else {
				if ( *(mDirText.end()-1) == '\\' ||
				     *(mDirText.end()-1) == '/' )
					setResult ( mDirText + tmp_string );
				else
					setResult ( mDirText + getSeperatorChar() + tmp_string );

				/*
				 * If Saving, check for file existance
				 */
				if ( mType == FileBrowser::SAVE ) {
					if ( !stat ( getResult().c_str(), &file_status ) && mShowOverwrite ) {
						releaseModalFocus();
						mGuiMain->getMessagePopup()->deleteActions();
						mGuiMain->getMessagePopup()->setActionEventId("Overwrite");
						mGuiMain->getMessagePopup()->addActionListener(this);
						mGuiMain->getMessagePopup()->show( "This will overwrite existing file. Are you sure?","Yes",true,"No",true,"",false, false );
						return;
					}
				}

				setCancelPressed ( false );
				distributeActionEvent();
				hide ( );
			}
		}

		/*
		 * Single Click, put in file box (save only)
		 */
		else {
			mCurrentIndex = mDirListBox.getSelected();
			if ( !mShowDirOnly && (mType == FileBrowser::SAVE) ) {
				tmp_string = mFileList.getElementAt(mDirListBox.getSelected()).c_str();
				if ( !strstr(tmp_string.c_str(), "/") )
					mFile.setText ( tmp_string );
			}
			mClickTime = clock();
		}

	} /* end if List is clicked */

	/*
	 * User attempted to write over existing file
	 */
	else if ( actionEvent.getId() == "Overwrite" ) {

		mGuiMain->getMessagePopup()->hide();
		if ( mGuiMain->getMessagePopup()->getButton() == 0 ) {
			setCancelPressed ( false );
			distributeActionEvent();
			hide ( );
		}
		else {
			requestModalFocus();
		}

	} /* end if Overwrite attempted */

} /* end action */

void FileBrowser::keyPressed (gcn::KeyEvent &keyEvent)
{
	int i,j;
	int start;

	/*
	 * Key Level depends on time from last key press
	 */
	if ( clock() - mKeyTime < (CLOCKS_PER_SEC*1.0) ) {
		mKeyLevel++;
		start = mDirListBox.getSelected();
	}
	else {
		mKeyLevel = 0;
		start = 0;
	}

	/*
	 * Determine if this is a directory or file search on initial press
	 */
	if ( !mKeyLevel ) {
		if ( strstr(mFileList.getLabelFromIndex(mDirListBox.getSelected()),"/") )
			mKeySearchType = 1;
		else
			mKeySearchType = 0;
	}

	mKeyTime = clock();

	if ( mKeyLevel > 3 )
		mKeyLevel = 3;

	mKeySave[mKeyLevel] = keyEvent.getKey().getValue();

	/*
	 * First check for previous characters matching
	 *   If not don't change position in list
	 */
	for ( j = 0; j < mKeyLevel; ++j ) {
		if ( (int)strlen(mFileList.getLabelFromIndex(start)) > j ) {
			if ( tolower(mFileList.getLabelFromIndex(start)[j]) != 
			     tolower(mKeySave[j]) )
			return;
		}
	}

	/*
	 * Now do a search
	 */ 
	if ( keyEvent.getKey().isCharacter() )
	{
		for ( i = start; i < mFileList.getNumberOfElements(); ++i )
		{

			if ( mFileList.getValueFromIndex(i) != mKeySearchType )
				continue;

			/*
			 * First check for previous characters matching
			 *   If not don't change position in list
			 */
			for ( j = 0; j < mKeyLevel; ++j ) {
				if ( (int)strlen(mFileList.getLabelFromIndex(i)) > j ) {
					if ( tolower(mFileList.getLabelFromIndex(i)[j]) != 
					     tolower(mKeySave[j]) )
					return;
				}
			}

			if ( (int)strlen(mFileList.getLabelFromIndex(i)) > mKeyLevel ) {
				if ( tolower(mFileList.getLabelFromIndex(i)[mKeyLevel]) >= 
				     tolower(keyEvent.getKey().getValue()) )
				{
					mDirListBox.setSelected(i);
					break;
				}
			}
		}
	}
}

void FileBrowser::keyReleased (gcn::KeyEvent &keyEvent)
{

}
