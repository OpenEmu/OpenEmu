#ifndef LIBRARIES_XADMASTER_H
#define LIBRARIES_XADMASTER_H

/*  $Id: xadmaster.h.in,v 1.24 2005/06/23 14:54:42 stoecker Exp $
    xadmaster.library defines and structures

    XAD library system for archive handling
    Copyright (C) 1998 and later by Dirk Stöcker <soft@dstoecker.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
typedef uint32_t           xadUINT32;
typedef int32_t            xadINT32;
typedef uint16_t           xadUINT16;
typedef int16_t            xadINT16;
typedef uint8_t            xadUINT8;
typedef int8_t             xadINT8;
typedef uint64_t           xadSize;
typedef int64_t            xadSignSize;

typedef void *             xadPTR;
typedef unsigned char      xadSTRING;
typedef xadSTRING *        xadSTRPTR;
typedef unsigned int       xadUINT;
typedef int                xadINT;
typedef xadINT             xadERROR;
typedef xadINT             xadBOOL;

#define XADFALSE        0
#define XADTRUE         1
#define XADMEMF_ANY     (0)
#define XADMEMF_CLEAR   (1L << 16)
#define XADMEMF_PUBLIC  (1L << 0)

/* Portable define to put the xadMasterBase parameter as the first
 * parameter of any function. This macro makes it simpler. */
#define XADM    xadMasterBase,

typedef xadUINT32        xadTag;

struct TagItem {
  xadTag  ti_Tag;
  xadSize ti_Data;
};

#include <stdio.h>
typedef int xadFileHandle;
typedef const struct TagItem * xadTAGPTR;

/* is a tag's data a pointer, an int, or a 64-bit size? */
#define TAG_PTR (0x80000000UL)
#define TAG_INT (0x40000000UL)
#define TAG_SIZ (0x20000000UL)

/* end of taglist */
#define TAG_DONE   (TAG_INT + 0x10000000UL)
/* ignore this tag */
#define TAG_IGNORE (TAG_INT + 0x10000001UL)
/* taglist continues at (struct TagItem *) ti_Data */
#define TAG_MORE   (TAG_PTR + 0x10000002UL)
/* ignore the next [(xadUINT32) ti_Data] tags */
#define TAG_SKIP   (TAG_INT + 0x10000003UL)

/* a standard callback system */
struct Hook {
  /* h_MinNode, used for hook linking on Amiga, is not needed by XAD */
  xadUINT32 (*h_Entry)();    /* entry point of callback */
  /* h_SubEntry, used when h_Entry on Amiga is an asm stub, is not needed */
  xadPTR *h_Data;            /* data parameter always passed to callback */
};

/* NOTE: Nearly all structures need to be allocated using the
   xadAllocObject function. */

/************************************************************************
*                                                                       *
*    library base structure                                             *
*                                                                       *
************************************************************************/

struct xadMasterBase {
  xadPTR dummy;
};

/************************************************************************
*                                                                       *
*    tag-function call flags                                            *
*                                                                       *
************************************************************************/

/* input tags for xadGetInfo, only one can be specified per call */
#define XAD_INSIZE              (TAG_SIZ+  1) /* input data size */
#define XAD_INFILENAME          (TAG_PTR+  2)
#define XAD_INFILEHANDLE        (TAG_PTR+  3)
#define XAD_INMEMORY            (TAG_PTR+  4)
#define XAD_INHOOK              (TAG_PTR+  5)
#define XAD_INSPLITTED          (TAG_PTR+  6) /* (V2) */
#define XAD_INDISKARCHIVE       (TAG_PTR+  7) /* (V4) */
#define XAD_INXADSTREAM         (TAG_PTR+  8) /* (V8) */
#define XAD_INDEVICE            (TAG_PTR+  9) /* (V11) */

/* output tags, only one can be specified per call, xadXXXXUnArc */
#define XAD_OUTSIZE             (TAG_SIZ+ 10) /* output data size */
#define XAD_OUTFILENAME         (TAG_PTR+ 11)
#define XAD_OUTFILEHANDLE       (TAG_PTR+ 12)
#define XAD_OUTMEMORY           (TAG_PTR+ 13)
#define XAD_OUTHOOK             (TAG_PTR+ 14)
#define XAD_OUTDEVICE           (TAG_PTR+ 15)
#define XAD_OUTXADSTREAM        (TAG_PTR+ 16) /* (V8) */

/* object allocation tags for xadAllocObjectA */
#define XAD_OBJNAMESIZE         (TAG_INT+ 20) /* XADOBJ_FILEINFO, size of needed name space */
#define XAD_OBJCOMMENTSIZE      (TAG_INT+ 21) /* XADOBJ_FILEINFO, size of needed comment space */
#define XAD_OBJPRIVINFOSIZE     (TAG_INT+ 22) /* XADOBJ_FILEINFO & XADOBJ_DISKINFO, self use size */
#define XAD_OBJBLOCKENTRIES     (TAG_INT+ 23) /* XADOBJ_DISKINFO, number of needed entries */

/* tags for xadGetInfo, xadFileUnArc and xadDiskUnArc */
#define XAD_NOEXTERN            (TAG_INT+ 50) /* do not use extern clients */
#define XAD_PASSWORD            (TAG_PTR+ 51) /* password when needed */
#define XAD_ENTRYNUMBER         (TAG_INT+ 52) /* number of wanted entry */
#define XAD_PROGRESSHOOK        (TAG_PTR+ 53) /* the progress hook */
#define XAD_OVERWRITE           (TAG_INT+ 54) /* overwrite file ? */
#define XAD_MAKEDIRECTORY       (TAG_INT+ 55) /* create directory tree */
#define XAD_IGNOREGEOMETRY      (TAG_INT+ 56) /* ignore drive geometry ? */
#define XAD_LOWCYLINDER         (TAG_INT+ 57) /* lowest cylinder */
#define XAD_HIGHCYLINDER        (TAG_INT+ 58) /* highest cylinder */
#define XAD_VERIFY              (TAG_INT+ 59) /* verify for disk hook */
#define XAD_NOKILLPARTIAL       (TAG_INT+ 60) /* do not delete partial/corrupt files (V3.3) */
#define XAD_FORMAT              (TAG_INT+ 61) /* format output device (V5) */
#define XAD_USESECTORLABELS     (TAG_INT+ 62) /* sector labels are stored on disk (V9) */
#define XAD_IGNOREFLAGS         (TAG_INT+ 63) /* ignore the client, if certain flags are set (V11) */
#define XAD_ONLYFLAGS           (TAG_INT+ 64) /* ignore the client, if certain flags are NOT set (V11) */
// GOA
#define XAD_CLIENT              (TAG_INT+ 65) /* skip the recognition, use this client  */


/* input tags for xadConvertDates, only one can be passed */
#define XAD_DATEUNIX            (TAG_INT+ 70) /* unix date variable */
#define XAD_DATEAMIGA           (TAG_INT+ 71) /* Amiga date variable */
#define XAD_DATEDATESTAMP       (TAG_PTR+ 72) /* Amiga struct DateStamp */
#define XAD_DATEXADDATE         (TAG_PTR+ 73) /* struct xadDate */
#define XAD_DATECLOCKDATA       (TAG_PTR+ 74) /* Amiga struct ClockData */
#define XAD_DATECURRENTTIME     (TAG_INT+ 75) /* input is system time */
#define XAD_DATEMSDOS           (TAG_INT+ 76) /* MS-DOS packed format (V2) */
#define XAD_DATEMAC             (TAG_INT+ 77) /* Mac date variable (V8) */
#define XAD_DATECPM             (TAG_PTR+ 78) /* CP/M data structure (V10) */
#define XAD_DATECPM2            (TAG_INT+ 79) /* CP/M data structure type 2 (V10) */
#define XAD_DATEISO9660         (TAG_PTR+300) /* ISO9660 date structure (V11) */

