/******************************************************************************
*
* FILENAME: romlauncher.cpp
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
#include "romlauncher.hpp"
#include "guimain.hpp"

extern "C" {
	#include "../interface/kconfig.h"
	#include "../interface/media.h"
	#include "../interface/util.h"
	#include "../interface/db_if.h"
}

RomLauncher::RomLauncher ( GuiMain *gMain ) : GuiObject(gMain)
{
	t_config *p_config = config_get_ptr();

	setCaption ( "Launcher" );
	setVisible ( false );

	mClickTime = clock();
	mKeyTime = clock();
	mKeyLevel = 0;

	/*
	 * Setup all of our widgets
	 */
	mMenu.setLauncher ( this );

	mOk.setCaption ("Ok");
	mCancel.setCaption ("Cancel");

	mInputBox.setListModel ( &mInputList );
	mVideoBox.setListModel ( &mVideoList );
	mSoundBox.setListModel ( &mSoundList );

	mMapping.setListModel (&mMappingList);
	mMachine.setListModel (&mMachineList);
	mRam.setListModel (&mRamList);
	mBasic.setListModel (&mBasicList);
	mSIO.setListModel (&mSIOList);

	mDetailsDec.setCaption ( "Image Details" );

	mImageNameTab.setCaption ( "Image" );
	mImageFileTab.setCaption ( "File" );
	mImageProfilesTab.setCaption ( "Profiles" );
	mImageSettingsTab.setCaption ( "Settings" );

	mCRCLabel.setCaption ("CRC:");
	mMD5Label.setCaption ("MD5:");
	mSHA1Label.setCaption ("SHA1:");
	mRomSizeLabel.setCaption ("Size:");
	mRomTypeLabel.setCaption ("Type:");
	mMachineLabel.setCaption ("Machine");
	mMachineFavLabel.setCaption ("Machine");
	mMappingLabel.setCaption ("Mapping");
	mRamLabel.setCaption ("RAM");
	mBasicLabel.setCaption ("BASIC");
	mSIOLabel.setCaption ("SIO Patch");
	mInputLabel.setCaption ("Input");
	mVideoLabel.setCaption ("Video");
	mSoundLabel.setCaption ("Sound");
	mDefaultLabel.setCaption ("Use Default");

	/*
	 * Create Callbacks and assign
	 */
	addActionListener ( this );

	mOk.addActionListener ( this );
	mCancel.addActionListener ( this );
	mMapping.addActionListener ( this );
	mMachine.addActionListener ( this );
	mRam.addActionListener ( this );
	mBasic.addActionListener ( this );
	mSIO.addActionListener ( this );

	mInputBox.addActionListener ( this );
	mVideoBox.addActionListener ( this );
	mSoundBox.addActionListener ( this );
	mInputDefault.addActionListener ( this );
	mVideoDefault.addActionListener ( this );
	mSoundDefault.addActionListener ( this );

	mOk.setActionEventId ( "mOk" );
	mCancel.setActionEventId ( "mCancel" );
	mMapping.setActionEventId ( "mDirty" );
	mMachine.setActionEventId ( "mMachine" );
	mRam.setActionEventId ( "mDirty" );
	mBasic.setActionEventId ( "mDirty" );
	mSIO.setActionEventId ( "mDirty" );
	mInputBox.setActionEventId ( "mDirty" );
	mVideoBox.setActionEventId ( "mDirty" );
	mSoundBox.setActionEventId ( "mDirty" );
	mInputDefault.setActionEventId ( "mInputDefault" );
	mVideoDefault.setActionEventId ( "mVideoDefault" );
	mSoundDefault.setActionEventId ( "mSoundDefault" );

	/*
	 * Size and place our widgets
	 */
	add ( &mRomWindow );
	add ( &mDetailsDec );

	add ( &mOk );
	add ( &mCancel );

	add ( &mImageWindow );
	mImageWindow.addTab ( 0, &mImageNameTab );
	mImageWindow.addTab ( 0, &mImageFileTab );
	mImageWindow.addTab ( 0, &mImageProfilesTab );
	mImageWindow.addTab ( 0, &mImageSettingsTab );

	mImageFileTab.add ( &mCRC );
	mImageFileTab.add ( &mMD5 );
	mImageFileTab.add ( &mSHA1 );
	mImageFileTab.add ( &mRomSize );
	mImageFileTab.add ( &mFile );
	mImageNameTab.add ( &mRomType );
	mImageNameTab.add ( &mMachine );
	mImageNameTab.add ( &mTitle );

	mImageSettingsTab.add ( &mMapping );
	mImageSettingsTab.add ( &mRam );
	mImageSettingsTab.add ( &mBasic );
	mImageSettingsTab.add ( &mSIO );

	mImageProfilesTab.add ( &mInputBox );
	mImageProfilesTab.add ( &mVideoBox );
	mImageProfilesTab.add ( &mSoundBox );
	mImageProfilesTab.add ( &mInputDefault );
	mImageProfilesTab.add ( &mVideoDefault );
	mImageProfilesTab.add ( &mSoundDefault );
	mImageProfilesTab.add ( &mDefaultLabel );
	mImageProfilesTab.add ( &mInputLabel );
	mImageProfilesTab.add ( &mVideoLabel );
	mImageProfilesTab.add ( &mSoundLabel );

	mImageFileTab.add ( &mCRCLabel );
	mImageFileTab.add ( &mMD5Label );
	mImageFileTab.add ( &mSHA1Label );
	mImageFileTab.add ( &mRomSizeLabel );
	mImageNameTab.add ( &mRomTypeLabel );
	mImageNameTab.add ( &mMachineLabel );
	mImageNameTab.add ( &mMachineFavLabel );
	mImageSettingsTab.add ( &mMappingLabel );
	mImageSettingsTab.add ( &mRamLabel );
	mImageSettingsTab.add ( &mBasicLabel );
	mImageSettingsTab.add ( &mSIOLabel );

	mMenu.setContainer(this);

	setDimension ( mGuiMain->getRectangle(2,2,74,26) );

	mRomWindow.setDimension ( mGuiMain->getRectangle(0.8,2.0,71,14.2) );
	mOk.setDimension ( mGuiMain->getRectangle(30,23.2,3,1) );
	mCancel.setDimension ( mGuiMain->getRectangle(40,23.2,6,1) );

	mImageWindow.setDimension ( mGuiMain->getRectangle(0.5,16.4,72,6.6) );
	mImageNameTab.setDimension ( mGuiMain->getRectangle(-1,-1,73,7.8) );
	mImageFileTab.setDimension ( mGuiMain->getRectangle(-1,-1,73,7.8) );
	mImageProfilesTab.setDimension ( mGuiMain->getRectangle(-1,-1,73,7.8) );
	mImageSettingsTab.setDimension ( mGuiMain->getRectangle(-1,-1,73,7.8) );

	mRomTypeLabel.setDimension ( mGuiMain->getRectangle(1,3.8,6,1) );
	mRomType.setDimension ( mGuiMain->getRectangle(8.5,3.8,10,1) );
	mMachineLabel.setDimension ( mGuiMain->getRectangle(1,2.4,6,1) );
	mMachineFavLabel.setDimension ( mGuiMain->getRectangle(1,2.4,6,1) );
	mMachine.setDimension ( mGuiMain->getRectangle(8.5,2.4,16.5,1) );
	mTitle.setDimension ( mGuiMain->getRectangle(1.5,0.5,70,1) );

	mFile.setDimension ( mGuiMain->getRectangle(1.5,0.5,70,1) );
	mCRCLabel.setDimension ( mGuiMain->getRectangle(18,2.0,4,1) );
	mCRC.setDimension ( mGuiMain->getRectangle(23,2.0,10,1) );
	mMD5Label.setDimension ( mGuiMain->getRectangle(1,3.3,4,1) );
	mMD5.setDimension ( mGuiMain->getRectangle(6,3.3,40,1) );
	mSHA1Label.setDimension ( mGuiMain->getRectangle(1,4.6,10,1) );
	mSHA1.setDimension ( mGuiMain->getRectangle(6,4.6,50,1) );
	mRomSizeLabel.setDimension ( mGuiMain->getRectangle(1,2.0,6,1) );
	mRomSize.setDimension ( mGuiMain->getRectangle(6,2.0,10,1) );

	mMappingLabel.setDimension ( mGuiMain->getRectangle(1,1.0,10,1) );
	mMapping.setDimension ( mGuiMain->getRectangle(9,1.0,20,1) );
	mRamLabel.setDimension ( mGuiMain->getRectangle(32,1.0,4,1) );
	mRam.setDimension ( mGuiMain->getRectangle(41,1.0,9,1) );
	mBasicLabel.setDimension ( mGuiMain->getRectangle(1,3.4,8,1) );
	mBasic.setDimension ( mGuiMain->getRectangle(9,3.4,9,1) );
	mSIOLabel.setDimension ( mGuiMain->getRectangle(32,3.4,8,1) );
	mSIO.setDimension ( mGuiMain->getRectangle(41,3.4,9,1) );

	mDefaultLabel.setDimension ( mGuiMain->getRectangle(57.5,0.4,6,1) );
	mInputLabel.setDimension ( mGuiMain->getRectangle(1,1.7,10,1) );
	mInputBox.setDimension ( mGuiMain->getRectangle(7,1.7,50,1) );
	mInputDefault.setDimension ( mGuiMain->getRectangle(64,1.7,2,1) );
	mVideoLabel.setDimension ( mGuiMain->getRectangle(1,3.1,10,1) );
	mVideoBox.setDimension ( mGuiMain->getRectangle(7,3.1,50,1) );
	mVideoDefault.setDimension ( mGuiMain->getRectangle(64,3.1,2,1) );
	mSoundLabel.setDimension ( mGuiMain->getRectangle(1,4.5,10,1) );
	mSoundBox.setDimension ( mGuiMain->getRectangle(7,4.5,50,1) );
	mSoundDefault.setDimension ( mGuiMain->getRectangle(64,4.5,2,1) );

	mOk.adjustSize();
	mCancel.adjustSize();
	mFile.adjustHeight();
	mTitle.adjustHeight();
	mMapping.adjustHeight();
	mRam.adjustHeight();
	mBasic.adjustHeight();
	mSIO.adjustHeight();
	mInputBox.adjustHeight();
	mVideoBox.adjustHeight();
	mSoundBox.adjustHeight();
	mInputDefault.adjustSize();
	mVideoDefault.adjustSize();
	mSoundDefault.adjustSize();

	mCRCLabel.adjustSize();
	mRomSizeLabel.adjustSize();
	mRomTypeLabel.adjustSize();
	mMachineLabel.adjustSize();
	mMachineFavLabel.adjustSize();
	mMappingLabel.adjustSize();
	mRamLabel.adjustSize();
	mBasicLabel.adjustSize();
	mSIOLabel.adjustSize();
	mInputLabel.adjustSize();
	mVideoLabel.adjustSize();
	mSoundLabel.adjustSize();
	mDefaultLabel.adjustSize();

	/*
	 * Initialize the list
	 */
	mMachineList.deleteAll();
	mMachineList.addElement ( "Atari 5200", MACHINE_TYPE_5200, 0 );
	mMachineList.addElement ( "Auto 8-bit", MACHINE_TYPE_MIN, 0 );
	mMachineList.addElement ( "Atari 800", MACHINE_TYPE_800, 0 );
	mMachineList.addElement ( "Atari 800 XL/XE", MACHINE_TYPE_XL, 0 );

	mRamList.deleteAll();
	mRamList.addElement ( "Auto", 0, 0 );
	mRamList.addElement ( "16 KB", 16, 0 );
	mRamList.addElement ( "48 KB", 48, 0 );
	mRamList.addElement ( "52 KB", 52, 0 );
	mRamList.addElement ( "64 KB", 64, 0 );
	mRamList.addElement ( "128 KB", 128, 0 );

	mBasicList.deleteAll();
	mBasicList.addElement ( "Auto", 2, 0 );
	mBasicList.addElement ( "Enable", 1, 0 );
	mBasicList.addElement ( "Disable", 0, 0 );

	mSIOList.deleteAll();
	mSIOList.addElement ( "Auto", 2, 0 );
	mSIOList.addElement ( "Enable", 1, 0 );
	mSIOList.addElement ( "Disable", 0, 0 );

	mCurrentGroup = 0x1;
	initGroupControls();
}

