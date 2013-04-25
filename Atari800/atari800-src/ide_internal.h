#ifndef IDE_INTERNAL_H
#define IDE_INTERNAL_H

#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

struct ide_device;

typedef void EndTransferFunc(struct ide_device *);

struct ide_device {
    int bus_status;
    int bus_unit;

    int is_cdrom, is_cf;

    FILE *file;
    off_t filesize;
    int blocksize;

    int cylinders, heads, sectors;

    int drive_serial;
    char drive_serial_str[21];

    /* ide regs */
    uint8_t feature;
    uint8_t error;
    uint32_t nsector;
    uint8_t sector;
    uint8_t lcyl;
    uint8_t hcyl;
    /* other part of tf for lba48 support */
    uint8_t hob_feature;
    uint8_t hob_nsector;
    uint8_t hob_sector;
    uint8_t hob_lcyl;
    uint8_t hob_hcyl;

    uint8_t select;
    uint8_t status;

    /* set for lba48 access */
    uint8_t lba48;


    int64_t nb_sectors;
    int mult_sectors;

    int req_nb_sectors;
    EndTransferFunc *end_transfer_func;
    uint8_t *data_ptr, *data_end, *io_buffer;
    int io_buffer_size;

    /* CF-ATA extended error */
    uint8_t ext_error;

    /* CF-ATA metadata storage */
    uint32_t mdata_size;
    uint8_t *mdata_storage;
    int media_changed;

    /* 8-bit mode (ATA-1 devices and CF devices) */
    int cycle;
    int do_8bit;
    uint16_t data;
    uint16_t upperhalf[8];
};

/* Bits of HD_STATUS */
#define ERR_STAT        0x01
#define INDEX_STAT      0x02
#define ECC_STAT        0x04    /* Corrected error */
#define DRQ_STAT        0x08
#define SEEK_STAT       0x10
#define SRV_STAT        0x10
#define WRERR_STAT      0x20
#define READY_STAT      0x40
#define BUSY_STAT       0x80

/* Bits for HD_ERROR */
#define MARK_ERR        0x01    /* Bad address mark */
#define TRK0_ERR        0x02    /* couldn't find track 0 */
#define ABRT_ERR        0x04    /* Command aborted */
#define MCR_ERR         0x08    /* media change request */
#define ID_ERR          0x10    /* ID field not found */
#define MC_ERR          0x20    /* media changed */
#define ECC_ERR         0x40    /* Uncorrectable ECC error */
#define BBD_ERR         0x80    /* pre-EIDE meaning:  block marked bad */
#define ICRC_ERR        0x80    /* new meaning:  CRC error during transfer */

/* Bits of HD_NSECTOR */
#define CD              0x01
#define IO              0x02
#define REL             0x04
#define TAG_MASK        0xf8

#define IDE_CMD_RESET           0x04
#define IDE_CMD_DISABLE_IRQ     0x02


/* ATA/ATAPI COMMANDS:      (M) Mandatory   (O) Optional */

