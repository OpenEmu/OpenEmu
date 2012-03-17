// Gens: ASPI definitions

/* This file should be 100% source compatible according to MSes docs and
 * Adaptecs docs */

#ifndef GENS_ASPI_H
#define GENS_ASPI_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

// Needed for uint_*.
#include <stdint.h>

// On Windows, pshpack1.h is needed to byte-align structs.
#ifdef GENS_OS_WIN32
#include "pshpack1.h"
#endif /* GENS_OS_WIN32 */

//***************************************************************************
//                          %%% TARGET STATUS VALUES %%%
//***************************************************************************
#define STATUS_GOOD     0x00    // Status Good
#define STATUS_CHKCOND  0x02    // Check Condition
#define STATUS_CONDMET  0x04    // Condition Met
#define STATUS_BUSY     0x08    // Busy
#define STATUS_INTERM   0x10    // Intermediate
#define STATUS_INTCDMET 0x14    // Intermediate-condition met
#define STATUS_RESCONF  0x18    // Reservation conflict
#define STATUS_COMTERM  0x22    // Command Terminated
#define STATUS_QFULL    0x28    // Queue full

//***************************************************************************
//                      %%% SCSI MISCELLANEOUS EQUATES %%%
//***************************************************************************
#define MAXLUN          7       // Maximum Logical Unit Id
#define MAXTARG         7       // Maximum Target Id
#define MAX_SCSI_LUNS   64      // Maximum Number of SCSI LUNs
#define MAX_NUM_HA      8       // Maximum Number of SCSI HA's

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
//
//                          %%% SCSI COMMAND OPCODES %%%
//
///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

//***************************************************************************
//               %%% Commands for all Device Types %%%
//***************************************************************************
#define SCSI_CHANGE_DEF 0x40    // Change Definition (Optional)
#define SCSI_COMPARE    0x39    // Compare (O)
#define SCSI_COPY       0x18    // Copy (O)
#define SCSI_COP_VERIFY 0x3A    // Copy and Verify (O)
#define SCSI_INQUIRY    0x12    // Inquiry (MANDATORY)
#define SCSI_LOG_SELECT 0x4C    // Log Select (O)
#define SCSI_LOG_SENSE  0x4D    // Log Sense (O)
#define SCSI_MODE_SEL6  0x15    // Mode Select 6-byte (Device Specific)
#define SCSI_MODE_SEL10 0x55    // Mode Select 10-byte (Device Specific)
#define SCSI_MODE_SEN6  0x1A    // Mode Sense 6-byte (Device Specific)
#define SCSI_MODE_SEN10 0x5A    // Mode Sense 10-byte (Device Specific)
#define SCSI_READ_BUFF  0x3C    // Read Buffer (O)
#define SCSI_REQ_SENSE  0x03    // Request Sense (MANDATORY)
#define SCSI_SEND_DIAG  0x1D    // Send Diagnostic (O)
#define SCSI_TST_U_RDY  0x00    // Test Unit Ready (MANDATORY)
#define SCSI_WRITE_BUFF 0x3B    // Write Buffer (O)

