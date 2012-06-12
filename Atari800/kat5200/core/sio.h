/******************************************************************************
*
* FILENAME: sio.h
*
* DESCRIPTION:  This contains function declarations for Serial I/O functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/26/07  bberlin      Creation
******************************************************************************/
#ifndef sio_h
#define sio_h

#include "boom6502.h"

#define MAX_SIO_DEVICES 8
#define MAX_CAS_BLOCKS  2048

/*
 * Device Commands
 */
#define SIO_CMD_RD_STATUS_BLK  0x4E
#define SIO_CMD_WT_STATUS_BLK  0x4F
#define SIO_CMD_READ           0x52
#define SIO_CMD_READ_HI        0xD2
#define SIO_CMD_WRITE          0x57
#define SIO_CMD_WRITE_HI       0xD7
#define SIO_CMD_STATUS         0x53
#define SIO_CMD_PUT            0x50
#define SIO_CMD_PUT_HI         0xD0
#define SIO_CMD_FORMAT         0x21
#define SIO_CMD_FORMAT_HI      0xA1
#define SIO_CMD_DUAL_FORMAT    0x22
#define SIO_CMD_DUAL_FORMAT_HI 0xA2
#define SIO_CMD_DOWNLOAD       0x20
#define SIO_CMD_READADDR       0x54
#define SIO_CMD_READ_SPIN      0x51
#define SIO_CMD_MOTOR_ON       0x55
#define SIO_CMD_VERIFY_SECTOR  0x56

/*
 * Device Responses
 */
#define SIO_RESP_ACK   0x41
#define SIO_RESP_NAK   0x4E
#define SIO_RESP_DONE  0x43
#define SIO_RESP_ERROR 0x45

/*
 * ATR File Indicators
 */
#define SIO_ATR_MAGIC1  0x96
#define SIO_ATR_MAGIC2  0x02

/*
 * Floppy File type
 */
#define SIO_DSK_128             0x01
#define SIO_DSK_256_BOOT_128    0x02
#define SIO_DSK_256_BOOT_256    0x03
#define SIO_DSK_256_BOOT_SIO2PC 0x04

/*
 * Floppy Response delay time 
 *    (I use 100 scanlines, not sure what it really should be)
 */
#define SIO_FLOPPY_GAP_DELAY 11400 

typedef enum {
	SIO_STATUS_NONE,
	SIO_STATUS_CMD,
	SIO_STATUS_READ,
	SIO_STATUS_WRITE,
	SIO_STATUS_GAP,
	SIO_STATUS_MAX
} e_sio_status;

typedef enum {
	SIO_TYPE_FLOPPY,
	SIO_TYPE_CASSETTE,
	SIO_TYPE_MAX
} e_sio_type;

typedef struct {
	BYTE *data_ptr;
	WORD size;
	WORD mark_tone_size;
} t_cas_block;

typedef struct {
	char filename[1024];
	BYTE *f_buffer;
	int buffer_size;
	int num_blocks;
	int current_block;
	int current_byte;
	int expected_bytes;
	BYTE header[16];
	BYTE data_frame[260];
	int header_size;
	int baud;
	BYTE *current_data;
	unsigned int action_time;
	e_sio_status status;
	e_sio_type type;
} t_sio_device;

typedef struct {
	t_sio_device dev;
	t_cas_block blk[MAX_CAS_BLOCKS];
	int record_pressed;
	unsigned int motor_gap_delay;
	char description[257];
} t_cas_device;

typedef struct {
	t_sio_device dev;
	int write_protect;
	int sector_size;
	int dsk_type;
} t_dsk_device;

typedef struct {
	t_dsk_device drives[MAX_SIO_DEVICES];
	t_cas_device cassette;
	t_sio_device *current_device;
	int motor_on;
	unsigned int motor_on_time;
	BYTE cmd_frame[6];
	int cmd_index;
} t_sio_interface;

void sio_init ( void );
void sio_reset ( void );
int sio_create_disk ( const char *filename, int num_sectors, int disk_type );
int sio_create_cassette ( const char *filename, const char *description );
void sio_press_record ( int pressed );
void sio_check ( void );
void sio_rewind_cassette ( t_cas_device *dev, int block );
int sio_mount_image ( int dev_num, e_sio_type dev_type, char *file, int write_protect, 
                      unsigned char *buffer, int buffer_size, int launch );
void sio_unmount_all ( void );
void sio_unmount_image ( e_sio_type type, int dev_num );
void sio_receive ( BYTE data );
void sio_set_motor ( BYTE value );
void sio_patch ( void );
BYTE sio_update_serial_in_bit ( void );

#endif