/* output tags, there can be specified multiple tags for one call */
#define XAD_GETDATEUNIX         (TAG_PTR+ 80) /* unix date variable */
#define XAD_GETDATEAMIGA        (TAG_PTR+ 81) /* Amiga date variable */
#define XAD_GETDATEDATESTAMP    (TAG_PTR+ 82) /* Amiga struct DateStamp */
#define XAD_GETDATEXADDATE      (TAG_PTR+ 83) /* struct xadDate */
#define XAD_GETDATECLOCKDATA    (TAG_PTR+ 84) /* Amiga struct ClockData */
#define XAD_GETDATEMSDOS        (TAG_PTR+ 86) /* MS-DOS packed format (V2) */
#define XAD_GETDATEMAC          (TAG_PTR+ 87) /* Mac date variable (V8) */
#define XAD_GETDATECPM          (TAG_PTR+ 88) /* CP/M data structure (V10) */
#define XAD_GETDATECPM2         (TAG_PTR+ 89) /* CP/M data structure type 2 (V10) */
#define XAD_GETDATEISO9660      (TAG_PTR+320) /* ISO9660 date structure (V11) */

/* following tags need locale.library to be installed on Amiga */
#define XAD_MAKEGMTDATE         (TAG_INT+ 90) /* make local to GMT time */
#define XAD_MAKELOCALDATE       (TAG_INT+ 91) /* make GMT to local time */

/* tags for xadHookTagAccess (V3) */
#define XAD_USESKIPINFO         (TAG_INT+104) /* the hook uses xadSkipInfo (V3) */
#define XAD_SECTORLABELS        (TAG_PTR+105) /* pass sector labels with XADAC_WRITE (V9) */

#define XAD_GETCRC16            (TAG_PTR+120) /* pointer to xadUINT16 value (V3) */
#define XAD_GETCRC32            (TAG_PTR+121) /* pointer to xadUINT32 value (V3) */

#define XAD_CRC16ID             (TAG_INT+130) /* ID for crc calculation (V3) */
#define XAD_CRC32ID             (TAG_INT+131) /* ID for crc calculation (V3) */

/* tags for xadConvertProtection (V4) */
#define XAD_PROTAMIGA           (TAG_INT+160) /* Amiga type protection bits (V4) */
#define XAD_PROTUNIX            (TAG_INT+161) /* protection bits in UNIX mode (V4) */
#define XAD_PROTMSDOS           (TAG_INT+162) /* MSDOS type protection bits (V4) */
#define XAD_PROTFILEINFO        (TAG_PTR+163) /* input is a xadFileInfo structure (V11) */

#define XAD_GETPROTAMIGA        (TAG_PTR+170) /* return Amiga protection bits (V4) */
#define XAD_GETPROTUNIX         (TAG_PTR+171) /* return UNIX protection bits (V11) */
#define XAD_GETPROTMSDOS        (TAG_PTR+172) /* return MSDOS protection bits (V11) */
#define XAD_GETPROTFILEINFO     (TAG_PTR+173) /* fill xadFileInfo protection fields (V11) */

/* tags for xadGetDiskInfo (V7) */
#define XAD_STARTCLIENT         (TAG_PTR+180) /* the client to start with (V7) */
#define XAD_NOEMPTYERROR        (TAG_INT+181) /* do not create XADERR_EMPTY (V8) */

/* tags for xadFreeHookAccess (V8) */
#define XAD_WASERROR            (TAG_INT+190) /* error occured, call abort method (V8) */

/* tags for miscellaneous stuff */
#define XAD_ARCHIVEINFO         (TAG_PTR+200) /* xadArchiveInfo for stream hooks (V8) */
#define XAD_ERRORCODE           (TAG_PTR+201) /* error code of function (V12) */
#define XAD_EXTENSION           (TAG_PTR+202) /* argument for xadGetDefaultName() (V13) */

/* tags for xadAddFileEntry and xadAddDiskEntry (V10) */
#define XAD_SETINPOS            (TAG_SIZ+240) /* set xai_InPos after call (V10) */
#define XAD_INSERTDIRSFIRST     (TAG_INT+241) /* insert dirs at list start (V10) */

/* tags for xadConvertName (V12) */
#define XAD_PATHSEPERATOR       (TAG_PTR+260) /* xadUINT16 *, default is {'/','\\',0} in source charset (V12) */
#define XAD_CHARACTERSET        (TAG_INT+261) /* the characterset of string (V12) */
#define XAD_STRINGSIZE          (TAG_INT+262) /* maximum size of following (V12) */
#define XAD_CSTRING             (TAG_PTR+263) /* zero-terminated string (V12) */
#define XAD_PSTRING             (TAG_PTR+264) /* lengthed Pascal string (V12) */
#define XAD_XADSTRING           (TAG_PTR+265) /* an xad string (V12) */
#define XAD_ADDPATHSEPERATOR    (TAG_INT+266) /* default is TRUE (V12) */

/* tags for xadGetFilename (V12) */
#define XAD_NOLEADINGPATH       (TAG_INT+280) /* default is FALSE (V12) */
#define XAD_NOTRAILINGPATH      (TAG_INT+281) /* default is FALSE (V12) */
#define XAD_MASKCHARACTERS      (TAG_PTR+282) /* default are #?()[]~%*:|",1-31,127-160 (V12) */
#define XAD_MASKINGCHAR         (TAG_INT+283) /* default is '_' (V12) */
#define XAD_REQUIREDBUFFERSIZE  (TAG_PTR+284) /* pointer which should hold buf size (V12) */

/* Places 300-339 used for dates! */

/************************************************************************
*                                                                       *
*    objects for xadAllocObjectA                                        *
*                                                                       *
************************************************************************/

#define XADOBJ_ARCHIVEINFO      0x0001 /* struct xadArchiveInfo */
#define XADOBJ_FILEINFO         0x0002 /* struct xadFileInfo */
#define XADOBJ_DISKINFO         0x0003 /* struct xadDiskInfo */
#define XADOBJ_HOOKPARAM        0x0004 /* struct HookParam */
#define XADOBJ_DEVICEINFO       0x0005 /* struct xadDeviceInfo */
#define XADOBJ_PROGRESSINFO     0x0006 /* struct xadProgressInfo */
#define XADOBJ_TEXTINFO         0x0007 /* struct xadTextInfo */
#define XADOBJ_SPLITFILE        0x0008 /* struct xadSplitFile (V2) */
#define XADOBJ_SKIPINFO         0x0009 /* struct xadSkipInfo (V3) */
#define XADOBJ_IMAGEINFO        0x000A /* struct xadImageInfo (V4) */
#define XADOBJ_SPECIAL          0x000B /* struct xadSpecial (V11) */

/* result type of xadAllocVec */
#define XADOBJ_MEMBLOCK         0x0100 /* memory of requested size and type */
/* private type */
#define XADOBJ_STRING           0x0101 /* a typed XAD string (V12) */

/************************************************************************
*                                                                       *
*    modes for xadCalcCRC126 and xadCalcCRC32                           *
*                                                                       *
************************************************************************/

#define XADCRC16_ID1            0xA001
#define XADCRC32_ID1            0xEDB88320

/************************************************************************
*                                                                       *
*    hook related stuff                                                 *
*                                                                       *
************************************************************************/