//***************************************************************************
//            %%% Commands Unique to Direct Access Devices %%%
//***************************************************************************
#define SCSI_COMPARE    0x39    // Compare (O)
#define SCSI_FORMAT     0x04    // Format Unit (MANDATORY)
#define SCSI_LCK_UN_CAC 0x36    // Lock Unlock Cache (O)
#define SCSI_PREFETCH   0x34    // Prefetch (O)
#define SCSI_MED_REMOVL 0x1E    // Prevent/Allow medium Removal (O)
#define SCSI_READ6      0x08    // Read 6-byte (MANDATORY)
#define SCSI_READ10     0x28    // Read 10-byte (MANDATORY)
#define SCSI_RD_CAPAC   0x25    // Read Capacity (MANDATORY)
#define SCSI_RD_DEFECT  0x37    // Read Defect Data (O)
#define SCSI_READ_LONG  0x3E    // Read Long (O)
#define SCSI_REASS_BLK  0x07    // Reassign Blocks (O)
#define SCSI_RCV_DIAG   0x1C    // Receive Diagnostic Results (O)
#define SCSI_RELEASE    0x17    // Release Unit (MANDATORY)
#define SCSI_REZERO     0x01    // Rezero Unit (O)
#define SCSI_SRCH_DAT_E 0x31    // Search Data Equal (O)
#define SCSI_SRCH_DAT_H 0x30    // Search Data High (O)
#define SCSI_SRCH_DAT_L 0x32    // Search Data Low (O)
#define SCSI_SEEK6      0x0B    // Seek 6-Byte (O)
#define SCSI_SEEK10     0x2B    // Seek 10-Byte (O)
#define SCSI_SEND_DIAG  0x1D    // Send Diagnostics (MANDATORY)
#define SCSI_SET_LIMIT  0x33    // Set Limits (O)
#define SCSI_START_STP  0x1B    // Start/Stop Unit (O)
#define SCSI_SYNC_CACHE 0x35    // Synchronize Cache (O)
#define SCSI_VERIFY     0x2F    // Verify (O)
#define SCSI_WRITE6     0x0A    // Write 6-Byte (MANDATORY)
#define SCSI_WRITE10    0x2A    // Write 10-Byte (MANDATORY)
#define SCSI_WRT_VERIFY 0x2E    // Write and Verify (O)
#define SCSI_WRITE_LONG 0x3F    // Write Long (O)
#define SCSI_WRITE_SAME 0x41    // Write Same (O)

//***************************************************************************
//          %%% Commands Unique to Sequential Access Devices %%%
//***************************************************************************
#define SCSI_ERASE      0x19    // Erase (MANDATORY)
#define SCSI_LOAD_UN    0x1B    // Load/Unload (O)
#define SCSI_LOCATE     0x2B    // Locate (O)
#define SCSI_RD_BLK_LIM 0x05    // Read Block Limits (MANDATORY)
#define SCSI_READ_POS   0x34    // Read Position (O)
#define SCSI_READ_REV   0x0F    // Read Reverse (O)
#define SCSI_REC_BF_DAT 0x14    // Recover Buffer Data (O)
#define SCSI_RESERVE    0x16    // Reserve Unit (MANDATORY)
#define SCSI_REWIND     0x01    // Rewind (MANDATORY)
#define SCSI_SPACE      0x11    // Space (MANDATORY)
#define SCSI_VERIFY_T   0x13    // Verify (Tape) (O)
#define SCSI_WRT_FILE   0x10    // Write Filemarks (MANDATORY)

//***************************************************************************
//                %%% Commands Unique to Printer Devices %%%
//***************************************************************************
#define SCSI_PRINT      0x0A    // Print (MANDATORY)
#define SCSI_SLEW_PNT   0x0B    // Slew and Print (O)
#define SCSI_STOP_PNT   0x1B    // Stop Print (O)
#define SCSI_SYNC_BUFF  0x10    // Synchronize Buffer (O)

//***************************************************************************
//               %%% Commands Unique to Processor Devices %%%
//***************************************************************************
#define SCSI_RECEIVE    0x08        // Receive (O)
#define SCSI_SEND       0x0A        // Send (O)