RomLauncher::~RomLauncher ()
{
	/*
	 * Delete current tabs
	 */
	TabIterator it;

	mRomWindow.removeAll();
	for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   	{
		delete (*it);
	}
	mTabList.clear();
}

void RomLauncher::show ( )
{
	t_config *p_config = config_get_ptr();

	mRomDirty = false;
	mCurrentCRC = util_get_crc(p_config->rom_file);

	mCurrentGroup = p_config->launcher_group_view;
	initGroupControls();
	changeRomGroup(mCurrentGroup);

	mRomListener.setGuiMain ( mGuiMain );
	mMenu.setGuiMain ( mGuiMain );

	setVisible ( true );
	if ( getParent () )
		getParent()->moveToTop (this);
}

void RomLauncher::hide ( )
{
	t_config *p_config = config_get_ptr();

	if ( mCurrentGroup != p_config->launcher_group_view )
	{
		p_config->launcher_group_view = mCurrentGroup; 
		config_save();
	}

	setVisible ( false );
	if ( getParent () )
		getParent()->moveToBottom (this);
}

void RomLauncher::initRomList ( RomLauncherTab *pTab, char *sClause )
{
	struct generic_node *rom_all_list;
	char sql[257];
	char machine[30];
	char where_1[20];
	char where_2[20];
	char s_and[10];
	t_config *p_config = config_get_ptr();

	pTab->mRomList.deleteAll();

	if ( strlen(sClause) )
		strcpy ( s_and, "AND " );
	else
		strcpy ( s_and, "" );

	if ( p_config->machine_type == MACHINE_TYPE_5200 )
		sprintf ( machine, "%s(MachineID = %d)", s_and, MACHINE_TYPE_5200 );
	else
		sprintf ( machine, "%s(MachineID <> %d)", s_and, MACHINE_TYPE_5200 );

	if ( p_config->launcher_show_all )
		strcpy ( machine, "" );

	if ( strlen(machine) || strlen(sClause) )
		strcpy ( where_1, "WHERE " );
	else
		strcpy ( where_1, "" );

	if ( !strlen(where_1) )
		strcpy ( where_2, "WHERE ");
	else
		strcpy ( where_2, "AND ");

	rom_all_list = 0;
	switch ( mCurrentGroup )
	{
		case 0x1:
			sprintf ( sql, "SELECT Title,CRC FROM Media %s%s %s ORDER BY 1 ASC", where_1, sClause, machine );
			break;
		case 0x2:
			sprintf ( sql, "SELECT Title,CRC FROM Media %s%s %s %s(Filename != '' AND Filename NOTNULL) ORDER BY 1 ASC", where_1, sClause, machine, where_2 );
			break;
		case 0x4:
			sprintf ( sql, "SELECT Title,CRC FROM Media %s%s %s %s(Filename = '' OR Filename ISNULL) ORDER BY 1 ASC", where_1, sClause, machine, where_2 );
			break;
		case 0x8:
			sprintf ( sql, "SELECT Title,CRC FROM Media WHERE Favorite <> 0  %s%s ORDER BY 1 ASC", s_and, sClause );
			break;
	}
	db_if_get_profile_names ( sql, &rom_all_list );
	pTab->mRomList.setList ( rom_all_list );

}

