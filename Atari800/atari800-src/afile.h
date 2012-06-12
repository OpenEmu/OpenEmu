#ifndef AFILE_H_
#define AFILE_H_

/* File types returned by AFILE_DetectFileType() and AFILE_OpenFile(). */
#define AFILE_ERROR      0
#define AFILE_ATR        1
#define AFILE_XFD        2
#define AFILE_ATR_GZ     3
#define AFILE_XFD_GZ     4
#define AFILE_DCM        5
#define AFILE_XEX        6
#define AFILE_BAS        7
#define AFILE_LST        8
#define AFILE_CART       9
#define AFILE_ROM        10
#define AFILE_CAS        11
#define AFILE_BOOT_TAPE  12
#define AFILE_STATE      13
#define AFILE_STATE_GZ   14
#define AFILE_PRO        15
#define AFILE_ATX        16

/* ATR format header */
struct AFILE_ATR_Header {
	unsigned char magic1;
	unsigned char magic2;
	unsigned char seccountlo;
	unsigned char seccounthi;
	unsigned char secsizelo;
	unsigned char secsizehi;
	unsigned char hiseccountlo;
	unsigned char hiseccounthi;
	unsigned char gash[7];
	unsigned char writeprotect;
};

/* First two bytes of an ATR file. */
#define AFILE_ATR_MAGIC1  0x96
#define AFILE_ATR_MAGIC2  0x02

/* Auto-detects file type and returns one of AFILE_* values. */
int AFILE_DetectFileType(const char *filename);

/* Auto-detects file type and mounts the file in the emulator.
   reboot: Atari800_Coldstart() for disks, cartridges and tapes
   diskno: drive number for disks (1-8)
   readonly: mount disks as read-only */
int AFILE_OpenFile(const char *filename, int reboot, int diskno, int readonly);

#endif /* AFILE_H_ */