//***************************************************************************
//              %%% Commands Unique to Write-Once Devices %%%
//***************************************************************************
#define SCSI_MEDIUM_SCN 0x38    // Medium Scan (O)
#define SCSI_SRCHDATE10 0x31    // Search Data Equal 10-Byte (O)
#define SCSI_SRCHDATE12 0xB1    // Search Data Equal 12-Byte (O)
#define SCSI_SRCHDATH10 0x30    // Search Data High 10-Byte (O)
#define SCSI_SRCHDATH12 0xB0    // Search Data High 12-Byte (O)
#define SCSI_SRCHDATL10 0x32    // Search Data Low 10-Byte (O)
#define SCSI_SRCHDATL12 0xB2    // Search Data Low 12-Byte (O)
#define SCSI_SET_LIM_10 0x33    // Set Limits 10-Byte (O)
#define SCSI_SET_LIM_12 0xB3    // Set Limits 10-Byte (O)
#define SCSI_VERIFY10   0x2F    // Verify 10-Byte (O)
#define SCSI_VERIFY12   0xAF    // Verify 12-Byte (O)
#define SCSI_WRITE12    0xAA    // Write 12-Byte (O)
#define SCSI_WRT_VER10  0x2E    // Write and Verify 10-Byte (O)
#define SCSI_WRT_VER12  0xAE    // Write and Verify 12-Byte (O)

//***************************************************************************
//                %%% Commands Unique to CD-ROM Devices %%%
//***************************************************************************
#define SCSI_PLAYAUD_10 0x45    // Play Audio 10-Byte (O)
#define SCSI_PLAYAUD_12 0xA5    // Play Audio 12-Byte 12-Byte (O)
#define SCSI_PLAYAUDMSF 0x47    // Play Audio MSF (O)
#define SCSI_PLAYA_TKIN 0x48    // Play Audio Track/Index (O)
#define SCSI_PLYTKREL10 0x49    // Play Track Relative 10-Byte (O)
#define SCSI_PAUSE_RESU 0x4B    // Pause/Resume Scan/Play (O)
#define SCSI_STOP_PL_SC 0x4E    // Stop Scan/Play (O)
#define SCSI_PLYTKREL12 0xA9    // Play Track Relative 12-Byte (O)
#define SCSI_READCDCAP  0x25    // Read CD-ROM Capacity (MANDATORY)
#define SCSI_READHEADER 0x44    // Read Header (O)
#define SCSI_SUBCHANNEL 0x42    // Read Subchannel (O)
#define SCSI_READ_TOC   0x43    // Read TOC (O)
#define SCSI_READ_MSF   0xB9    // Read CD MSF format (O)
#define SCSI_SET_SPEED  0xBB    // Set CD speed (O)
#define SCSI_GET_MCH_ST 0xBD    // Mechanic Status (O)
#define SCSI_READ_LBA   0xBE    // Read CD LBA format (O)

//***************************************************************************
//                %%% Commands Unique to Scanner Devices %%%
//***************************************************************************
#define SCSI_GETDBSTAT  0x34    // Get Data Buffer Status (O)
#define SCSI_GETWINDOW  0x25    // Get Window (O)
#define SCSI_OBJECTPOS  0x31    // Object Postion (O)
#define SCSI_SCAN       0x1B    // Scan (O)
#define SCSI_SETWINDOW  0x24    // Set Window (MANDATORY)

//***************************************************************************
//           %%% Commands Unique to Optical Memory Devices %%%
//***************************************************************************
#define SCSI_UpdateBlk  0x3D    // Update Block (O)

//***************************************************************************
//           %%% Commands Unique to Medium Changer Devices %%%
//***************************************************************************
#define SCSI_EXCHMEDIUM 0xA6    // Exchange Medium (O)
#define SCSI_INITELSTAT 0x07    // Initialize Element Status (O)
#define SCSI_POSTOELEM  0x2B    // Position to Element (O)
#define SCSI_REQ_VE_ADD 0xB5    // Request Volume Element Address (O)
#define SCSI_SENDVOLTAG 0xB6    // Send Volume Tag (O)

//***************************************************************************
//            %%% Commands Unique to Communication Devices %%%
//***************************************************************************
#define SCSI_GET_MSG_6  0x08    // Get Message 6-Byte (MANDATORY)
#define SCSI_GET_MSG_10 0x28    // Get Message 10-Byte (O)
#define SCSI_GET_MSG_12 0xA8    // Get Message 12-Byte (O)
#define SCSI_SND_MSG_6  0x0A    // Send Message 6-Byte (MANDATORY)
#define SCSI_SND_MSG_10 0x2A    // Send Message 10-Byte (O)
#define SCSI_SND_MSG_12 0xAA    // Send Message 12-Byte (O)