void RomLauncher::initGroupControls ( )
{
	struct generic_node *tmp_list;
	t_config *p_config = config_get_ptr();

	mInputList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Input", &tmp_list );
	mInputList.setList ( tmp_list );

	mVideoList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Video", &tmp_list );
	mVideoList.setList ( tmp_list );

	mSoundList.deleteAll();
	db_if_get_profile_names ( "SELECT Name FROM Sound", &tmp_list );
	mSoundList.setList ( tmp_list );
}

void RomLauncher::changeTabView ()
{
	RomLauncherTab *pTab, *pPrevTab;
	struct generic_node *types_list, *prev_list;
	char sql[256];
	int i, items, lpp, row;
	char letter;
	GenericList typesList;
	t_config *p_config = config_get_ptr();

	/*
	 * Delete current tabs
	 */
	TabIterator it;

	mRomWindow.removeAll();
	for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   	{
		delete (*it);
	}
	mTabList.clear();

	db_if_exec_sql ( "BEGIN", 0, 0 );

	/*
	 * Start adding tabs based on settings
	 */
	switch ( p_config->launcher_tabbed_view )
	{
		case 0:           /* No Tab */
			pTab = new RomLauncherTab(mGuiMain,"Image List",this,this);
			mRomWindow.addTab(0,pTab);
			mTabList.push_back ( pTab );
			initRomList ( pTab, "" );
			break;
		case 1:           /* TOSEC */
			sprintf ( sql, "SELECT TosecType,TosecTypeID FROM TosecTypes" );
			db_if_get_profile_names ( sql, &types_list );
			typesList.setSortMethod ( GLIST_SORT_NONE );
			typesList.setList ( types_list );
			row = -1;
			for ( i = 0; i < typesList.getNumberOfElements(); ++i ) 
			{
				if ( !(i%3) )
					++row;
				util_search_and_replace ( typesList.getLabelFromIndex(i), "Atari 8bit", "8bit" );
				util_search_and_replace ( typesList.getLabelFromIndex(i), "Atari 5200", "A5200" );
				util_search_and_replace ( typesList.getLabelFromIndex(i), " - ", "-" );
				pTab = new RomLauncherTab(mGuiMain,typesList.getLabelFromIndex(i),this,this);
				mRomWindow.addTab(row,pTab);
				mTabList.push_back ( pTab );
				sprintf ( sql, "(Flags = %d)", typesList.getValueFromIndex(i) );
				initRomList ( pTab, sql );
			}
			for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   			{
				(*it)->mRomScroller.setDimension ( mGuiMain->getRectangle(1,0.5,70,12.4-row) );
				(*it)->mRomBox.setDimension ( mGuiMain->getRectangle(0,2.0,71,13.7-row) );
			}
			typesList.deleteAll();

			break;
		case 2:           /* Media Type */
			pTab = new RomLauncherTab(mGuiMain,"Carts",this,this);
			mRomWindow.addTab(0,pTab);
			mTabList.push_back ( pTab );
			sprintf ( sql, "MediaType = %d", CART_CART );
			initRomList ( pTab, sql );

			pTab = new RomLauncherTab(mGuiMain,"Floppies",this,this);
			mRomWindow.addTab(0,pTab);
			mTabList.push_back ( pTab );
			sprintf ( sql, "MediaType = %d", CART_FLOPPY );
			initRomList ( pTab, sql );

			pTab = new RomLauncherTab(mGuiMain,"Cassettes",this,this);
			mRomWindow.addTab(0,pTab);
			mTabList.push_back ( pTab );
			sprintf ( sql, "MediaType = %d", CART_CASSETTE );
			initRomList ( pTab, sql );

			pTab = new RomLauncherTab(mGuiMain,"Executables",this,this);
			mRomWindow.addTab(0,pTab);
			mTabList.push_back ( pTab );
			sprintf ( sql, "MediaType = %d", CART_EXE );
			initRomList ( pTab, sql );
			break;
		case 3:           /* Alpha */
			pTab = new RomLauncherTab(mGuiMain,"all",this,this);
			mRomWindow.addTab(0,pTab);
			mTabList.push_back ( pTab );
			initRomList ( pTab, "" );
			pPrevTab = pTab;
	
			items = pTab->mRomList.getNumberOfElements();
			if ( items / 1000 > 1 ) {
				pTab->setCaption ( "0-9" );
				lpp = 26 / (items/1000);	
				for ( letter = 'a'; letter <= 'z'; letter += lpp )
				{
					if ( letter + lpp - 1 > 'z' )
						sprintf ( sql, "%c-z", letter );
					else
						sprintf ( sql, "%c-%c", letter, letter + lpp -1 );
					pTab = new RomLauncherTab(mGuiMain,sql,this,this);
					mRomWindow.addTab(0,pTab);
					mTabList.push_back ( pTab );

					types_list = pPrevTab->mRomList.getList();
					prev_list = 0;
					while ( types_list )
					{
						if ( tolower(types_list->name[0]) >= letter )
						{
							pTab->mRomList.setList ( types_list );	
							if ( prev_list )
								prev_list->next = 0;
							break;
						}
						prev_list = types_list;
						types_list = types_list->next;
					}
					pPrevTab = pTab;
				}
				
				/*
				 * After splitting up the list, recalculate elements
				 */
				for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   				{
					(*it)->mRomList.getNumberOfItems();
				}

			}
			break;
	}

	db_if_exec_sql ( "END", 0, 0 );
}