#define XADHC_READ      1       /* read data into buffer */
#define XADHC_WRITE     2       /* write buffer data to file/memory */
#define XADHC_SEEK      3       /* seek in file */
#define XADHC_INIT      4       /* initialize the hook */
#define XADHC_FREE      5       /* end up hook work, free stuff */
#define XADHC_ABORT     6       /* an error occured, delete partial stuff */
#define XADHC_FULLSIZE  7       /* complete input size is needed */
#define XADHC_IMAGEINFO 8       /* return disk image info (V4) */

struct xadHookParam {
  xadUINT32    xhp_Command;
  xadSignSize  xhp_CommandData;
  xadPTR       xhp_BufferPtr;
  xadSize      xhp_BufferSize;
  xadSize      xhp_DataPos;        /* current seek position */
  xadPTR       xhp_PrivatePtr;
  xadTAGPTR    xhp_TagList;        /* allows to transport tags to hook (V9) */
};

/* xadHookAccess commands */
#define XADAC_READ              10      /* get data */
#define XADAC_WRITE             11      /* write data */
#define XADAC_COPY              12      /* copy input to output */
#define XADAC_INPUTSEEK         13      /* seek in input file */
#define XADAC_OUTPUTSEEK        14      /* seek in output file */

/************************************************************************
*                                                                       *
*    support structures                                                 *
*                                                                       *
************************************************************************/

/* Own date structure to cover all possible dates in a human friendly
   format. xadConvertDates may be used to convert between different date
   structures and variables. */
struct xadDate {
  xadUINT32 xd_Micros;  /* values 0 to 999999     */
  xadINT32  xd_Year;    /* values 1 to 2147483648 */
  xadUINT8  xd_Month;   /* values 1 to 12         */
  xadUINT8  xd_WeekDay; /* values 1 to 7          */
  xadUINT8  xd_Day;     /* values 1 to 31         */
  xadUINT8  xd_Hour;    /* values 0 to 23         */
  xadUINT8  xd_Minute;  /* values 0 to 59         */
  xadUINT8  xd_Second;  /* values 0 to 59         */
};

#define XADDAY_MONDAY           1       /* monday is the first day and */
#define XADDAY_TUESDAY          2
#define XADDAY_WEDNESDAY        3
#define XADDAY_THURSDAY         4
#define XADDAY_FRIDAY           5
#define XADDAY_SATURDAY         6
#define XADDAY_SUNDAY           7       /* sunday the last day of a week */

struct xadDeviceInfo { /* for XAD_OUTDEVICE tag */
  xadSTRPTR xdi_DeviceName; /* name of device */
  xadUINT32 xdi_Unit;       /* unit of device */
  xadSTRPTR xdi_DOSName;    /* instead of Device+Unit, dos name without ':' */
};

struct xadSplitFile { /* for XAD_INSPLITTED */
  struct xadSplitFile *xsf_Next;
  xadUINT32            xsf_Type; /* XAD_INFILENAME, XAD_INFILEHANDLE, XAD_INMEMORY, XAD_INHOOK */
  xadSize              xsf_Size; /* necessary for XAD_INMEMORY, useful for others */
  xadSize            xsf_Data; /* FileName, Filehandle, Hookpointer or Memory */
};

struct xadSkipInfo {
  struct xadSkipInfo *xsi_Next;
  xadSize             xsi_Position; /* position, where it should be skipped */
  xadSize             xsi_SkipSize; /* size to skip */
};

struct xadImageInfo { /* for XADHC_IMAGEINFO */
  xadUINT32 xii_SectorSize;   /* usually 512 */
  xadUINT32 xii_FirstSector;  /* of the image file */
  xadUINT32 xii_NumSectors;   /* of the image file */
  xadUINT32 xii_TotalSectors; /* of this device type */
};
/* If the image file holds total data of disk xii_TotalSectors equals
   xii_NumSectors and xii_FirstSector is zero. Addition of xii_FirstSector
   and xii_NumSectors cannot exceed xii_TotalSectors value!
*/

/************************************************************************
*                                                                       *
*    system information structure                                       *
*                                                                       *
************************************************************************/
struct xadSystemInfo {
  xadUINT16                xsi_Version;   /* master library version */
  xadUINT16                xsi_Revision;  /* master library revision */
  xadSize                  xsi_RecogSize; /* size for recognition */
};

/************************************************************************
*                                                                       *
*    information structures                                             *
*                                                                       *
************************************************************************/

struct xadArchiveInfo {
  struct xadClient *   xai_Client;   /* pointer to unarchiving client */
  xadPTR               xai_PrivateClient; /* private client data */
  xadSTRPTR            xai_Password; /* password for crypted archives */
  xadUINT32            xai_Flags;    /* read only XADAIF_ flags */
  xadUINT32            xai_LowCyl;   /* lowest cylinder to unarchive */
  xadUINT32            xai_HighCyl;  /* highest cylinder to unarchive */
  xadSize              xai_InPos;    /* input position, read only */
  xadSize              xai_InSize;   /* input size, read only */
  xadSize              xai_OutPos;   /* output position, read only */
  xadSize              xai_OutSize;  /* output file size, read only */
  struct xadFileInfo * xai_FileInfo; /* data pointer for file arcs */
  struct xadDiskInfo * xai_DiskInfo; /* data pointer for disk arcs */
  struct xadFileInfo * xai_CurFile;  /* data pointer for current file arc */
  struct xadDiskInfo * xai_CurDisk;  /* data pointer for current disk arc */
  xadERROR             xai_LastError;   /* last error, when XADAIF_FILECORRUPT (V2) */
  xadSize *            xai_MultiVolume; /* array of start offsets from parts (V2) */
  struct xadSkipInfo * xai_SkipInfo;    /* linked list of skip entries (V3) */
  struct xadImageInfo *xai_ImageInfo;   /* for filesystem clients (V5) */
  xadSTRPTR            xai_InName;   /* Input archive name if available (V7) */
};
/* This structure is nearly complete private to either xadmaster or its
clients. An application program may access for reading only xai_Client,
xai_Flags, xai_FileInfo and xai_DiskInfo. For xai_Flags only XADAIF_CRYPTED
and XADAIF_FILECORRUPT are useful. All the other stuff is private and should
not be accessed! */

#define XADAIB_CRYPTED           0 /* archive entries are encrypted */
#define XADAIB_FILECORRUPT       1 /* file is corrupt, but valid entries are in the list */
#define XADAIB_FILEARCHIVE       2 /* unarchive file entry */
#define XADAIB_DISKARCHIVE       3 /* unarchive disk entry */
#define XADAIB_OVERWRITE         4 /* overwrite the file (PRIVATE) */
#define XADAIB_MAKEDIRECTORY     5 /* create directory when missing (PRIVATE) */
#define XADAIB_IGNOREGEOMETRY    6 /* ignore drive geometry (PRIVATE) */
#define XADAIB_VERIFY            7 /* verify is turned on for disk hook (PRIVATE) */
#define XADAIB_NOKILLPARTIAL     8 /* do not delete partial files (PRIVATE) */
#define XADAIB_DISKIMAGE         9 /* is disk image extraction (V5) */
#define XADAIB_FORMAT           10 /* format in disk hook (PRIVATE) */
#define XADAIB_NOEMPTYERROR     11 /* do not create empty error (PRIVATE) */
#define XADAIB_ONLYIN           12 /* in stuff only (PRIVATE) */
#define XADAIB_ONLYOUT          13 /* out stuff only (PRIVATE) */
#define XADAIB_USESECTORLABELS  14 /* use SectorLabels (PRIVATE) */