#define WIN_NOP                 0x00    /* (O) ATA1 nop */
#if 0
#define                         0x01    /*      Reserved */
#define                         0x02    /*      Reserved */
#endif
#define CFA_REQ_EXT_ERROR_CODE  0x03    /* (O) ATA4 cfa request ext. error code */
#if 0
#define                         0x04    /*      Reserved */
#define                         0x05    /*      Reserved */
#define                         0x06    /*      Reserved */
#define                         0x07    /*      Reserved */
#endif
#define WIN_SRST                0x08
#define WIN_DEVICE_RESET        0x08    /* (M) ATA4 device reset */
#if 0
#define                         0x09    /*      Reserved */
#define                         0x0a    /*      Reserved */
#define                         0x0b    /*      Reserved */
#define                         0x0c    /*      Reserved */
#define                         0x0d    /*      Reserved */
#define                         0x0e    /*      Reserved */
#define                         0x0f    /*      Reserved */
#endif
#define WIN_RECAL               0x10    /* (M) ATA1 recalibrate */
#if 0
#define                         0x11    /*      Reserved */
#define                         0x12    /*      Reserved */
#define                         0x13    /*      Reserved */
#define                         0x14    /*      Reserved */
#define                         0x15    /*      Reserved */
#define                         0x16    /*      Reserved */
#define                         0x17    /*      Reserved */
#define                         0x18    /*      Reserved */
#define                         0x19    /*      Reserved */
#define                         0x1a    /*      Reserved */
#define                         0x1b    /*      Reserved */
#define                         0x1c    /*      Reserved */
#define                         0x1d    /*      Reserved */
#define                         0x1e    /*      Reserved */
#define                         0x1f    /*      Reserved */
#endif
#define WIN_READ                0x20    /* (M) ATA1 read sector(s) w/ retry */
#define WIN_READ_ONCE           0x21    /* (M) ATA1 read sector(s) w/o retry */
#define WIN_READ_LONG           0x22    /* (M) ATA1 read long w/ retry */
#define WIN_READ_LONG_ONCE      0x23    /* (M) ATA1 read long w/o retry */
#define WIN_READ_EXT            0x24    /* (O) ATA6, LBA48 */
#define WIN_READDMA_EXT         0x25    /* (O) ATA6, LBA48 */
#define WIN_READDMA_QUEUED_EXT  0x26    /* (O) ATA6, LBA48 */
#define WIN_READ_NATIVE_MAX_EXT 0x27    /* (O) ATA6, LBA48 */
#if 0
#define                         0x28    /*      Reserved */
#endif
#define WIN_MULTREAD_EXT        0x29    /* (O) ATA6, LBA48 */
#if 0
#define                         0x2a    /*      Reserved */
#define                         0x2b    /*      Reserved */
#define                         0x2c    /*      Reserved */
#define                         0x2d    /*      Reserved */
#define                         0x2e    /*      Reserved */
#endif
#define WIN_READ_LOG_EXT        0x2f    /* (O) ATA6 read log ext */
#define WIN_WRITE               0x30    /* (M) ATA1 write sector(s) w/ retry */
#define WIN_WRITE_ONCE          0x31    /* (M) ATA1 write sector(s) w/o retry */
#define WIN_WRITE_LONG          0x32    /* (M) ATA1 write long w/ retry */
#define WIN_WRITE_LONG_ONCE     0x33    /* (M) ATA1 write long w/o retry */
#define WIN_WRITE_EXT           0x34    /* (O) ATA6, LBA48 */
#define WIN_WRITEDMA_EXT        0x35    /* (O) ATA6, LBA48 */
#define WIN_WRITEDMA_QUEUED_EXT 0x36    /* (O) ATA6, LBA48 */
#define WIN_SET_MAX_EXT         0x37    /* (O) ATA6, LBA48 */
#define CFA_WRITE_SECT_WO_ERASE 0x38    /* (O) ATA4 cfa write sectors w/o erase */
#define WIN_MULTWRITE_EXT       0x39    /* (O) ATA6, LBA48 */
#if 0
#define                         0x3a    /*      Reserved */
#define                         0x3b    /*      Reserved */
#endif
#define WIN_WRITE_VERIFY        0x3c    /* (O) ATA1 write verify, obsolete ATA4 */
#if 0
#define                         0x3d    /*      Reserved */
#define                         0x3e    /*      Reserved */
#define                         0x3f    /*      Reserved */
#endif
#define WIN_VERIFY              0x40    /* (M) ATA1 verify sector(s) w/ retry */
#define WIN_VERIFY_ONCE         0x41    /* (M) ATA1 verify sector(s) w/o retry */
#define WIN_VERIFY_EXT          0x42    /* (O) ATA6, LBA48 */
#if 0
#define                         0x43    /*      Reserved */
#define                         0x44    /*      Reserved */
#define                         0x45    /*      Reserved */
#define                         0x46    /*      Reserved */
#define                         0x47    /*      Reserved */
#define                         0x48    /*      Reserved */
#define                         0x49    /*      Reserved */
#define                         0x4a    /*      Reserved */
#define                         0x4b    /*      Reserved */
#define                         0x4c    /*      Reserved */
#define                         0x4d    /*      Reserved */
#define                         0x4e    /*      Reserved */
#define                         0x4f    /*      Reserved */
#endif
#define WIN_FORMAT              0x50    /* (M) ATA1 format track */
#if 0
#define                         0x51    /*      Reserved */
#define                         0x52    /*      Reserved */
#define                         0x53    /*      Reserved */
#define                         0x54    /*      Reserved */
#define                         0x55    /*      Reserved */
#define                         0x56    /*      Reserved */
#define                         0x57    /*      Reserved */
#define                         0x58    /*      Reserved */
#define                         0x59    /*      Reserved */
#define                         0x5a    /*      Reserved */
#define                         0x5b    /*      Reserved */
#define                         0x5c    /*      Reserved */
#define                         0x5d    /*      Reserved */
#define                         0x5e    /*      Reserved */
#define                         0x5f    /*      Reserved */
#endif
#define WIN_INIT                0x60 /* xxxx */
#if 0
#define                         0x61    /*      Reserved */
#define                         0x62    /*      Reserved */
#define                         0x63    /*      Reserved */
#define                         0x64    /*      Reserved */
#define                         0x65    /*      Reserved */
#define                         0x66    /*      Reserved */
#define                         0x67    /*      Reserved */
#define                         0x68    /*      Reserved */
#define                         0x69    /*      Reserved */
#define                         0x6a    /*      Reserved */
#define                         0x6b    /*      Reserved */
#define                         0x6c    /*      Reserved */
#define                         0x6d    /*      Reserved */
#define                         0x6e    /*      Reserved */
#define                         0x6f    /*      Reserved */
#endif
#define WIN_SEEK                0x70    /* (M) ATA1 seek */
#if 0
#define                         0x71    /*      Reserved */
#define                         0x72    /*      Reserved */
#define                         0x73    /*      Reserved */
#define                         0x74    /*      Reserved */
#define                         0x75    /*      Reserved */
#define                         0x76    /*      Reserved */
#define                         0x77    /*      Reserved */
#define                         0x78    /*      Reserved */
#define                         0x79    /*      Reserved */
#define                         0x7a    /*      Reserved */
#define                         0x7b    /*      Reserved */
#define                         0x7c    /*      Reserved */
#define                         0x7d    /*      Reserved */
#define                         0x7e    /*      Reserved */
#define                         0x7f    /*      Reserved */
#define                         0x80    /*      Reserved (Vendor) */
#define                         0x81    /*      Reserved (Vendor) */
#define                         0x82    /*      Reserved (Vendor) */
#define                         0x83    /*      Reserved (Vendor) */
#define                         0x84    /*      Reserved (Vendor) */
#define                         0x85    /*      Reserved (Vendor) */
#define                         0x86    /*      Reserved (Vendor) */
#endif
#define CFA_TRANSLATE_SECTOR    0x87    /* (O) ATA4 cfa translate sector */
#if 0
#define                         0x88    /*      Reserved (Vendor) */
#define                         0x89    /*      Reserved (Vendor) */
#define                         0x8a    /*      Reserved (Vendor) */
#define                         0x8b    /*      Reserved (Vendor) */
#define                         0x8c    /*      Reserved (Vendor) */
#define                         0x8d    /*      Reserved (Vendor) */
#define                         0x8e    /*      Reserved (Vendor) */
#define                         0x8f    /*      Reserved (Vendor) */
#endif
#define WIN_DIAGNOSE            0x90    /* (M) ATA1 execute drive diagnostic */
#define WIN_SPECIFY             0x91    /* (M) ATA1 initialize drive parameters */
#define WIN_DOWNLOAD_MICROCODE  0x92    /* (O) ATA2 download microcode */
#if 0
#define                         0x93    /*      Reserved */
#endif
#define WIN_STANDBYNOW2         0x94    /* (O) ATA1 standby immediate */
#define CFA_IDLEIMMEDIATE       0x95    /* (O) ATA1 idle immediate */
#define WIN_STANDBY2            0x96    /* (O) ATA1 standby */
#define WIN_SETIDLE2            0x97    /* (O) ATA1 idle */
#define WIN_CHECKPOWERMODE2     0x98    /* (O) ATA1 check power mode */
#define WIN_SLEEPNOW2           0x99    /* (O) ATA1 sleep */
#if 0
#define                         0x9a    /*      Reserved (Vendor) */
#define                         0x9b    /*      Reserved */
#define                         0x9c    /*      Reserved */
#define                         0x9d    /*      Reserved */
#define                         0x9e    /*      Reserved */
#define                         0x9f    /*      Reserved */
#endif
#define WIN_PACKETCMD           0xa0    /* (O) ATA4 packet command */
#define WIN_PIDENTIFY           0xa1    /* (O) ATA4 identify packet device */
#define WIN_QUEUED_SERVICE      0xa2    /* (O) ATA4 service */
#if 0
#define                         0xa3    /*      Reserved */
#define                         0xa4    /*      Reserved */
#define                         0xa5    /*      Reserved */
#define                         0xa6    /*      Reserved */
#define                         0xa7    /*      Reserved */
#define                         0xa8    /*      Reserved */
#define                         0xa9    /*      Reserved */
#define                         0xaa    /*      Reserved */
#define                         0xab    /*      Reserved */
#define                         0xac    /*      Reserved */
#define                         0xad    /*      Reserved */
#define                         0xae    /*      Reserved */
#define                         0xaf    /*      Reserved */
#endif
#define WIN_SMART               0xB0    /* (O) ATA3 smart */
#if 0
#define                         0xb1    /*      Reserved */
#define                         0xb2    /*      Reserved */
#define                         0xb3    /*      Reserved */
#define                         0xb4    /*      Reserved */
#define                         0xb5    /*      Reserved */
#define                         0xb6    /*      Reserved */
#define                         0xb7    /*      Reserved */
#endif
#define CFA_ACCESS_METADATA_STORAGE 0xb8
#if 0
#define                         0xb9    /*      Reserved (CFA) */
#define                         0xba    /*      Reserved (CFA) */
#define                         0xbb    /*      Reserved (CFA) */
#define                         0xbc    /*      Reserved (CFA) */
#define                         0xbd    /*      Reserved (CFA) */
#define                         0xbe    /*      Reserved (CFA) */
#define                         0xbf    /*      Reserved (CFA) */
#endif
#define CFA_ERASE_SECTORS       0xc0    /* (O) ATA4 erase sectors */
#if 0
#define                         0xc1    /*      Reserved (Vendor) */
#define                         0xc2    /*      Reserved (Vendor) */
#define                         0xc3    /*      Reserved (Vendor) */
#endif
#define WIN_MULTREAD            0xc4    /* (M) ATA3 (O) ATA1 read multiple */
#define WIN_MULTWRITE           0xc5    /* (M) ATA3 (O) ATA1 write multiple */
#define WIN_SETMULT             0xc6    /* (M) ATA3 (O) ATA1 set multiple mode */
#define WIN_READDMA_QUEUED      0xc7    /* (O) ATA4 read DMA queued */
#define WIN_READDMA             0xc8    /* (M) ATA3 (O) ATA1 read DMA w/ retry */
#define WIN_READDMA_ONCE        0xc9    /* (M) ATA3 (O) ATA1 read DMA w/o retry */
#define WIN_WRITEDMA            0xca    /* (M) ATA3 (O) ATA1 write DMA w/ retry */
#define WIN_WRITEDMA_ONCE       0xcb    /* (M) ATA3 (O) ATA1 write DMA w/o retry */
#define WIN_WRITEDMA_QUEUED     0xcc    /* (O) ATA4 write DMA queued */
#define CFA_WRITE_MULTI_WO_ERASE 0xcd   /* (O) ATA4 cfa write multiple w/o erase */
#if 0
#define                         0xce    /*      Reserved */
#define                         0xcf    /*      Reserved */
#define                         0xd0    /*      Reserved */
#define                         0xd1    /*      Reserved */
#define                         0xd2    /*      Reserved */
#define                         0xd3    /*      Reserved */
#define                         0xd4    /*      Reserved */
#define                         0xd5    /*      Reserved */
#define                         0xd6    /*      Reserved */
#define                         0xd7    /*      Reserved */
#define                         0xd8    /*      Reserved */
#define                         0xd9    /*      Reserved */
#endif
#define WIN_GETMEDIASTATUS      0xda    /* (O) ATA4 get media status */
#define WIN_ACKMEDIACHANGE      0xdb    /* (O) ATA1 ack media chg, removed ATA3 */
#define WIN_POSTBOOT            0xdc    /* (O) ATA1 post-boot, removed ATA3 */
#define WIN_PREBOOT             0xdd    /* (O) ATA1 pre-boot, removed ATA3 */
#define WIN_DOORLOCK            0xde    /* (O) ATA1 door lock */
#define WIN_DOORUNLOCK          0xdf    /* (O) ATA1 door unlock */
#define WIN_STANDBYNOW1         0xe0    /* (O) ATA1 standby immediate */
#define WIN_IDLEIMMEDIATE       0xe1    /* (O) ATA1 idle immediate */
#define WIN_STANDBY             0xe2    /* (O) ATA1 standby */
#define WIN_SETIDLE1            0xe3    /* (O) ATA1 idle */
#define WIN_READ_BUFFER         0xe4    /* (O) ATA1 read buffer */
#define WIN_CHECKPOWERMODE1     0xe5    /* (O) ATA1 check power mode */
#define WIN_SLEEPNOW1           0xe6    /* (O) ATA1 sleep */
#define WIN_FLUSH_CACHE         0xe7    /* (M) ATA5, (O) ATA4 flush cache */
#define WIN_WRITE_BUFFER        0xe8    /* (O) ATA1 write buffer */
#define WIN_WRITE_SAME          0xe9    /* (O) ATA1 write same, removed ATA3 */
#define WIN_FLUSH_CACHE_EXT     0xea    /* (O) ATA6, LBA48 */
#if 0
#define                         0xeb        /*      Reserved */
#endif
#define WIN_IDENTIFY            0xec    /* (M) ATA2 ((O) ATA1) identify drive */
#define WIN_MEDIAEJECT          0xed    /* (O) ATA2 media eject */
#define WIN_IDENTIFY_DMA        0xee    /* (O) ATA3 identify device DMA */
#define WIN_SETFEATURES         0xef    /* (M) ATA4, (O) ATA1 set features */
#define EXABYTE_ENABLE_NEST     0xf0    /* */
#define IBM_SENSE_CONDITION     0xf0    /*     measure disk temperature */
#define WIN_SECURITY_SET_PASS   0xf1    /* (O) ATA3 security set password */
#define WIN_SECURITY_UNLOCK     0xf2    /* (O) ATA3 security unlock */
#define WIN_SECURITY_ERASE_PREPARE 0xf3 /* (O) ATA3 security erase prepare */
#define WIN_SECURITY_ERASE_UNIT    0xf4 /* (O) ATA3 security erase unit */
#define WIN_SECURITY_FREEZE_LOCK   0xf5 /* (O) ATA3 security freeze lock */
#define CFA_WEAR_LEVEL          0xf5
#define WIN_SECURITY_DISABLE    0xf6    /* (O) ATA3 security disable password */
#if 0
#define                         0xf7    /*      Reserved (Vendor) */
#endif
#define WIN_READ_NATIVE_MAX     0xf8    /* (O) ATA4 read native max address */
#define WIN_SET_MAX             0xf9    /* (O) ATA4 set max address */
#if 0
#define                         0xfa    /*      Reserved (Vendor) */
#endif
#define DISABLE_SEAGATE         0xfb
#if 0
#define                         0xfc    /*      Reserved (Vendor) */
#define                         0xfd    /*      Reserved (Vendor) */
#define                         0xfe    /*      Reserved (Vendor) */
#define                         0xff    /*      Reserved (Vendor) */
#endif