void RomLauncher::changeRomGroup ( unsigned long value )
{
	int i;
	bool display_attributes;
	bool rom_found;

	if ( value == 0x8 )
	{
		display_attributes = false;
		mMachine.setVisible ( false );
		mMachineLabel.setVisible ( false );
		mMachineFavLabel.setVisible ( true );
		mMenu.setItemCaption ( "mRemove", "Remove From Favorites" );
	}
	else
	{
		display_attributes = true;
		mMachine.setVisible ( true );
		mMachineLabel.setVisible ( true );
		mMachineFavLabel.setVisible ( false );
		mMenu.setItemCaption ( "mRemove", "Add To Favorites" );
	}

	/*
	 * Now we either display or hide attributes based on if 
	 *   this is the favorites list
	 */
	mMapping.setVisible ( display_attributes );
	mRam.setVisible ( display_attributes );
	mInputBox.setVisible ( display_attributes );
	mVideoBox.setVisible ( display_attributes );
	mSoundBox.setVisible ( display_attributes );
	mDefaultLabel.setVisible ( display_attributes );
	mInputDefault.setVisible ( display_attributes );
	mVideoDefault.setVisible ( display_attributes );
	mSoundDefault.setVisible ( display_attributes );
	mMappingLabel.setVisible ( display_attributes );
	mRamLabel.setVisible ( display_attributes );
	mInputLabel.setVisible ( display_attributes );
	mVideoLabel.setVisible ( display_attributes );
	mSoundLabel.setVisible ( display_attributes );

	mCurrentGroup = value;

	TabIterator it;

	rom_found = false;

	changeTabView ();

	switch ( value ) {
		case 0x1: mMenu.setGroupChecked ( "mAll" ); break;
		case 0x2: mMenu.setGroupChecked ( "mAvailable" ); break;
		case 0x4: mMenu.setGroupChecked ( "mUnAvailable" ); break;
		case 0x8: mMenu.setGroupChecked ( "mFavorites" ); break;
	}

	for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   	{
		(*it)->mRomBox.setListModel ( &((*it)->mRomList) );
		(*it)->mRomBox.adjustSize();
		(*it)->mRomBox.setSelected(0);

		if ( !rom_found )
		{
			for ( i = 0; i < (*it)->mRomList.getNumberOfElements(); ++i ) {
				if ( mCurrentCRC == (*it)->mRomList.getValueFromIndex(i) ) {
					(*it)->mRomBox.setSelected(i);
					rom_found = true;
					mRomWindow.selectTab ( (*it) );
					changeRom ( i );
					break;
				}
			}
		}

	} /* end for tab list */

	if ( !rom_found )
	{
		changeRom ( 0 );
	}
}