#define XADAIF_CRYPTED          (1<<XADAIB_CRYPTED)
#define XADAIF_FILECORRUPT      (1<<XADAIB_FILECORRUPT)
#define XADAIF_FILEARCHIVE      (1<<XADAIB_FILEARCHIVE)
#define XADAIF_DISKARCHIVE      (1<<XADAIB_DISKARCHIVE)
#define XADAIF_OVERWRITE        (1<<XADAIB_OVERWRITE)
#define XADAIF_MAKEDIRECTORY    (1<<XADAIB_MAKEDIRECTORY)
#define XADAIF_IGNOREGEOMETRY   (1<<XADAIB_IGNOREGEOMETRY)
#define XADAIF_VERIFY           (1<<XADAIB_VERIFY)
#define XADAIF_NOKILLPARTIAL    (1<<XADAIB_NOKILLPARTIAL)
#define XADAIF_DISKIMAGE        (1<<XADAIB_DISKIMAGE)
#define XADAIF_FORMAT           (1<<XADAIB_FORMAT)
#define XADAIF_NOEMPTYERROR     (1<<XADAIB_NOEMPTYERROR)
#define XADAIF_ONLYIN           (1<<XADAIB_ONLYIN)
#define XADAIF_ONLYOUT          (1<<XADAIB_ONLYOUT)
#define XADAIF_USESECTORLABELS  (1<<XADAIB_USESECTORLABELS)

struct xadFileInfo {
  struct xadFileInfo * xfi_Next;
  xadUINT32            xfi_EntryNumber;/* number of entry */
  xadSTRPTR            xfi_EntryInfo;  /* additional archiver text */
  xadPTR               xfi_PrivateInfo;/* client private, see XAD_OBJPRIVINFOSIZE */
  xadUINT32            xfi_Flags;      /* see XADFIF_xxx defines */
  xadSTRPTR            xfi_FileName;   /* see XAD_OBJNAMESIZE tag */
  xadSTRPTR            xfi_Comment;    /* see XAD_OBJCOMMENTSIZE tag */
  xadUINT32            xfi_Protection; /* AmigaOS3 bits (including multiuser) */
  xadUINT32            xfi_OwnerUID;   /* user ID */
  xadUINT32            xfi_OwnerGID;   /* group ID */
  xadSTRPTR            xfi_UserName;   /* user name */
  xadSTRPTR            xfi_GroupName;  /* group name */
  xadSize              xfi_Size;       /* size of this file */
  xadSize              xfi_GroupCrSize;/* crunched size of group */
  xadSize              xfi_CrunchSize; /* crunched size */
  xadSTRPTR            xfi_LinkName;   /* name and path of link */
  struct xadDate       xfi_Date;
  xadUINT16            xfi_Generation; /* File Generation [0...0xFFFF] (V3) */
  xadSize              xfi_DataPos;    /* crunched data position (V3) */
  struct xadFileInfo * xfi_MacFork;    /* pointer to 2nd fork for Mac (V7) */
  xadUINT16            xfi_UnixProtect;/* protection bits for Unix (V11) */
  xadUINT8             xfi_DosProtect; /* protection bits for MS-DOS (V11) */
  xadUINT8             xfi_FileType;   /* XADFILETYPE to define type of exe files (V11) */
  struct xadSpecial *  xfi_Special;    /* pointer to special data (V11) */
};

/* These are used for xfi_FileType to define file type. (V11) */
#define XADFILETYPE_DATACRUNCHER     1   /* infile was only one data file */
#define XADFILETYPE_TEXTLINKER       2   /* infile was text-linked */

#define XADFILETYPE_AMIGAEXECRUNCHER 11  /* infile was an Amiga exe cruncher */
#define XADFILETYPE_AMIGAEXELINKER   12  /* infile was an Amiga exe linker */
#define XADFILETYPE_AMIGATEXTLINKER  13  /* infile was an Amiga text-exe linker */
#define XADFILETYPE_AMIGAADDRESS     14  /* infile was an Amiga address cruncher */

#define XADFILETYPE_UNIXBLOCKDEVICE  21  /* this file is a block device */
#define XADFILETYPE_UNIXCHARDEVICE   22  /* this file is a character device */
#define XADFILETYPE_UNIXFIFO         23  /* this file is a named pipe */
#define XADFILETYPE_UNIXSOCKET       24  /* this file is a socket */

#define XADFILETYPE_MSDOSEXECRUNCHER 31  /* infile was an MSDOS exe cruncher */

#define XADSPECIALTYPE_UNIXDEVICE    1 /* xadSpecial entry is xadSpecialUnixDevice */
#define XADSPECIALTYPE_AMIGAADDRESS  2 /* xadSpecial entry is xadSpecialAmigaAddress */
#define XADSPECIALTYPE_CBM8BIT       3 /* xadSpecial entry is xadSpecialCBM8bit */

struct xadSpecialUnixDevice
{
  xadUINT32 xfis_MajorVersion;    /* major device version */
  xadUINT32 xfis_MinorVersion;    /* minor device version */
};

struct xadSpecialAmigaAddress
{
  xadUINT32 xfis_JumpAddress;     /* code execution start address */
  xadUINT32 xfis_DecrunchAddress; /* decrunch start of code */
};

struct xadSpecialCBM8bit
{
  xadUINT8 xfis_FileType;        /* File type XADCBM8BITTYPE_xxx */
  xadUINT8 xfis_RecordLength;    /* record length if relative file */
};
#define XADCBM8BITTYPE_UNKNOWN  0x00    /*        Unknown / Unused */
#define XADCBM8BITTYPE_BASIC    0x01    /* Tape - BASIC program file */
#define XADCBM8BITTYPE_DATA     0x02    /* Tape - Data block (SEQ file) */
#define XADCBM8BITTYPE_FIXED    0x03    /* Tape - Fixed addres program file */
#define XADCBM8BITTYPE_SEQDATA  0x04    /* Tape - Sequential data file */
#define XADCBM8BITTYPE_SEQ      0x81    /* Disk - Sequential file "SEQ" */
#define XADCBM8BITTYPE_PRG      0x82    /* Disk - Program file "PRG" */
#define XADCBM8BITTYPE_USR      0x83    /* Disk - User-defined file "USR" */
#define XADCBM8BITTYPE_REL      0x84    /* Disk - Relative records file "REL" */
#define XADCBM8BITTYPE_CBM      0x85    /* Disk - CBM (partition) "CBM" */

struct xadSpecial
{
  xadUINT32          xfis_Type; /* XADSPECIALTYPE to define type of block (V11) */
  struct xadSpecial *xfis_Next; /* pointer to next entry */
  union
  {
    struct xadSpecialUnixDevice   xfis_UnixDevice;
    struct xadSpecialAmigaAddress xfis_AmigaAddress;
    struct xadSpecialCBM8bit      xfis_CBM8bit;
  } xfis_Data;
};

/* Multiuser fields (xfi_OwnerUID, xfi_OwnerUID, xfi_UserName, xfi_GroupName)
   and multiuser bits (see <dos/dos.h>) are currently not supported with normal
   Amiga filesystem. But the clients support them, if archive format holds
   such information.

   The protection bits (all 3 fields) should always be set using the
   xadConvertProtection procedure. Call it with as much protection information
   as possible. It extracts the relevant data at best (and also sets the 2 flags).
   DO NOT USE these fields directly, but always through xadConvertProtection
   call.
*/