#define MAX_MULT_SECTORS 16

#define ASC_ILLEGAL_OPCODE                   0x20
#define ASC_LOGICAL_BLOCK_OOR                0x21
#define ASC_INV_FIELD_IN_CMD_PACKET          0x24
#define ASC_MEDIUM_MAY_HAVE_CHANGED          0x28
#define ASC_INCOMPATIBLE_FORMAT              0x30
#define ASC_MEDIUM_NOT_PRESENT               0x3a
#define ASC_SAVING_PARAMETERS_NOT_SUPPORTED  0x39
#define ASC_MEDIA_REMOVAL_PREVENTED          0x53

#define CFA_NO_ERROR            0x00
#define CFA_MISC_ERROR          0x09
#define CFA_INVALID_COMMAND     0x20
#define CFA_INVALID_ADDRESS     0x21
#define CFA_ADDRESS_OVERFLOW    0x2f

#define SENSE_NONE            0
#define SENSE_NOT_READY       2
#define SENSE_ILLEGAL_REQUEST 5
#define SENSE_UNIT_ATTENTION  6

/* Self Monitoring And Reporting Technology */
#define SMART_READ_DATA       0xd0  /* ATA3 read attribute values */
#define SMART_READ_THRESH     0xd1  /* ATA3 read attr. threshold, ATA4 obsolete */
#define SMART_ATTR_AUTOSAVE   0xd2  /* ATA3 attribute autosave */
#define SMART_SAVE_ATTR       0xd3  /* ATA3 save attribute values */
#define SMART_EXECUTE_OFFLINE 0xd4  /* ATA4 execute offline immediate */
#define SMART_READ_LOG        0xd5  /* ATA5 */
#define SMART_WRITE_LOG       0xd6  /* ATA5 */
/*#define   unknown            0xd7  // ATA4 obsolete */
#define SMART_ENABLE          0xd8  /* ATA3 enable operations */
#define SMART_DISABLE         0xd9  /* ATA3 disable operations */
#define SMART_STATUS          0xda  /* ATA3 return status */
/*#define   unknown            0xdb  // ATA4 obsolete */