//\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/
//
//                    %%% END OF SCSI COMMAND OPCODES %%%
//
///\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/

//***************************************************************************
//                      %%% Request Sense Data Format %%%
//***************************************************************************
typedef struct
{
	uint8_t ErrorCode;		// Error Code (70H or 71H)
	uint8_t SegmentNum;		// Number of current segment descriptor
	uint8_t SenseKey;		// Sense Key(See bit definitions too)
	uint8_t InfoByte0;		// Information MSB
	uint8_t InfoByte1;		// Information MID
	uint8_t InfoByte2;		// Information MID
	uint8_t InfoByte3;		// Information LSB
	uint8_t AddSenLen;		// Additional Sense Length
	uint8_t ComSpecInf0;		// Command Specific Information MSB
	uint8_t ComSpecInf1;		// Command Specific Information MID
	uint8_t ComSpecInf2;		// Command Specific Information MID
	uint8_t ComSpecInf3;		// Command Specific Information LSB
	uint8_t AddSenseCode;		// Additional Sense Code
	uint8_t AddSenQual;		// Additional Sense Code Qualifier
	uint8_t FieldRepUCode;		// Field Replaceable Unit Code
	uint8_t SenKeySpec15;		// Sense Key Specific 15th byte
	uint8_t SenKeySpec16;		// Sense Key Specific 16th byte
	uint8_t SenKeySpec17;		// Sense Key Specific 17th byte
	uint8_t AddSenseBytes;		// Additional Sense Bytes
} SENSE_DATA_FMT;

//***************************************************************************
//                       %%% REQUEST SENSE ERROR CODE %%%
//***************************************************************************
#define SERROR_CURRENT  0x70    // Current Errors
#define SERROR_DEFERED  0x71    // Deferred Errors

//***************************************************************************
//                   %%% REQUEST SENSE BIT DEFINITIONS %%%
//***************************************************************************
#define SENSE_VALID     0x80    // Byte 0 Bit 7
#define SENSE_FILEMRK   0x80    // Byte 2 Bit 7
#define SENSE_EOM       0x40    // Byte 2 Bit 6
#define SENSE_ILI       0x20    // Byte 2 Bit 5

//***************************************************************************
//               %%% REQUEST SENSE SENSE KEY DEFINITIONS %%%
//***************************************************************************
#define KEY_NOSENSE     0x00    // No Sense
#define KEY_RECERROR    0x01    // Recovered Error
#define KEY_NOTREADY    0x02    // Not Ready
#define KEY_MEDIUMERR   0x03    // Medium Error
#define KEY_HARDERROR   0x04    // Hardware Error
#define KEY_ILLGLREQ    0x05    // Illegal Request
#define KEY_UNITATT     0x06    // Unit Attention
#define KEY_DATAPROT    0x07    // Data Protect
#define KEY_BLANKCHK    0x08    // Blank Check
#define KEY_VENDSPEC    0x09    // Vendor Specific
#define KEY_COPYABORT   0x0A    // Copy Abort
#define KEY_EQUAL       0x0C    // Equal (Search)
#define KEY_VOLOVRFLW   0x0D    // Volume Overflow
#define KEY_MISCOMP     0x0E    // Miscompare (Search)
#define KEY_RESERVED    0x0F    // Reserved