#define XADFIB_CRYPTED          0 /* entry is crypted */
#define XADFIB_DIRECTORY        1 /* entry is a directory */
#define XADFIB_LINK             2 /* entry is a link */
#define XADFIB_INFOTEXT         3 /* file is an information text */
#define XADFIB_GROUPED          4 /* file is in a crunch group */
#define XADFIB_ENDOFGROUP       5 /* crunch group ends here */
#define XADFIB_NODATE           6 /* no date supported, CURRENT date is set */
#define XADFIB_DELETED          7 /* file is marked as deleted (V3) */
#define XADFIB_SEEKDATAPOS      8 /* before unarchiving the datapos is set (V3) */
#define XADFIB_NOFILENAME       9 /* there was no filename, using internal one (V6) */
#define XADFIB_NOUNCRUNCHSIZE  10 /* file size is unknown and thus set to zero (V6) */
#define XADFIB_PARTIALFILE     11 /* file is only partial (V6) */
#define XADFIB_MACDATA         12 /* file is Apple data fork (V7) */
#define XADFIB_MACRESOURCE     13 /* file is Apple resource fork (V7) */
#define XADFIB_EXTRACTONBUILD  14 /* allows extract file during scanning (V10) */
#define XADFIB_UNIXPROTECTION  15 /* UNIX protection bits are present (V11) */
#define XADFIB_DOSPROTECTION   16 /* MSDOS protection bits are present (V11) */
#define XADFIB_ENTRYMAYCHANGE  17 /* this entry may change until GetInfo is finished (V11) */
#define XADFIB_XADSTRFILENAME  18 /* the xfi_FileName fields is an XAD string (V12) */
#define XADFIB_XADSTRLINKNAME  19 /* the xfi_LinkName fields is an XAD string (V12) */
#define XADFIB_XADSTRCOMMENT   20 /* the xfi_Comment fields is an XAD string (V12) */

#define XADFIF_CRYPTED          (1<<XADFIB_CRYPTED)
#define XADFIF_DIRECTORY        (1<<XADFIB_DIRECTORY)
#define XADFIF_LINK             (1<<XADFIB_LINK)
#define XADFIF_INFOTEXT         (1<<XADFIB_INFOTEXT)
#define XADFIF_GROUPED          (1<<XADFIB_GROUPED)
#define XADFIF_ENDOFGROUP       (1<<XADFIB_ENDOFGROUP)
#define XADFIF_NODATE           (1<<XADFIB_NODATE)
#define XADFIF_DELETED          (1<<XADFIB_DELETED)
#define XADFIF_SEEKDATAPOS      (1<<XADFIB_SEEKDATAPOS)
#define XADFIF_NOFILENAME       (1<<XADFIB_NOFILENAME)
#define XADFIF_NOUNCRUNCHSIZE   (1<<XADFIB_NOUNCRUNCHSIZE)
#define XADFIF_PARTIALFILE      (1<<XADFIB_PARTIALFILE)
#define XADFIF_MACDATA          (1<<XADFIB_MACDATA)
#define XADFIF_MACRESOURCE      (1<<XADFIB_MACRESOURCE)
#define XADFIF_EXTRACTONBUILD   (1<<XADFIB_EXTRACTONBUILD)
#define XADFIF_UNIXPROTECTION   (1<<XADFIB_UNIXPROTECTION)
#define XADFIF_DOSPROTECTION    (1<<XADFIB_DOSPROTECTION)
#define XADFIF_ENTRYMAYCHANGE   (1<<XADFIB_ENTRYMAYCHANGE)
#define XADFIF_XADSTRFILENAME   (1<<XADFIB_XADSTRFILENAME)
#define XADFIF_XADSTRLINKNAME   (1<<XADFIB_XADSTRLINKNAME)
#define XADFIF_XADSTRCOMMENT    (1<<XADFIB_XADSTRCOMMENT)

/* NOTE: the texts passed with that structure must not always be printable.
   Although the clients should add an additional (not counted) zero at the text
   end, the whole file may contain other unprintable stuff (e.g. for DMS).
   So when printing this texts do it on a byte for byte base including
   printability checks.
*/
struct xadTextInfo {
  struct xadTextInfo *  xti_Next;
  xadSize               xti_Size;  /* maybe zero - no text - e.g. when crypted */
  xadSTRPTR             xti_Text;  /* and there is no password in xadGetInfo() */
  xadUINT32             xti_Flags; /* see XADTIF_xxx defines */
};

#define XADTIB_CRYPTED          0 /* entry is empty, as data was crypted */
#define XADTIB_BANNER           1 /* text is a banner */
#define XADTIB_FILEDIZ          2 /* text is a file description */

#define XADTIF_CRYPTED          (1<<XADTIB_CRYPTED)
#define XADTIF_BANNER           (1<<XADTIB_BANNER)
#define XADTIF_FILEDIZ          (1<<XADTIB_FILEDIZ)

struct xadDiskInfo {
  struct xadDiskInfo *  xdi_Next;
  xadUINT32             xdi_EntryNumber;  /* number of entry */
  xadSTRPTR             xdi_EntryInfo;    /* additional archiver text */
  xadPTR                xdi_PrivateInfo;  /* client private, see XAD_OBJPRIVINFOSIZE */
  xadUINT32             xdi_Flags;        /* see XADDIF_xxx defines */
  xadUINT32             xdi_SectorSize;
  xadUINT32             xdi_TotalSectors; /* see devices/trackdisk.h */
  xadUINT32             xdi_Cylinders;    /* to find out what these */
  xadUINT32             xdi_CylSectors;   /* fields mean, they are equal */
  xadUINT32             xdi_Heads;        /* to struct DriveGeometry */
  xadUINT32             xdi_TrackSectors;
  xadUINT32             xdi_LowCyl;       /* lowest cylinder stored */
  xadUINT32             xdi_HighCyl;      /* highest cylinder stored */
  xadUINT32             xdi_BlockInfoSize;/* number of BlockInfo entries */
  xadUINT8 *            xdi_BlockInfo;    /* see XADBIF_xxx defines and XAD_OBJBLOCKENTRIES tag */
  struct xadTextInfo *  xdi_TextInfo;     /* linked list with info texts */
  xadSize               xdi_DataPos;      /* crunched data position (V3) */
};

/* BlockInfo points to a xadUINT8 field for every track from first sector of
   lowest cylinder to last sector of highest cylinder. When not used,
   pointer must be 0. Do not use it, when there are no entries!
   This is just for information. The applications still asks the client
   to unarchive whole cylinders and not archived blocks are cleared for
   unarchiving.
*/

#define XADDIB_CRYPTED            0 /* entry is crypted */
#define XADDIB_SEEKDATAPOS        1 /* before unarchiving the datapos is set (V3) */
#define XADDIB_SECTORLABELS       2 /* the clients delivers sector labels (V9) */
#define XADDIB_EXTRACTONBUILD     3 /* allows extract disk during scanning (V10) */
#define XADDIB_ENTRYMAYCHANGE     4 /* this entry may change until GetInfo is finished (V11) */

/* Some of the crunchers do not store all necessary information, so it
may be needed to guess some of them. Set the following flags in that case
and geometry check will ignore these fields. */
#define XADDIB_GUESSSECTORSIZE    5 /* sectorsize is guessed (V10) */
#define XADDIB_GUESSTOTALSECTORS  6 /* totalsectors number is guessed (V10) */
#define XADDIB_GUESSCYLINDERS     7 /* cylinder number is guessed */
#define XADDIB_GUESSCYLSECTORS    8 /* cylsectors is guessed */
#define XADDIB_GUESSHEADS         9 /* number of heads is guessed */
#define XADDIB_GUESSTRACKSECTORS 10 /* tracksectors is guessed */
#define XADDIB_GUESSLOWCYL       11 /* lowcyl is guessed */
#define XADDIB_GUESSHIGHCYL      12 /* highcyl is guessed */

