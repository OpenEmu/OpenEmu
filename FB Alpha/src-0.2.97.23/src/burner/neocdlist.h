#ifndef _neocdlist_
#define _neocdlist_

struct NGCDGAME
{
	TCHAR*	pszName;		// Short name
	TCHAR*	pszTitle;		// Title
	TCHAR*	pszYear;		// Release Year
	TCHAR*	pszCompany;		// Developer
	unsigned int id;				// Game ID
};

NGCDGAME*	GetNeoGeoCDInfo(unsigned int nID);
int			GetNeoCDTitle(unsigned int nGameID);
void		SetNeoCDTitle(TCHAR* pszTitle); 
int			GetNeoGeoCD_Identifier();

bool	IsNeoGeoCD();		// neo_run.cpp
TCHAR*	GetIsoPath();	// cd_isowav.cpp

int		NeoCDInfo_Init();
TCHAR*	NeoCDInfo_Text(int nText);
int		NeoCDInfo_ID();
void	NeoCDInfo_Exit();

// ------------------------------------------------------
// ISO9660 STUFF

// ISO9660 date and time
struct iso9660_date
{
	unsigned char Year;		// Number of year since 1900
	unsigned char Month;	// Month (1 to 12)
	unsigned char Day;		// Day (1 to 31) 
	unsigned char Hour;		// Hour (0 to 23) 
	unsigned char Minute;	// Minute (0 to 59) 
	unsigned char Second;	// Second (0 to 59) 
	unsigned char Zone;		// Offset related to GMT, by 15-min interval (from -48 to +52)
};

// ISO9660 BCD date and time
struct iso9660_bcd_date
{
	unsigned char Year[4];
    unsigned char Month[2];
    unsigned char Day[2];
    unsigned char Hour[2];
    unsigned char Minute[2];
    unsigned char Second[2];
	unsigned char SecFrac[2];
    char Zone;
};

struct iso9660_DirectoryRecord
{
    unsigned char len_dr;					// [1] Length of Directory Record (bytes)
    unsigned char ext_attr_rec_len;			// [1] Extended Attribute Record Length (bytes)
    unsigned char location_of_extent[8];	// [8] [LEF / BEF] LBN / Sector location of the file data
    unsigned char data_lenth[8];			// [8] [LEF / BEF] Length of the file section (bytes)
	iso9660_date rec_date_time;				// [7] Recording Date and Time
	unsigned char file_flags;				// [1] 8-bit flags 
												// [bit 0] File is Hidden if this bit is 1
												// [bit 1] Entry is a Directory if this bit is 1
												// [bit 2] Entry is an Associated file is this bit is 1
												// [bit 3] Information is structured according to the extended attribute record if this bit is 1
												// [bit 4] Owner, group and permissions are specified in the extended attribute record if this bit is 1
												// [bit 5] Reserved (0)
												// [bit 6] Reserved (0)
												// [bit 7] File has more than one directory record if this bit is 1
	unsigned char file_unit_size;			// [1] This field is only valid if the file is recorded in interleave mode, otherwise this field is (00)
    unsigned char int_gap_size;				// [1] This field is only valid if the file is recorded in interleave mode, otherwise this field is (00)
   	unsigned char vol_seq_number[4];		// [4] The ordinal number of the volume in the Volume Set on which the file described by the directory record is recorded.
    unsigned char len_fi;					// [1] Length of File Identifier (LEN_FI)
    char* file_id;							// [LEN_FI] File Identifier
};

struct iso9660_PathTableRecord 
{
	unsigned char len_di;					// [1] Length of Directory Identifier
	unsigned char extended_attr_len;		// [1] Extended Attribute Record Length
	unsigned char extent_location[4];		// [4] Extent Location (Sector)
	unsigned char parent_dir_num;			// [1] Parent Directory Number
	char* directory_id;						// [LEN_DI] Directory Identifier
};

// Volume Descriptor Header (Sector 16)
struct iso9660_VDH 
{
	char vdtype;	// [1] Volume Descriptor Type
	unsigned char stdid[5];	// [5] ISO9660 Standard Identifier
	char vdver;		// [1] Volume Descriptor Version
};

// Primary Volume Descriptor (Volume Descriptor Type 1)
struct iso9660_PVD
{
	// -----------------------------
	// Volume Descriptor Header
	char type;
	unsigned char id[5];
	char version;
	// -----------------------------
	// Primary Volume Descriptor
	char unused1;
	unsigned char system_id[32];
	unsigned char volume_id[32];
	unsigned char unused2[8];
	unsigned char volume_space_size[8];
	unsigned char unused3[32];
	unsigned char volume_set_size[4];
	unsigned char volume_sequence_number[4];
	unsigned char logical_block_size[4];
	unsigned char path_table_size[8];
	unsigned char type_l_path_table[4];
	unsigned char opt_type_l_path_table[4];
	unsigned char type_m_path_table[4];
	unsigned char opt_type_m_path_table[4];
	iso9660_DirectoryRecord root_directory_record;
	unsigned char volume_set_id[128]; 
	unsigned char publisher_id[128];
	unsigned char preparer_id[128];
	unsigned char application_id[128]; 
	unsigned char copyright_file_id[37]; 
	unsigned char abstract_file_id[37]; 
	unsigned char bibliographic_file_id[37];

	iso9660_bcd_date creation_date;		// [17]
	iso9660_bcd_date modification_date;	// [17]
	iso9660_bcd_date expiration_date;	// [17]
	iso9660_bcd_date effective_date;	// [17]

	char file_structure_version;
	char unused4;
	unsigned char application_data[512];
	unsigned char unused5[643];
};

void iso9660_ReadOffset(unsigned char *Dest, FILE* fp, unsigned int lOffset, unsigned int lSize, unsigned int lLength);

// ------------------------------------------------------


#endif