unsigned long RomLauncher::getSelectedCRC()
{
	return mCurrentCRC;
}

int RomLauncher::getSelectedGroup()
{
	return mCurrentGroup;
}

void RomLauncher::saveRom ( )
{
	t_config *p_config = config_get_ptr();

	mCurrentMedia.crc = mCurrentCRC;
	strcpy ( mCurrentMedia.title, mTitle.getText().c_str() );
	strcpy ( mCurrentMedia.filename, mFile.getText().c_str() );
	strcpy ( mCurrentMedia.md5, mMD5.getCaption().c_str() );
	strcpy ( mCurrentMedia.sha1, mSHA1.getCaption().c_str() );
	mCurrentMedia.image_size = atoi ( mRomSize.getCaption().c_str() );
	mCurrentMedia.mapping = (e_cart_type) (mMappingList.getValueFromIndex(mMapping.getSelected()));
	mCurrentMedia.machine_type = (e_machine_type) (mMachineList.getValueFromIndex(mMachine.getSelected()));
	mCurrentMedia.ram_size = mRamList.getValueFromIndex(mRam.getSelected());
	mCurrentMedia.basic_enable = mBasicList.getValueFromIndex(mBasic.getSelected());
	mCurrentMedia.sio_patch_enable = mSIOList.getValueFromIndex(mSIO.getSelected());

	// FIXME:  Actually set this one day
	mCurrentMedia.write_protect = 1;

	if ( mInputDefault.isSelected() )
		strcpy ( mCurrentMedia.input_profile, "" );
	else
		strcpy ( mCurrentMedia.input_profile, mInputList.getLabelFromIndex( mInputBox.getSelected()) );

	if ( mVideoDefault.isSelected() )
		strcpy ( mCurrentMedia.video_profile, "" );
	else
		strcpy ( mCurrentMedia.video_profile, mVideoList.getLabelFromIndex( mVideoBox.getSelected()) );

	if ( mSoundDefault.isSelected() )
		strcpy ( mCurrentMedia.sound_profile, "" );
	else
		strcpy ( mCurrentMedia.sound_profile, mSoundList.getLabelFromIndex( mSoundBox.getSelected()) );

	media_save_settings ( &mCurrentMedia );
}