/* Execute Device Diagnostic    ATA1    (device 0) (device 1 OR 0x80) */
#define DDIAG_NO_ERROR          0x01
#define DDIAG_FORMAT_ERROR      0x02
#define DDIAG_BUFFER_ERROR      0x03
#define DDIAG_ECC_ERROR         0x04
#define DDIAG_MPU_ERROR         0x05
/* After ATA1, any value other than 0x01 or 0x81 should be considered Vendor */


/* Identify Drive Information   ATA1 */

/* General Configuration Bit-significant Information: */
#define GCBI_ATAPI_DEVICE                   0x8000  /* ATA3 */
#define GCBI_FORMAT_SPEED_TOLERANCE_GAP_REQ 0x4000  /* ATA1, obsolete ATA2 */
#define GCBI_TRACK_OFFSET_OPTION_AVAILABLE  0x2000  /* ATA1, obsolete ATA2 */
#define GCBI_DATA_STROBE_OFFSET_AVAILABLE   0x1000  /* ATA1, obsolete ATA2 */
#define GCBI_ROTATIONAL_SPEED_TOL_OVER_HALF 0x0800  /* ATA1, obsolete ATA2 */
#define GCBI_DISK_TRANSFER_RATE_OVER_10MBS  0x0400  /* ATA1, obsolete ATA2 */
#define GCBI_DISK_TRANSFER_RATE_OVER_5MBS   0x0200  /* ATA1, obsolete ATA2 */
#define GCBI_DISK_TRANSFER_RATE_UNDER_5MBS  0x0100  /* ATA1, obsolete ATA2 */
#define GCBI_REMOVABLE_CARTRIDGE_DRIVE      0x0080  /* ATA1 */
#define GCBI_FIXED_DRIVE                    0x0040  /* ATA1 */
#define GCBI_SPINDLE_MOTOR_CONTROL          0x0020  /* ATA1, obsolete ATA2 */
#define GCBI_HEAD_SWITCH_TIME_OVER_15USEC   0x0010  /* ATA1, obsolete ATA2 */
#define GCBI_NOT_MFM_ENCODED                0x0008  /* ATA1, obsolete ATA2 */
#define GCBI_SOFT_SECTORED                  0x0004  /* ATA1, obsolete ATA2 */
#define GCBI_HARD_SECTORED                  0x0002  /* ATA1, obsolete ATA2 */
/*#define                                   0x0001  //       reserved */