//***************************************************************************
//                %%% PERIPHERAL DEVICE TYPE DEFINITIONS %%%
//***************************************************************************
#define DTYPE_DASD      0x00    // Disk Device
#define DTYPE_SEQD      0x01    // Tape Device
#define DTYPE_PRNT      0x02    // Printer
#define DTYPE_PROC      0x03    // Processor
#define DTYPE_WORM      0x04    // Write-once read-multiple
#define DTYPE_CROM      0x05    // CD-ROM device
#define DTYPE_CDROM     0x05    // CD-ROM device
#define DTYPE_SCAN      0x06    // Scanner device
#define DTYPE_OPTI      0x07    // Optical memory device
#define DTYPE_JUKE      0x08    // Medium Changer device
#define DTYPE_COMM      0x09    // Communications device
#define DTYPE_RESL      0x0A    // Reserved (low)
#define DTYPE_RESH      0x1E    // Reserved (high)
#define DTYPE_UNKNOWN   0x1F    // Unknown or no device type

//***************************************************************************
//                %%% ANSI APPROVED VERSION DEFINITIONS %%%
//***************************************************************************
#define ANSI_MAYBE      0x0     // Device may or may not be ANSI approved stand
#define ANSI_SCSI1      0x1     // Device complies to ANSI X3.131-1986 (SCSI-1)
#define ANSI_SCSI2      0x2     // Device complies to SCSI-2
#define ANSI_RESLO      0x3     // Reserved (low)
#define ANSI_RESHI      0x7     // Reserved (high)

/* SCSI Miscellaneous Stuff */
#define SENSE_LEN			14
#define SRB_DIR_SCSI			0x00
#define SRB_POSTING			0x01
#define SRB_ENABLE_RESIDUAL_COUNT	0x04
#define SRB_DIR_IN			0x08
#define SRB_DIR_OUT			0x10

/* ASPI Command Definitions */
#define SC_HA_INQUIRY			0x00
#define SC_GET_DEV_TYPE			0x01
#define SC_EXEC_SCSI_CMD		0x02
#define SC_ABORT_SRB			0x03
#define SC_RESET_DEV			0x04
#define SC_SET_HA_PARMS			0x05
#define SC_GET_DISK_INFO		0x06

/* SRB status codes */
#define SS_PENDING			0x00
#define SS_COMP				0x01
#define SS_ABORTED			0x02
#define SS_ABORT_FAIL			0x03
#define SS_ERR				0x04

#define SS_INVALID_CMD			0x80
#define SS_INVALID_HA			0x81
#define SS_NO_DEVICE			0x82

#define SS_INVALID_SRB			0xE0
#define SS_OLD_MANAGER			0xE1
#define SS_BUFFER_ALIGN			0xE1 // Win32
#define SS_ILLEGAL_MODE			0xE2
#define SS_NO_ASPI			0xE3
#define SS_FAILED_INIT			0xE4
#define SS_ASPI_IS_BUSY			0xE5
#define SS_BUFFER_TO_BIG		0xE6
#define SS_MISMATCHED_COMPONENTS	0xE7 // DLLs/EXE version mismatch
#define SS_NO_ADAPTERS			0xE8
#define SS_INSUFFICIENT_RESOURCES	0xE9
#define SS_ASPI_IS_SHUTDOWN		0xEA
#define SS_BAD_INSTALL			0xEB


/* Host status codes */
#define HASTAT_OK			0x00
#define HASTAT_SEL_TO			0x11
#define HASTAT_DO_DU			0x12
#define HASTAT_BUS_FREE			0x13
#define HASTAT_PHASE_ERR		0x14

#define HASTAT_TIMEOUT			0x09
#define HASTAT_COMMAND_TIMEOUT		0x0B
#define HASTAT_MESSAGE_REJECT		0x0D
#define HASTAT_BUS_RESET		0x0E
#define HASTAT_PARITY_ERROR		0x0F
#define HASTAT_REQUEST_SENSE_FAILED	0x10


/* Additional definitions */
/* SCSI Miscellaneous Stuff */
#define SRB_EVENT_NOTIFY		0x40
#define RESIDUAL_COUNT_SUPPORTED	0x02
#define MAX_SRB_TIMEOUT			1080001u
#define DEFAULT_SRB_TIMEOUT		1080001u