void RomLauncher::changeRom ( int index )
{
	t_config *p_config = config_get_ptr();
	char tmp_string[256];

	/*
	 * Save current rom, if necessary
	 * FIXME: Keep track of current ROM?
	 */
	if ( mRomDirty == true ) {
		saveRom();
	}

	mRomDirty = false;

	/*
	 * Get values for this Rom
	 */
	TabIterator it;

	for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   	{
		if ( mTabList.at(mRomWindow.getSelectedIndex()) != (*it) )
			continue;

		mCurrentCRC = (*it)->mRomList.getValueFromIndex(index);
		mCurrentMedia.crc = mCurrentCRC;
		(*it)->mRomBox.setSelected(index);
	}

	media_get_settings ( &mCurrentMedia );

	/*
	 * Set values on the GUI
	 */
	sprintf ( tmp_string, "%lx", mCurrentCRC );
	mCRC.setCaption ( tmp_string );
	sprintf ( tmp_string, "%d", mCurrentMedia.image_size );
	mMD5.setCaption ( mCurrentMedia.md5 );
	mSHA1.setCaption ( mCurrentMedia.sha1 );
	mRomSize.setCaption ( tmp_string );
	switch ( mCurrentMedia.media_type )
	{
		case CART_CART: mRomType.setCaption ( "Cart" ); break;
		case CART_FLOPPY: mRomType.setCaption ( "Floppy" ); break;
		case CART_CASSETTE: mRomType.setCaption ( "Cassette" ); break;
		case CART_EXE: mRomType.setCaption ( "Executable" ); break;
		default: mRomType.setCaption ( "N/A" );
	}
	mMachine.setSelected ( mMachineList.getIndexFromValue(mCurrentMedia.machine_type ));
	switch ( mCurrentMedia.machine_type ) {
		case MACHINE_TYPE_MIN: mMachineFavLabel.setCaption("Machine - Auto 8-bit" ); break;
		case MACHINE_TYPE_5200: mMachineFavLabel.setCaption("Machine - Atari 5200" ); break;
		case MACHINE_TYPE_800: mMachineFavLabel.setCaption("Machine - Atari 800" ); break;
		case MACHINE_TYPE_XL: mMachineFavLabel.setCaption("Machine - Atari 800 XL/XE" ); break;
		default: mMachineFavLabel.setCaption("Machine - Unknown" ); break;
	}
	setMachine ( mCurrentMedia.machine_type );
	mMachineFavLabel.adjustSize();
	mCRC.adjustSize();
	mRomSize.adjustSize();
	mRomType.adjustSize();
	mFile.setText ( mCurrentMedia.filename );
	mTitle.setText ( mCurrentMedia.title );

	mMapping.setSelected ( mMappingList.getIndexFromValue(mCurrentMedia.mapping) );
	mRam.setSelected ( mRamList.getIndexFromValue(mCurrentMedia.ram_size) );
	mBasic.setSelected ( mBasicList.getIndexFromValue(mCurrentMedia.basic_enable) );
	mSIO.setSelected ( mSIOList.getIndexFromValue(mCurrentMedia.sio_patch_enable) );

	if ( !strlen(mCurrentMedia.input_profile) ) {
		if ( mCurrentMedia.machine_type == MACHINE_TYPE_5200 )
			mInputBox.setSelected ( mInputList.getIndexFromLabel(p_config->default_input_profile) );
		else
			mInputBox.setSelected ( mInputList.getIndexFromLabel(p_config->default_800input_profile) );
		mInputDefault.setSelected ( true );
	}
	else {
		mInputBox.setSelected ( mInputList.getIndexFromLabel(mCurrentMedia.input_profile) );
		mInputDefault.setSelected ( false );
	}

	if ( !strlen(mCurrentMedia.video_profile) ) {
		mVideoBox.setSelected ( mVideoList.getIndexFromLabel(p_config->default_video_profile) );
		mVideoDefault.setSelected ( true );
	}
	else {
		mVideoBox.setSelected ( mVideoList.getIndexFromLabel(mCurrentMedia.video_profile) );
		mVideoDefault.setSelected ( false );
	}

	if ( !strlen(mCurrentMedia.sound_profile) ) {
		mSoundBox.setSelected ( mSoundList.getIndexFromLabel(p_config->default_sound_profile) );
		mSoundDefault.setSelected ( true );
	}
	else {
		mSoundBox.setSelected ( mSoundList.getIndexFromLabel(mCurrentMedia.sound_profile) );
		mSoundDefault.setSelected ( false );
	}

	if ( mCurrentGroup != 0x8 ) {
		mInputBox.setVisible ( !mInputDefault.isSelected() );
		mVideoBox.setVisible ( !mVideoDefault.isSelected() );
		mSoundBox.setVisible ( !mSoundDefault.isSelected() );
	}
}