/* ATAPI General Configuration Bit-significant Information: */
#define GCBI_HAS_PACKET_FEAT_SET            0x8000  /* ATA4, ATAPI4 */
#define GCBI_NO_PACKET_FEAT_SET             0x4000  /* ATA4, ATAPI4 */

#define GCBI_DIRECT_ACCESS_DEVICE           0x0000  /* ATA4, ATAPI4 */
#define GCBI_SEQUENTIAL_ACCESS_DEVICE       0x0100  /* ATA4, ATAPI4 */
#define GCBI_PRINTER_DEVICE                 0x0200  /* ATA4, ATAPI4 */
#define GCBI_PROCESSOR_DEVICE               0x0300  /* ATA4, ATAPI4 */
#define GCBI_WRITE_ONCE_DEVICE              0x0400  /* ATA4, ATAPI4 */
#define GCBI_CDROM_DEVICE                   0x0500  /* ATA4, ATAPI4 */
#define GCBI_SCANNER_DEVICE                 0x0600  /* ATA4, ATAPI4 */
#define GCBI_OPTICAL_MEMORY_DEVICE          0x0700  /* ATA4, ATAPI4 */
#define GCBI_MEDIUM_CHANGER_DEVICE          0x0800  /* ATA4, ATAPI4 */
#define GCBI_COMMUNICATIONS_DEVICE          0x0900  /* ATA4, ATAPI4 */
#define GCBI_ARRAY_CONTROLLER_DEVICE        0x0c00  /* ATA4, ATAPI4 */
#define GCBI_ENCLOSURE_SERVICES_DEVICE      0x0d00  /* ATA5, ATAPI5 */
#define GCBI_REDUCED_BLOCK_COMMAND_DEVICE   0x0e00  /* ATA5, ATAPI5 */
#define GCBI_OPTICAL_CARD_RW_DEVICE         0x0f00  /* ATA5, ATAPI5 */
#define GCBI_UNKNOWN_DEVICE                 0x1f00  /* ATA4, ATAPI4 */

#define GCBI_HAS_REMOVABLE_MEDIA            0x0080  /* ATA4, ATAPI4 */

#define GCBI_50US_TILL_DRQ                  0x0400  /* ATA4, ATAPI4 */
#define GCBI_10MS_TILL_INTRQ_DRQ            0x0200  /* ATA4, ATAPI4 */
#define GCBI_3MS_TILL_DRQ                   0x0000  /* ATA4, ATAPI4 */