/* These are defined by MS but not adaptec */
#define SRB_DATA_SG_LIST		0x02
#define WM_ASPIPOST			0x4D42


/* ASPI Command Definitions */
#define SC_RESCAN_SCSI_BUS		0x07
#define SC_GETSET_TIMEOUTS		0x08

/* SRB Status.. MS defined */
#define SS_SECURITY_VIOLATION		0xE2 // Replaces SS_INVALID_MODE
/*** END DEFS */

/* SRB - HOST ADAPTER INQUIRY - SC_HA_INQUIRY */
typedef struct tagSRB32_HaInquiry
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_HA_INQUIRY */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* ASPI request flags */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved, MUST = 0 */
	uint8_t  HA_Count;		/* Number of host adapters present */
	uint8_t  HA_SCSI_ID;		/* SCSI ID of host adapter */
	uint8_t  HA_ManagerId[16];	/* String describing the manager */
	uint8_t  HA_Identifier[16];	/* String describing the host adapter */
	uint8_t  HA_Unique[16];		/* Host Adapter Unique parameters */
	uint16_t HA_Rsvd1;
} SRB_HaInquiry, *PSRB_HaInquiry;

/* SRB - GET DEVICE TYPE - SC_GET_DEV_TYPE */
typedef struct tagSRB32_GDEVBlock
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_GET_DEV_TYPE */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* Reserved */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved */
	uint8_t  SRB_Target;		/* Target's SCSI ID */
	uint8_t  SRB_Lun;		/* Target's LUN number */
	uint8_t  SRB_DeviceType;	/* Target's peripheral device type */
	uint8_t  SRB_Rsvd1;
	
	// TODO: There seems to be an 8-byte overflow here,
	// which causes a crash with the default Win98SE ASPI driver.
	uint8_t  reserved[8];
} SRB_GDEVBlock, *PSRB_GDEVBlock;

/* SRB - EXECUTE SCSI COMMAND - SC_EXEC_SCSI_CMD */
typedef struct tagSRB32_ExecSCSICmd
{
	uint8_t SRB_Cmd;		/* ASPI command code = SC_EXEC_SCSI_CMD */
	uint8_t SRB_Status;		/* ASPI command status byte */
	uint8_t SRB_HaId;		/* ASPI host adapter number */
	uint8_t SRB_Flags;		/* ASPI request flags */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved */
	uint8_t SRB_Target;		/* Target's SCSI ID */
	uint8_t SRB_Lun;		/* Target's LUN number */
	uint16_t SRB_Rsvd1;		/* Reserved for Alignment */
	uint32_t SRB_BufLen;		/* Data Allocation Length */
	uint8_t *SRB_BufPointer;	/* Data Buffer Point */
	uint8_t SRB_SenseLen;		/* Sense Allocation Length */
	uint8_t SRB_CDBLen;		/* CDB Length */
	uint8_t SRB_HaStat;		/* Host Adapter Status */
	uint8_t SRB_TargStat;		/* Target Status */
	int     (*SRB_PostProc)(struct tagSRB32_ExecSCSICmd *s);	/* Post routine */
	void    *SRB_Rsvd2;		/* Reserved */
	uint8_t SRB_Rsvd3[16];		/* Reserved for expansion */
	uint8_t CDBByte[16];		/* SCSI CDB */
	uint8_t SenseArea[SENSE_LEN+2];	/* Request sense buffer - var length */
} SRB_ExecSCSICmd, *PSRB_ExecSCSICmd;