void RomLauncher::setMachine ( e_machine_type machine )
{
	int i;
	bool setting;
	char tmp_string[50];
	e_cart_type list_5200[] = {CART_MIN, CART_32K, CART_16K_TWO_CHIP, 
	                           CART_16K_ONE_CHIP, CART_40K, CART_64K};
	e_cart_type list_800[] = {CART_MIN, CART_PC_8K, CART_PC_16K, CART_PC_32K,
	                           CART_PC_40K, CART_PC_RIGHT, CART_XEGS, CART_SWXEGS, CART_MEGA,
	                           CART_OSS, CART_OSS2, CART_ATMAX, CART_WILL, CART_DIAMOND,
	                           CART_SDX, CART_EXP, CART_PHOENIX, CART_BLIZZARD, CART_ATRAX, 
	                           CART_FLOPPY, CART_CASSETTE, CART_EXE };

	mMappingList.deleteAll();

	if ( machine == MACHINE_TYPE_5200 )
	{
		setting = false;
		for ( i = 0; i < 6; ++i )
			mMappingList.addElement ( cart_get_mapping_string(list_5200[i],tmp_string), list_5200[i], 0 );
	}
	else
	{
		setting = true;
		for ( i = 0; i < 12; ++i )
			mMappingList.addElement ( cart_get_mapping_string(list_800[i],tmp_string), list_800[i], 0 );
	}
	mMapping.setSelected ( mMappingList.getIndexFromValue(mCurrentMedia.mapping) );

	mBasic.setVisible ( setting );
	mSIO.setVisible ( setting );
	mRam.setVisible ( setting );
	mBasicLabel.setVisible ( setting );
	mSIOLabel.setVisible ( setting );
	mRamLabel.setVisible ( setting );
}

void RomLauncher::launch ( int load_flag )
{
	t_config *p_config = config_get_ptr();
	t_media tmp_cart;

	if ( mRomDirty == true ) {
		saveRom();
	}

	tmp_cart.crc = mCurrentCRC;

	media_get_settings ( &tmp_cart );
	media_get_ptr()->crc = 0;

	strcpy ( p_config->rom_file, tmp_cart.filename );
	util_set_file_to_program_dir ( p_config->rom_file );
	hide();
	mGuiMain->launch( 0, load_flag );
}

void RomLauncher::load ( e_cart_type type, int slot )
{
	t_config *p_config = config_get_ptr();
	t_media tmp_cart;

	tmp_cart.crc = mCurrentCRC;

	media_get_settings ( &tmp_cart );

	util_set_file_to_program_dir ( tmp_cart.filename );
	
	media_open_rom ( tmp_cart.filename, type, slot, 0 );
}

void RomLauncher::actOnFavorites ( void )
{
	t_config *p_config = config_get_ptr();
	t_media tmp_cart;
	RomLauncherTab *pTab = 0;
	TabIterator it;

	for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   	{
		if ( mTabList.at(mRomWindow.getSelectedIndex()) == (*it) )
		{
			pTab = (*it);
			break;
		}
	}

	if ( mCurrentGroup != 0x8 ) {
		tmp_cart.crc = pTab->mRomList.getValueFromIndex(pTab->mRomBox.getSelected());
		if ( pTab->mRomBox.getSelected() > -1 )
		{
			media_get_settings ( &tmp_cart );
			tmp_cart.favorite = 1;
			media_save_settings ( &tmp_cart );
			mGuiMain->updateLauncherList();
		}
	}
	else {
		tmp_cart.crc = pTab->mRomList.getValueFromIndex(pTab->mRomBox.getSelected());
		if ( pTab->mRomBox.getSelected() > -1 ) {
			media_get_settings ( &tmp_cart );
			tmp_cart.favorite = 0;
			media_save_settings ( &tmp_cart );
			mGuiMain->updateLauncherList();
		}
	}
}