#define GCBI_16BYTE_PACKETS                 0x0001  /* ATA4, ATAPI4 */
#define GCBI_12BYTE_PACKETS                 0x0000  /* ATA4, ATAPI4 */

/* Capabilities */
#define CAP_INTERLEAVED_DMA_SUPPORTED       0x8000  /* ATA4, ATAPI4 */
#define CAP_COMMAND_QUEUING_SUPPORTED       0x4000  /* ATA4, ATAPI4 */
#define CAP_STDBY_TIMER_SUPPORTED           0x2000  /* ATA2 */
#define CAP_OVERLAP_OPERATION_SUPPORTED     0x2000  /* ATA4, ATAPI4 */
#define CAP_ATA_SOFT_RESET_REQUIRED         0x1000  /* ATA , obsolete ATAPI4 */
#define CAP_IORDY_SUPPORTED                 0x0800  /* ATA2 */
#define CAP_IORDY_CAN_BE_DISABLED           0x0400  /* ATA3 */
#define CAP_LBA_SUPPORTED                   0x0200  /* ATA1, obsolete ATA3 */
#define CAP_DMA_SUPPORTED                   0x0100  /* ATA1, obsolete ATA3 */
/* ATA4, LBA and DMA should be set */

/* Features */
#define FEAT_ENABLE_8BIT_DATA_TRANSFERS     0x01    /* ATA1, obsolete ATA3 */
                                                    /*       retired ATA4 */
                                                    /*       CFA ATA5 */
#define FEAT_ENABLE_WRITE_CACHE             0x02    /* ATA1 / Vendor */
#define FEAT_SET_TRANSFER_MODE              0x03    /* ATA1 */
#define FEAT_ENABLE_AUTO_DEFECT_REASSIGN    0x04    /* ATA3, obsolete ATA4 */
#define FEAT_ENABLE_ADVANCED_PM             0x05    /* ATA4 */
#define FEAT_ENABLE_POWER_UP_IN_STANDBY     0x06    /* ATA5 */
#define FEAT_POWER_UP_IN_STANDBY_SPIN_UP    0x07    /* ATA5 */
#define FEAT_ENABLE_CFA_POWER_MODE_1        0x0a    /* ATA5 */
#define FEAT_DISABLE_MEDIA_STATUS_NOTIFY    0x31    /* ATA4 */
#define FEAT_DISABLE_RETRIES                0x33    /* ATA1, obsolete ATA4 */
#define FEAT_ENABLE_AUTO_ACOUSTIC_MNGMNT    0x42    /* ATA6 */
#define FEAT_VENDOR_ECC_ON_RW_LONG          0x44    /* ATA1, obsolete ATA4 */
#define FEAT_SET_CACHE_SEGMENTS             0x54    /* ATA1, obsolete ATA4 */
#define FEAT_DISABLE_READ_LOOKAHEAD         0x55    /* ATA1 */
#define FEAT_ENABLE_RELEASE_INTERRUPT       0x5d    /* ATA4 */
#define FEAT_ENABLE_SERVICE_INTERRUPT       0x5e    /* ATA4 */
#define FEAT_DISABLE_REVERTING_TO_DEFAULTS  0x66    /* ATA1 */
#define FEAT_DISABLE_ECC                    0x77    /* ATA1, obsolete ATA4 */
#define FEAT_DISABLE_8BIT_DATA_TRANSFERS    0x81    /* ATA1, obsolete ATA3 */
                                                    /*       retired ATA4 */
                                                    /*       CFA ATA5 */
#define FEAT_DISABLE_WRITE_CACHE            0x82    /* ATA1 */
#define FEAT_DISABLE_AUTO_DEFECT_REASSIGN   0x84    /* ATA3, obsolete ATA4 */
#define FEAT_DISABLE_ADVANCED_PM            0x85    /* ATA4 */
#define FEAT_DISABLE_POWER_UP_IN_STANDBY    0x86    /* ATA5 */
#define FEAT_ENABLE_ECC                     0x88    /* ATA1, obsolete ATA4 */
#define FEAT_DISABLE_CFA_POWER_MODE_1       0x8a    /* ATA5 */
#define FEAT_ENABLE_MEDIA_STATUS_NOTIFY     0x95    /* ATA4 */
#define FEAT_ENABLE_RETRIES                 0x99    /* ATA1, obsolete ATA4 */
#define FEAT_SET_MAXIMUM_AVG_CURRENT        0x9a    /* ATA3, obsolete ATA4 */
#define FEAT_ENABLE_READ_LOOKAHEAD          0xaa    /* ATA1 */
#define FEAT_SET_MAXIMUM_PREFETCH           0xab    /* ATA1, obsolete ATA4 */
#define FEAT_4BYTE_ECC_ON_RW_LONG           0xbb    /* ATA1, obsolete ATA4 */
#define FEAT_DISABLE_AUTO_ACOUSTIC_MNGMNT   0xc2    /* ATA6 */
#define FEAT_ENABLE_REVERTING_TO_DEFAULTS   0xcc    /* ATA1 */
#define FEAT_DISABLE_RELEASE_INTERRUPT      0xdd    /* ATA4 */
#define FEAT_DISABLE_SERVICE_INTERRUPT      0xde    /* ATA4 */
/* 0xf0-0xff reserved for CFA */