/* If it is impossible to set some of the fields, you need to set some of
these flags. NOTE: XADDIB_NOCYLINDERS is really important, as this turns
off usage of lowcyl and highcyl keywords. When you have cylinder information,
you should not use these and instead use guess flags and calculate
possible values for the missing fields. */
#define XADDIB_NOCYLINDERS       15 /* cylinder number is not set */
#define XADDIB_NOCYLSECTORS      16 /* cylsectors is not set */
#define XADDIB_NOHEADS           17 /* number of heads is not set */
#define XADDIB_NOTRACKSECTORS    18 /* tracksectors is not set */
#define XADDIB_NOLOWCYL          19 /* lowcyl is not set */
#define XADDIB_NOHIGHCYL         20 /* highcyl is not set */

#define XADDIF_CRYPTED           (1<<XADDIB_CRYPTED)
#define XADDIF_SEEKDATAPOS       (1<<XADDIB_SEEKDATAPOS)
#define XADDIF_SECTORLABELS      (1<<XADDIB_SECTORLABELS)
#define XADDIF_EXTRACTONBUILD    (1<<XADDIB_EXTRACTONBUILD)
#define XADDIF_ENTRYMAYCHANGE    (1<<XADDIB_ENTRYMAYCHANGE)

#define XADDIF_GUESSSECTORSIZE   (1<<XADDIB_GUESSSECTORSIZE)
#define XADDIF_GUESSTOTALSECTORS (1<<XADDIB_GUESSTOTALSECTORS)
#define XADDIF_GUESSCYLINDERS    (1<<XADDIB_GUESSCYLINDERS)
#define XADDIF_GUESSCYLSECTORS   (1<<XADDIB_GUESSCYLSECTORS)
#define XADDIF_GUESSHEADS        (1<<XADDIB_GUESSHEADS)
#define XADDIF_GUESSTRACKSECTORS (1<<XADDIB_GUESSTRACKSECTORS)
#define XADDIF_GUESSLOWCYL       (1<<XADDIB_GUESSLOWCYL)
#define XADDIF_GUESSHIGHCYL      (1<<XADDIB_GUESSHIGHCYL)

#define XADDIF_NOCYLINDERS       (1<<XADDIB_NOCYLINDERS)
#define XADDIF_NOCYLSECTORS      (1<<XADDIB_NOCYLSECTORS)
#define XADDIF_NOHEADS           (1<<XADDIB_NOHEADS)
#define XADDIF_NOTRACKSECTORS    (1<<XADDIB_NOTRACKSECTORS)
#define XADDIF_NOLOWCYL          (1<<XADDIB_NOLOWCYL)
#define XADDIF_NOHIGHCYL         (1<<XADDIB_NOHIGHCYL)

/* defines for BlockInfo */
#define XADBIB_CLEARED          0 /* this block was cleared for archiving */
#define XADBIB_UNUSED           1 /* this block was not archived */

#define XADBIF_CLEARED          (1<<XADBIB_CLEARED)
#define XADBIF_UNUSED           (1<<XADBIB_UNUSED)

/************************************************************************
*                                                                       *
*    progress report stuff                                              *
*                                                                       *
************************************************************************/

struct xadProgressInfo {
  xadUINT32             xpi_Mode;       /* work modus */
  struct xadClient *    xpi_Client;     /* the client doing the work */
  struct xadDiskInfo *  xpi_DiskInfo;   /* current diskinfo, for disks */
  struct xadFileInfo *  xpi_FileInfo;   /* current info for files */
  xadSize               xpi_CurrentSize;/* current filesize */
  xadUINT32             xpi_LowCyl;     /* for disks only */
  xadUINT32             xpi_HighCyl;    /* for disks only */
  xadUINT32             xpi_Status;     /* see XADPIF flags */
  xadERROR              xpi_Error;      /* any of the error codes */
  xadSTRPTR             xpi_FileName;   /* name of file to overwrite (V2) */
  xadSTRPTR             xpi_NewName;    /* new name buffer, passed by hook (V2) */
};
/* NOTE: For disks CurrentSize is Sector*SectorSize, where SectorSize can
be found in xadDiskInfo structure. So you may output the sector value. */

/* different progress modes */
#define XADPMODE_ASK            1
#define XADPMODE_PROGRESS       2
#define XADPMODE_END            3
#define XADPMODE_ERROR          4
#define XADPMODE_NEWENTRY       5 /* (V10) */
#define XADPMODE_GETINFOEND     6 /* (V11) */

/* flags for progress hook and ProgressInfo status field */
#define XADPIB_OVERWRITE         0 /* overwrite the file */
#define XADPIB_MAKEDIRECTORY     1 /* create the directory */
#define XADPIB_IGNOREGEOMETRY    2 /* ignore drive geometry */
#define XADPIB_ISDIRECTORY       3 /* destination is a directory (V10) */
#define XADPIB_RENAME           10 /* rename the file (V2) */
#define XADPIB_OK               16 /* all ok, proceed */
#define XADPIB_SKIP             17 /* skip file */

#define XADPIF_OVERWRITE        (1<<XADPIB_OVERWRITE)
#define XADPIF_MAKEDIRECTORY    (1<<XADPIB_MAKEDIRECTORY)
#define XADPIF_IGNOREGEOMETRY   (1<<XADPIB_IGNOREGEOMETRY)
#define XADPIF_ISDIRECTORY      (1<<XADPIB_ISDIRECTORY)
#define XADPIF_RENAME           (1<<XADPIB_RENAME)
#define XADPIF_OK               (1<<XADPIB_OK)
#define XADPIF_SKIP             (1<<XADPIB_SKIP)

/************************************************************************
*                                                                       *
*    errors                                                             *
*                                                                       *
************************************************************************/

#define XADERR_OK               0x0000 /* no error */
#define XADERR_UNKNOWN          0x0001 /* unknown error */
#define XADERR_INPUT            0x0002 /* input data buffers border exceeded */
#define XADERR_OUTPUT           0x0003 /* output data buffers border exceeded */
#define XADERR_BADPARAMS        0x0004 /* function called with illegal parameters */
#define XADERR_NOMEMORY         0x0005 /* not enough memory available */
#define XADERR_ILLEGALDATA      0x0006 /* data is corrupted */
#define XADERR_NOTSUPPORTED     0x0007 /* command is not supported */
#define XADERR_RESOURCE         0x0008 /* required resource missing */
#define XADERR_DECRUNCH         0x0009 /* error on decrunching */
#define XADERR_FILETYPE         0x000A /* unknown file type */
#define XADERR_OPENFILE         0x000B /* opening file failed */
#define XADERR_SKIP             0x000C /* file, disk has been skipped */
#define XADERR_BREAK            0x000D /* user break in progress hook */
#define XADERR_FILEEXISTS       0x000E /* file already exists */
#define XADERR_PASSWORD         0x000F /* missing or wrong password */
#define XADERR_MAKEDIR          0x0010 /* could not create directory */
#define XADERR_CHECKSUM         0x0011 /* wrong checksum */
#define XADERR_VERIFY           0x0012 /* verify failed (disk hook) */
#define XADERR_GEOMETRY         0x0013 /* wrong drive geometry */
#define XADERR_DATAFORMAT       0x0014 /* unknown data format */
#define XADERR_EMPTY            0x0015 /* source contains no files */
#define XADERR_FILESYSTEM       0x0016 /* unknown filesystem */
#define XADERR_FILEDIR          0x0017 /* name of file exists as directory */
#define XADERR_SHORTBUFFER      0x0018 /* buffer was too short */
#define XADERR_ENCODING         0x0019 /* text encoding was defective */

/************************************************************************
*                                                                       *
*    characterset and filename conversion                               *
*                                                                       *
************************************************************************/

#define CHARSET_HOST                      0 /* this is the ONLY destination setting for clients! */