void RomLauncher::action(const gcn::ActionEvent &actionEvent)
{
	t_config *p_config = config_get_ptr();

	if ( actionEvent.getId() == "mRomBox" ) {

		TabIterator it;

		for ( it = mTabList.begin(); it != mTabList.end(); ++it )
   		{
			if ( mTabList.at(mRomWindow.getSelectedIndex()) != (*it) )
				continue;

			if ( (clock() - mClickTime < CLOCKS_PER_SEC) && 
			     (mCurrentCRC == (*it)->mRomList.getValueFromIndex((*it)->mRomBox.getSelected())) ) {

				launch(0);
			}
	
			changeRom ( (*it)->mRomBox.getSelected() );
	
			mClickTime = clock();
		}
	}
	else if ( actionEvent.getId() == "mDirty" ) {
		mRomDirty = true;
	}
	else if ( actionEvent.getId() == "mOk" ) {
		if ( mRomDirty == true ) {
			saveRom();
		}
		hide();
	}
	else if ( actionEvent.getId() == "mCancel" ) {
		hide();
	}
	else if ( actionEvent.getId() == "mInputDefault" ) {
		mInputBox.setVisible ( !mInputDefault.isSelected() );
		mRomDirty = true;
	}
	else if ( actionEvent.getId() == "mVideoDefault" ) {
		mVideoBox.setVisible ( !mVideoDefault.isSelected() );
		mRomDirty = true;
	}
	else if ( actionEvent.getId() == "mSoundDefault" ) {
		mSoundBox.setVisible ( !mSoundDefault.isSelected() );
		mRomDirty = true;
	}
	else if ( actionEvent.getId() == "mMachine" ) {
		mRomDirty = true;
		setMachine ( (e_machine_type) mMachineList.getValueFromIndex ( mMachine.getSelected() ) );
	}
}

void RomLauncher::mousePressed(gcn::MouseEvent &mouseEvent)
{
	bool comp_has_mouse = false;

	if ( mouseEvent.getSource() == &mFile || mouseEvent.getSource() == &mTitle )
		comp_has_mouse = true;

    if ((comp_has_mouse == true) && mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
		mRomDirty = true;
    }

	gcn::Window::mousePressed(mouseEvent);
}

void RomLauncher::keyPressed (gcn::KeyEvent &keyEvent)
{
	int i,j;
	int start;
	RomLauncherTab *pTab;

	if ( !keyEvent.getKey().isCharacter() )
		return;

	pTab = mTabList.at(mRomWindow.getSelectedIndex());
	/*
	 * Key Level depends on time from last key press
	 */
	if ( clock() - mKeyTime < (CLOCKS_PER_SEC*1.0) ) {
		mKeyLevel++;
		start = pTab->mRomBox.getSelected();
	}
	else {
		mKeyLevel = 0;
		start = 0;
	}

	if ( mKeyLevel > 3 )
		mKeyLevel = 3;

	mKeySave[mKeyLevel] = keyEvent.getKey().getValue();

	/*
	 * First check for previous characters matching
	 *   If not don't change position in list
	 */

	/*
	 * Now do a search
	 */ 
	//if ( key.isCharacter() )
	//{
		for ( i = start; i < pTab->mRomList.getNumberOfElements(); ++i )
		{

			/*
			 * First check for previous characters matching
			 *   If not don't change position in list
			 */
			for ( j = 0; j < mKeyLevel; ++j ) {
				if ( (int)strlen(pTab->mRomList.getLabelFromIndex(i)) > j ) {
					if ( tolower(pTab->mRomList.getLabelFromIndex(i)[j]) != 
					     tolower(mKeySave[j]) )
					return;
				}
			}

			if ( (int)strlen(pTab->mRomList.getLabelFromIndex(i)) > mKeyLevel ) {

				if ( tolower(pTab->mRomList.getLabelFromIndex(i)[mKeyLevel]) >= 
				     tolower(keyEvent.getKey().getValue()) )
				{
					pTab->mRomBox.setSelected(i);
					changeRom ( i );
					break;
				}
			}
		}
	//}

	mKeyTime = clock();
}

RomLauncherTab::RomLauncherTab ( GuiMain *gMain, std::string caption, gcn::ActionListener *al, gcn::KeyListener *kl ) : GuiObject(gMain)
{
	setCaption ( caption );
	mRomList.setSortMethod ( GLIST_SORT_NONE );
	mRomScroller.setContent ( &mRomBox );

	mRomBox.addActionListener ( al );
	mRomBox.addKeyListener ( kl );
	mRomBox.setActionEventId ( "mRomBox" );

	add ( &mRomScroller );

	setDimension ( mGuiMain->getRectangle(-1,-1,72,14.2) );
	mRomScroller.setDimension ( mGuiMain->getRectangle(1,0.5,70,12.4) );
	mRomBox.setDimension ( mGuiMain->getRectangle(0,2.0,71,13.7) );
}