/* Transfer Modes */
#define MODE_BLOCK_TRANSFER                 0x00    /* ATA1 */
#define MODE_PIO                            0x00    /* ATA2 */
#define MODE_PIO_NO_IORDY                   0x01    /* ATA2 */
#define MODE_PIO_FLOW_CONTROL               0x08    /* ATA2 (+0..7) */
#define MODE_SINGLE_WORD_DMA                0x10    /* ATA1 (+0..3) */
                                                    /* ATA2 (+0..7) */
                                                    /* ATA3 removed */
#define MODE_MULTIWORD_DMA                  0x20    /* ATA1 */
                                                    /* ATA2 (+0..7) */
#define MODE_ULTRA_DMA                      0x40    /* ATA4 (+0..7) */
                                                    /* 0x80 reserved */

/* Advanced Power Management Levels */
#define APML_MAXIMUM_PERFORMANC             0xfe    /* ATA4 */
#define APML_INTERMEDIATE_WITHOUT_STDBY     0x81    /* ATA4 0x81-0xfd */
#define APML_MINIMUM_POWER_WITHOUT_STDBY    0x80    /* ATA4 */
#define APML_INTERMEDIATE_WITH_STDBY        0x82    /* ATA4 0x81-0x7f */
#define APML_MINIMUM_POWER_WITH_STDBY       0x01    /* ATA4 */

/* Valid Fields */
#define FIELDS_VALID_54_58                  0x01    /* ATA1 */
#define FIELDS_VALID_64_70                  0x02    /* ATA2 */
#define FIELDS_VALID_88                     0x04    /* ATA4 */

/* Version Numbers (index is bit number! (i.e. 1<<index)) */
#define VERSION_MAJOR_ARRAY { \
    [0x00] = "ATA-1", \
    [0x01] = "ATA-2", \
    [0x02] = "ATA-3", \
    [0x03] = "ATA/ATAPI-4", \
    [0x04] = "ATA/ATAPI-5", \
    [0x05] = "ATA/ATAPI-6", \
}

#define VERSION_MINOR_ARRAY { \
    [0x01] = "ATA (ATA-1) X3T9.2 781D prior to revision 4", \
    [0x02] = "ATA-1 published, ANSI X3.221-1994", \
    [0x03] = "ATA (ATA-1) X3T9.2 781D revision 4", \
    [0x04] = "ATA-2 published, ANSI X3.279-1996", \
    [0x05] = "ATA-2 X3T10 948D prior to revision 2k", \
    [0x06] = "ATA-3 X3T10 2008D revision 1", \
    [0x07] = "ATA-2 X3T10 948D revision 2k", \
    [0x08] = "ATA-3 X3T10 2008D revision 0", \
    [0x09] = "ATA-2 X3T10 948D revision 3", \
    [0x0a] = "ATA-3 published, ANSI X3.298-1997", \
    [0x0b] = "ATA-3 X3T10 2008D revision 6", \
    [0x0c] = "ATA-3 X3T13 2008D revision 7 and 7a", \
    [0x0d] = "ATA/ATAPI-4 X3T13 1153D revision 6", \
    [0x0e] = "ATA/ATAPI-4 T13 1153D revision 13", \
    [0x0f] = "ATA/ATAPI-4 X3T13 1153D revision 7", \
    [0x10] = "ATA/ATAPI-4 T13 1153D revision 18", \
    [0x11] = "ATA/ATAPI-4 T13 1153D revision 15", \
    [0x12] = "ATA/ATAPI-4 published, ANSI NCITS 317-1998", \
    [0x13] = "ATA/ATAPI-5 T13 1321D revision 3", \
    [0x14] = "ATA/ATAPI-4 T13 1153D revision 14", \
    [0x15] = "ATA/ATAPI-5 T13 1321D revision 1", \
    [0x16] = "ATA/ATAPI-5 published, ANSI NCITS 340-2000", \
    [0x17] = "ATA/ATAPI-4 T13 1153D revision 17", \
    [0x18] = "ATA/ATAPI-6 T13 1410D revision 0", \
    [0x19] = "ATA/ATAPI-6 T13 1410D revision 3a", \
    [0x1a] = "Reserved", \
    [0x1b] = "ATA/ATAPI-6 T13 1410D revision 2", \
    [0x1c] = "ATA/ATAPI-6 T13 1410D revision 1", \
}

/* Feature Sets / Command Sets */
#define FEAT_SET_SMART                      0x0001  /* ATA3 */
#define FEAT_SET_SECURITY                   0x0002  /* ATA3 */
#define FEAT_SET_REMOVABLE_MEDIA            0x0004  /* ATA3 */
#define FEAT_SET_POWER_MANAGEMENT           0x0008  /* ATA3 */
#define FEAT_SET_PACKET                     0x0010  /* ATA4 */
#define FEAT_WRITE_CACHE                    0x0020  /* ATA4 */
#define FEAT_LOOK_AHEAD                     0x0040  /* ATA4 */
#define FEAT_RELEASE_INTERRUPT              0x0080  /* ATA4 */
#define FEAT_SERVICE_INTERRUPT              0x0100  /* ATA4 */
#define FEAT_DEVICE_RESET                   0x0200  /* ATA4 */
#define FEAT_SET_HOST_PROTECTED_AREA        0x0400  /* ATA4 */
/*#define                                   0x0800  // unknown, obsolete */
#define FEAT_WRITE_BUFFER                   0x1000  /* ATA4 */
#define FEAT_READ_BUFFER                    0x2000  /* ATA4 */
#define FEAT_NOP                            0x4000  /* ATA4 */
/*#define                                   0x8000  // unknown, obsolete */