#define CHARSET_UNICODE_UCS2_HOST         10 /* 16bit Unicode (usually no source type) */
#define CHARSET_UNICODE_UCS2_BIGENDIAN    11 /* 16bit Unicode big endian storage */
#define CHARSET_UNICODE_UCS2_LITTLEENDIAN 12 /* 16bit Unicode little endian storage */
#define CHARSET_UNICODE_UTF8              13 /* variable size unicode encoding */

/* all the 1xx types are generic types which also maybe a bit dynamic */
#define CHARSET_AMIGA                   100 /* the default Amiga charset */
#define CHARSET_MSDOS                   101 /* the default MSDOS charset */
#define CHARSET_MACOS                   102 /* the default MacOS charset */
#define CHARSET_C64                     103 /* the default C64 charset */
#define CHARSET_ATARI_ST                104 /* the default Atari ST charset */
#define CHARSET_WINDOWS                 105 /* the default Windows charset */

/* all the 2xx to 9xx types are real charsets, use them whenever you know
   what the data really is */
#define CHARSET_ASCII                   200 /* the lower 7 bits of ASCII charsets */
#define CHARSET_ISO_8859_1              201 /* the base charset */
#define CHARSET_ISO_8859_15             215 /* Euro-sign fixed ISO variant */
#define CHARSET_ATARI_ST_US             300 /* Atari ST (US) charset */
#define CHARSET_PETSCII_C64_LC          301 /* C64 lower case charset */
#define CHARSET_CODEPAGE_437            400 /* IBM Codepage 437 charset */
#define CHARSET_CODEPAGE_1252           401 /* Windows Codepage 1252 charset */

/************************************************************************
*                                                                       *
*    client related stuff                                               *
*                                                                       *
************************************************************************/

struct xadClient {
  struct xadClient * xc_Next;
  xadUINT16          xc_Version;    /* set to XADCLIENT_VERSION */
  xadUINT16          xc_MasterVersion;
  xadUINT16          xc_ClientVersion;
  xadUINT16          xc_ClientRevision;
  xadSize            xc_RecogSize;  /* needed size to recog the type */
  xadUINT32          xc_Flags;      /* see XADCF_xxx defines */
  xadUINT32          xc_Identifier; /* ID of internal clients */
  const char       * xc_ArchiverName;
  xadBOOL         (* xc_RecogData)(xadSize size, const xadUINT8 *data,
                  struct xadMasterBase *xadMasterBase);
  xadERROR        (* xc_GetInfo)(struct xadArchiveInfo *ai,
                  struct xadMasterBase *xadMasterBase);
  xadERROR        (* xc_UnArchive)(struct xadArchiveInfo *ai,
                  struct xadMasterBase *xadMasterBase);
  void            (* xc_Free)(struct xadArchiveInfo *ai,
                  struct xadMasterBase *xadMasterBase);
};

/* xc_RecogData returns 1 when recognized and 0 when not, all the others
   return 0 when ok and XADERR values on error. xc_Free has no return
   value.

   Filesystem clients need to clear xc_RecogSize and xc_RecogData. The
   recognition is automatically done by GetInfo. XADERR_FILESYSTEM is
   returned in case of unknown format. If it is known detection should
   go on and any other code may be returned, if it fails.
   The field xc_ArchiverName means xc_FileSystemName for filesystem
   clients.
*/

#define XADCLIENT_VERSION       1

#define XADCB_FILEARCHIVER       0 /* archiver is a file archiver */
#define XADCB_DISKARCHIVER       1 /* archiver is a disk archiver */
#define XADCB_EXTERN             2 /* external client, set by xadmaster */
#define XADCB_FILESYSTEM         3 /* filesystem clients (V5) */
#define XADCB_NOCHECKSIZE        4 /* do not check size for recog call (V6) */
#define XADCB_DATACRUNCHER       5 /* file archiver is plain data file (V11) */
#define XADCB_EXECRUNCHER        6 /* file archiver is executable file (V11) */
#define XADCB_ADDRESSCRUNCHER    7 /* file archiver is address crunched file (V11) */
#define XADCB_LINKER             8 /* file archiver is a linker file (V11) */
#define XADCB_FREEXADSTRINGS    25 /* master frees XAD strings (V12) */
#define XADCB_FREESPECIALINFO   26 /* master frees xadSpecial  structures (V11) */
#define XADCB_FREESKIPINFO      27 /* master frees xadSkipInfo structures (V3) */
#define XADCB_FREETEXTINFO      28 /* master frees xadTextInfo structures (V2) */
#define XADCB_FREETEXTINFOTEXT  29 /* master frees xadTextInfo text block (V2) */
#define XADCB_FREEFILEINFO      30 /* master frees xadFileInfo structures (V2) */
#define XADCB_FREEDISKINFO      31 /* master frees xadDiskInfo structures (V2) */

#define XADCF_FILEARCHIVER      (1UL<<XADCB_FILEARCHIVER)
#define XADCF_DISKARCHIVER      (1UL<<XADCB_DISKARCHIVER)
#define XADCF_EXTERN            (1UL<<XADCB_EXTERN)
#define XADCF_FILESYSTEM        (1UL<<XADCB_FILESYSTEM)
#define XADCF_NOCHECKSIZE       (1UL<<XADCB_NOCHECKSIZE)
#define XADCF_DATACRUNCHER      (1UL<<XADCB_DATACRUNCHER)
#define XADCF_EXECRUNCHER       (1UL<<XADCB_EXECRUNCHER)
#define XADCF_ADDRESSCRUNCHER   (1UL<<XADCB_ADDRESSCRUNCHER)
#define XADCF_LINKER            (1UL<<XADCB_LINKER)
#define XADCF_FREEXADSTRINGS    (1UL<<XADCB_FREEXADSTRINGS)
#define XADCF_FREESPECIALINFO   (1UL<<XADCB_FREESPECIALINFO)
#define XADCF_FREESKIPINFO      (1UL<<XADCB_FREESKIPINFO)
#define XADCF_FREETEXTINFO      (1UL<<XADCB_FREETEXTINFO)
#define XADCF_FREETEXTINFOTEXT  (1UL<<XADCB_FREETEXTINFOTEXT)
#define XADCF_FREEFILEINFO      (1UL<<XADCB_FREEFILEINFO)
#define XADCF_FREEDISKINFO      (1UL<<XADCB_FREEDISKINFO)

/* The types 5 to 9 always need XADCB_FILEARCHIVER set also. These only specify
the type of the archiver somewhat better. Do not mix real archivers and these
single file data clients. */

/************************************************************************
*                                                                       *
*    client ID's                                                        *
*                                                                       *
************************************************************************/

/* If an external client has set the xc_Identifier field, the internal
client is replaced. */

/* disk archivers start with 1000 */
#define XADCID_XMASH                    1000
#define XADCID_SUPERDUPER3              1001
#define XADCID_XDISK                    1002
#define XADCID_PACKDEV                  1003
#define XADCID_ZOOM                     1004
#define XADCID_ZOOM5                    1005
#define XADCID_CRUNCHDISK               1006
#define XADCID_PACKDISK                 1007
#define XADCID_MDC                      1008
#define XADCID_COMPDISK                 1009
#define XADCID_LHWARP                   1010
#define XADCID_SAVAGECOMPRESSOR         1011
#define XADCID_WARP                     1012
#define XADCID_GDC                      1013
#define XADCID_DCS                      1014
#define XADCID_MSA                      1015
#define XADCID_COP                      1016
#define XADCID_DIMP                     1017
#define XADCID_DIMPSFX                  1018