typedef struct tagSRB32_ExecSCSICmd2
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_EXEC_SCSI_CMD */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* ASPI request flags */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved */
	uint8_t  SRB_Target;		/* Target's SCSI ID */
	uint8_t  SRB_Lun;		/* Target's LUN number */
	uint16_t SRB_Rsvd1;		/* Reserved for Alignment */
	uint32_t SRB_BufLen;		/* Data Allocation Length */
	uint8_t  *SRB_BufPointer;	/* Data Buffer Point */
	uint8_t  SRB_SenseLen;		/* Sense Allocation Length */
	uint8_t  SRB_CDBLen;		/* CDB Length */
	uint8_t  SRB_HaStat;		/* Host Adapter Status */
	uint8_t  SRB_TargStat;		/* Target Status */
	void     (*SRB_PostProc)();	/* Post routine */
	void     *SRB_Rsvd2;		/* Reserved */
	uint8_t  SRB_Rsvd3[16];		/* Reserved for expansion */
	uint8_t  CDBByte[16];		/* SCSI CDB */
	uint8_t  SenseArea[SENSE_LEN+2]; /* Request sense buffer - var length */
} SRB_ExecSCSICmd2, *PSRB_ExecSCSICmd2;

/* SRB - ABORT AN ARB - SC_ABORT_SRB */
typedef struct tagSRB32_Abort
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_ABORT_SRB */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* Reserved */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved, MUST = 0 */
	void     *SRB_ToAbort;		/* Pointer to SRB to abort */
} SRB_Abort, *PSRB_Abort;

/* SRB - BUS DEVICE RESET - SC_RESET_DEV */
typedef struct tagSRB32_BusDeviceReset
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_RESET_DEV */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* Reserved */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved */
	uint8_t  SRB_Target;		/* Target's SCSI ID */
	uint8_t  SRB_Lun;		/* Target's LUN number */
	uint8_t  SRB_Rsvd1[12];		/* Reserved for Alignment */
	uint8_t  SRB_HaStat;		/* Host Adapter Status */
	uint8_t  SRB_TargStat;		/* Target Status */
	void     (*SRB_PostProc)();	/* Post routine */
	void     *SRB_Rsvd2;		/* Reserved */
	uint8_t  SRB_Rsvd3[32];		/* Reserved */
} SRB_BusDeviceReset, *PSRB_BusDeviceReset;

/* SRB - GET DISK INFORMATION - SC_GET_DISK_INFO */
typedef struct tagSRB32_GetDiskInfo
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_RESET_DEV */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* Reserved */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved */
	uint8_t  SRB_Target;		/* Target's SCSI ID */
	uint8_t  SRB_Lun;		/* Target's LUN number */
	uint8_t  SRB_DriveFlags;	/* Driver flags */
	uint8_t  SRB_Int13HDriveInfo;	/* Host Adapter Status */
	uint8_t  SRB_Heads;		/* Preferred number of heads trans */
	uint8_t  SRB_Sectors;		/* Preferred number of sectors trans */
	uint8_t  SRB_Rsvd1[10];		/* Reserved */
} SRB_GetDiskInfo, *PSRB_GetDiskInfo;

/* SRB header */
typedef struct tagSRB32_Header
{
	uint8_t  SRB_Cmd;		/* ASPI command code = SC_RESET_DEV */
	uint8_t  SRB_Status;		/* ASPI command status byte */
	uint8_t  SRB_HaId;		/* ASPI host adapter number */
	uint8_t  SRB_Flags;		/* Reserved */
	uint32_t SRB_Hdr_Rsvd;		/* Reserved */
} SRB_Header, *PSRB_Header;

typedef union tagSRB32
{
	SRB_Header		common;
	SRB_HaInquiry		inquiry;
	SRB_ExecSCSICmd		cmd;
	SRB_Abort		abort;
	SRB_BusDeviceReset	reset;
	SRB_GDEVBlock		devtype;
} SRB, *PSRB, *LPSRB;

// On Windows, pshpack1.h is needed to byte-pack structs.
// poppack.h turns off pshpack1.h, since byte-packing is only needed for the ASPI structs.
#ifdef GENS_OS_WIN32
#include "poppack.h"
#endif /* GENS_OS_WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* GENS_ASPI_H */