#define FEAT_DOWNLOAD_MICROCODE                         0x0001  /* ATA4 */
#define FEAT_READ_WRITE_DMA_QUEUED                      0x0002  /* ATA4 */
#define FEAT_SET_CFA                                    0x0004  /* ATA4 */
#define FEAT_SET_ADVANCED_POWER_MANAGEMENT              0x0008  /* ATA4 */
#define FEAT_SET_REMOVABLE_MEDIA_STATUS_NOTIFICATION    0x0010  /* ATA4 */
#define FEAT_SET_POWER_UP_IN_STANDBY                    0x0020  /* ATA5 */
#define FEAT_POWERUP_BY_SET_FEATURES                    0x0040  /* ATA5 */
/*#define                                               0x0080  // reserved */
#define FEAT_SECURITY_EXTENSION_BY_SET_MAX_SET_PWD      0x0100  /* ATA5 */
#define FEAT_SET_AUTOMATIC_ACOUSTIC_MANAGEMENT          0x0200  /* ATA6 */
#define FEAT_SET_LBA48                                  0x0400  /* ATA6 */
#define FEAT_SET_DEV_CONF_OVERLAY                       0x0800  /* ATA6 */
#define FEAT_MANDATORY_FLSUH_CACHE                      0x1000  /* ATA6 */
#define FEAT_FLUSH_CACHE_EXT_SUPPORTED                  0x2000  /* ATA6 */
#define FEAT_14SHALL_BE_SET_15SHALL_BE_CLEAR            0x4000  /* ATA6 */

#define FEAT_SMART_ERROR_LOGGING                        0x0001  /* ATA6 */
#define FEAT_SMART_SELF_TEST                            0x0002  /* ATA6 */
#define FEAT_MEDIA_SERIAL_NUMBER                        0x0004  /* ATA6 */
#define FEAT_SET_MEDIA_CARD_PASS_THROUGH                0x0008  /* ATA6 */
/*#define                                               0x0010  // reserved */
#define FEAT_SET_GENERAL_PURPOSE_LOGGING                0x0020  /* ATA6 */

/* Security */
#define SECURITY_MAXIMUM                        0x0100  /* ATA3 */
#define SECURITY_COUNT_EXPIRED                  0x0010  /* ATA3 */
#define SECURITY_FROZEN                         0x0008  /* ATA3 */
#define SECURITY_LOCKED                         0x0004  /* ATA3 */
#define SECURITY_ENABLED                        0x0002  /* ATA3 */
#define SECURITY_SUPPORTED                      0x0001  /* ATA3 */

/* CFA Extended Error Codes */
#define CFA_EXTENDED_ERROR_CODES_ARRAY { \
    [0x00] = "No error detected / no additional information", \
    [0x01] = "Self test passed", \
    [0x03] = "Write / Erase failed", \
    [0x05] = "Self test or diagnostic failed", \
    [0x09] = "Miscellaneous error", \
    [0x0b] = "Vendor specific", \
    [0x0c] = "Corrupted media format", \
    [0x0d] = "Vendor specific", \
    [0x0e] = "Vendor specific", \
    [0x0f] = "Vendor specific", \
    [0x10] = "ID Not Found / ID Error", \
    [0x11] = "Uncorrectable ECC error", \
    [0x14] = "ID Not Found", \
    [0x18] = "Corrected ECC error", \
    [0x1d] = "Vendor specific", \
    [0x1e] = "Vendor specific", \
    [0x1f] = "Data transfer error / command aborted", \
    [0x20] = "Invalid command", \
    [0x21] = "Invalid address", \
    [0x22] = "Vendor specific", \
    [0x23] = "Vendor specific", \
    [0x27] = "Write protect violation", \
    [0x2f] = "Address overflow (address too large)", \
    [0x30] = "Self test or diagnostic failed", \
    [0x31] = "Self test or diagnostic failed", \
    [0x32] = "Self test or diagnostic failed", \
    [0x33] = "Self test or diagnostic failed", \
    [0x34] = "Self test or diagnostic failed", \
    [0x35] = "Supply or generated voltage out of tolerance", \
    [0x36] = "Supply or generated voltage out of tolerance", \
    [0x37] = "Self test or diagnostic failed", \
    [0x38] = "Corrupted media format", \
    [0x39] = "Vendor specific", \
    [0x3a] = "Spare sectors exhausted", \
    [0x3b] = "Corrupted media format", \
    [0x3c] = "Corrupted media format", \
    [0x3d] = "Vendor specific", \
    [0x3e] = "Self test or diagnostic failed", \
    [0x3f] = "Corrupted media format", \
}

/* Log Address Definition */
#define LOG_DIRECTORY                                   0x00    /* ATA6 */
#define LOG_COMPREHENSIVE_SMART_ERROR_LOG               0x02    /* ATA6 */
#define LOG_EXTENDED_COMPREHENSIVE_SMART_ERROR_LOG      0x03    /* ATA6 */
#define LOG_SMART_SELF_TEST_LOG                         0x06    /* ATA6 */
#define LOG_EXTENDED_SMART_SELF_TEST_LOG                0x07    /* ATA6 */

#endif