/* file archivers start with 5000 */
#define XADCID_TAR                      5000
#define XADCID_SDSSFX                   5001
#define XADCID_LZX                      5002
#define XADCID_MXMSIMPLEARC             5003
#define XADCID_LHPAK                    5004
#define XADCID_AMIGAPLUSUNPACK          5005
#define XADCID_AMIPACK                  5006
#define XADCID_LHA                      5007
#define XADCID_LHASFX                   5008
#define XADCID_PCOMPARC                 5009
#define XADCID_SOMNI                    5010
#define XADCID_LHSFX                    5011
#define XADCID_XPKARCHIVE               5012
#define XADCID_SHRINK                   5013
#define XADCID_SPACK                    5014
#define XADCID_SPACKSFX                 5015
#define XADCID_ZIP                      5016
#define XADCID_WINZIPEXE                5017
#define XADCID_GZIP                     5018
#define XADCID_ARC                      5019
#define XADCID_ZOO                      5020
#define XADCID_LHAEXE                   5021
#define XADCID_ARJ                      5022
#define XADCID_ARJEXE                   5023
#define XADCID_ZIPEXE                   5024
#define XADCID_LHF                      5025
#define XADCID_COMPRESS                 5026
#define XADCID_ACE                      5027
#define XADCID_ACEEXE                   5028
#define XADCID_GZIPSFX                  5029
#define XADCID_HA                       5030
#define XADCID_SQ                       5031
#define XADCID_LHAC64SFX                5032
#define XADCID_SIT                      5033
#define XADCID_SIT5                     5034
#define XADCID_SIT5EXE                  5035
#define XADCID_MACBINARY                5036
#define XADCID_CPIO                     5037
#define XADCID_PACKIT                   5038
#define XADCID_CRUNCH                   5039
#define XADCID_ARCCBM                   5040
#define XADCID_ARCCBMSFX                5041
#define XADCID_CAB                      5042
#define XADCID_CABMSEXE                 5043
#define XADCID_RPM                      5044
#define XADCID_BZIP2                    5045
#define XADCID_BZIP2SFX                 5046
#define XADCID_BZIP                     5047
#define XADCID_IDPAK                    5048
#define XADCID_IDWAD                    5049
#define XADCID_IDWAD2                   5050
#define XADCID_RAR                      5051
#define XADCID_BINHEX                   5052
#define XADCID_SEVENZIP                 5053

/* filesystem client start with 8000 */
#define XADCID_FSAMIGA                  8000
#define XADCID_FSSANITYOS               8001
#define XADCID_FSFAT                    8002
#define XADCID_FSTRDOS                  8003

/* mixed archivers start with 9000 */
#define XADCID_DMS                      9000
#define XADCID_DMSSFX                   9001

/* XAD prototypes */

#ifndef XAD_NO_PROTOTYPES
/*** BEGIN auto-generated section (EXTERNAL VARARGS) */
extern struct xadMasterBase *xadOpenLibrary( xadINT32 version );
extern void xadCloseLibrary(struct xadMasterBase *);
extern xadERROR xadAddDiskEntry(struct xadMasterBase *xadMasterBase, struct xadDiskInfo *di, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadAddDiskEntryA(struct xadMasterBase *xadMasterBase, struct xadDiskInfo *di, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern xadERROR xadAddFileEntry(struct xadMasterBase *xadMasterBase, struct xadFileInfo *fi, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadAddFileEntryA(struct xadMasterBase *xadMasterBase, struct xadFileInfo *fi, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern xadPTR xadAllocObject(struct xadMasterBase *xadMasterBase, xadUINT32 type, xadTag tag, ...);
extern xadPTR xadAllocObjectA(struct xadMasterBase *xadMasterBase, xadUINT32 type, xadTAGPTR tags);
extern xadPTR xadAllocVec(struct xadMasterBase *xadMasterBase, xadSize size, xadUINT32 flags);
extern xadUINT16 xadCalcCRC16(struct xadMasterBase *xadMasterBase, xadUINT16 id, xadUINT16 init, xadSize size, const xadUINT8 *buffer);
extern xadUINT32 xadCalcCRC32(struct xadMasterBase *xadMasterBase, xadUINT32 id, xadUINT32 init, xadSize size, const xadUINT8 *buffer);
extern xadERROR xadConvertDates(struct xadMasterBase *xadMasterBase, xadTag tag, ...);
extern xadERROR xadConvertDatesA(struct xadMasterBase *xadMasterBase, xadTAGPTR tags);
extern xadSTRPTR xadConvertName(struct xadMasterBase *xadMasterBase, xadUINT32 charset, xadTag tag, ...);
extern xadSTRPTR xadConvertNameA(struct xadMasterBase *xadMasterBase, xadUINT32 charset, xadTAGPTR tags);
extern xadERROR xadConvertProtection(struct xadMasterBase *xadMasterBase, xadTag tag, ...);
extern xadERROR xadConvertProtectionA(struct xadMasterBase *xadMasterBase, xadTAGPTR tags);
extern void xadCopyMem(struct xadMasterBase *xadMasterBase, const void *s, xadPTR d, xadSize size);
extern xadERROR xadDiskUnArc(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadDiskUnArcA(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern xadERROR xadFileUnArc(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadFileUnArcA(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern void xadFreeHookAccess(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTag tag, ...);
extern void xadFreeHookAccessA(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern void xadFreeInfo(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai);
extern void xadFreeObject(struct xadMasterBase *xadMasterBase, xadPTR object, xadTag tag, ...);
extern void xadFreeObjectA(struct xadMasterBase *xadMasterBase, xadPTR object, xadTAGPTR tags);
extern struct xadClient * xadGetClientInfo(struct xadMasterBase *xadMasterBase);
extern xadSTRPTR xadGetDefaultName(struct xadMasterBase *xadMasterBase, xadTag tag, ...);
extern xadSTRPTR xadGetDefaultNameA(struct xadMasterBase *xadMasterBase, xadTAGPTR tags);
extern xadERROR xadGetDiskInfo(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadGetDiskInfoA(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern xadSTRPTR xadGetErrorText(struct xadMasterBase *xadMasterBase, xadERROR errnum);
extern xadERROR xadGetFilename(struct xadMasterBase *xadMasterBase, xadUINT32 buffersize, xadSTRPTR buffer, const xadSTRING *path, const xadSTRING *name, xadTag tag, ...);
extern xadERROR xadGetFilenameA(struct xadMasterBase *xadMasterBase, xadUINT32 buffersize, xadSTRPTR buffer, const xadSTRING *path, const xadSTRING *name, xadTAGPTR tags);
extern xadERROR xadGetInfo(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadGetInfoA(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern xadERROR xadGetHookAccess(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadGetHookAccessA(struct xadMasterBase *xadMasterBase, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern struct xadSystemInfo * xadGetSystemInfo(struct xadMasterBase *xadMasterBase);
extern xadERROR xadHookAccess(struct xadMasterBase *xadMasterBase, xadUINT32 command, xadSignSize data, xadPTR buffer, struct xadArchiveInfo *ai);
extern xadERROR xadHookTagAccess(struct xadMasterBase *xadMasterBase, xadUINT32 command, xadSignSize data, xadPTR buffer, struct xadArchiveInfo *ai, xadTag tag, ...);
extern xadERROR xadHookTagAccessA(struct xadMasterBase *xadMasterBase, xadUINT32 command, xadSignSize data, xadPTR buffer, struct xadArchiveInfo *ai, xadTAGPTR tags);
extern struct xadClient * xadRecogFile(struct xadMasterBase *xadMasterBase, xadSize size, const void *mem, xadTag tag, ...);
extern struct xadClient * xadRecogFileA(struct xadMasterBase *xadMasterBase, xadSize size, const void *mem, xadTAGPTR tags);
/*** END auto-generated section */
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_XADMASTER_H */
