/******************************************************************************
*
* FILENAME: sio.c
*
* DESCRIPTION:  This contains functions for emulating Serial I/O devices.  Some
*               comes straight from atari800
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/26/07  bberlin      Creation
******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sio.h"
#include "cart.h"
#include "console.h"
#include "boom6502.h"
#include "memory.h"
#include "pokey.h"

extern struct cpu cpu6502;
extern unsigned int pokey_cpu_clock;

t_sio_interface sio;

int sio_mount_atr ( t_dsk_device *dev, int write_protect,unsigned char *buffer, 
                                               int buffer_size );
int sio_mount_xfd ( t_dsk_device *dev, int write_protect,unsigned char *buffer, 
                                               int buffer_size );
int sio_mount_cas ( t_cas_device *cas, int write_protect,unsigned char *buffer, 
                                               int buffer_size );
int sio_mount_raw ( t_cas_device *cas, int write_protect,unsigned char *buffer, 
                                               int buffer_size );
int sio_allocate_disk_buffer ( t_dsk_device *drive, unsigned char *buffer, 
                                                            int buffer_size );
void sio_format_cmd_response ( void );
BYTE sio_read_sector ( t_dsk_device *dev, int sector, BYTE *frame );
BYTE sio_write_sector ( t_dsk_device *dev, int sector, BYTE *frame );
BYTE sio_read_status ( t_dsk_device *dev, BYTE *frame );
BYTE sio_read_status_block ( t_dsk_device *dev, BYTE *frame );
BYTE sio_format_disk ( t_dsk_device *dev, int sector_size, int num_sectors );
BYTE sio_checksum ( BYTE *buffer, int num_bytes );

/******************************************************************************
**  Function   :  sio_init
**                            
**  Objective  :  This function initializes Serial I/O
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_init ( void ) {

	int i;

	for ( i = 0; i < MAX_SIO_DEVICES; ++i ) {
		sio.drives[i].dev.f_buffer = 0;
		sio.drives[i].write_protect = 0;
		sio.drives[i].dev.baud = 8 * 114;
		sio.drives[i].dev.type = SIO_TYPE_FLOPPY;
		sio.drives[i].dev.status = SIO_STATUS_NONE;
		sio.drives[i].dev.action_time = 1;
	}
	sio.cassette.dev.f_buffer = 0;
	sio.cassette.record_pressed = 0;
	sio.cassette.dev.baud = 262 * 114;
	sio.cassette.dev.type = SIO_TYPE_CASSETTE;
	sio.cassette.dev.status = SIO_STATUS_NONE;
	sio.cassette.dev.action_time = 1;
	sio.current_device = &sio.drives[0].dev;
	sio.motor_on = 0;

} /* end sio_init */

/******************************************************************************
**  Function   :  sio_reset
**                            
**  Objective  :  This function is called on a console reset
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_reset ( void ) {

	int i;

	sio.motor_on = 0;
	sio.current_device->action_time = 1;
	sio.current_device->status = SIO_STATUS_NONE;

	for ( i = 0; i < MAX_SIO_DEVICES; ++i ) {
		sio.drives[i].dev.status = SIO_STATUS_NONE;
	}

	sio.cassette.dev.status = SIO_STATUS_NONE;

	sio_rewind_cassette ( &sio.cassette, 0 );

} /* end sio_reset */

/******************************************************************************
**  Function   :  sio_press_record
**                            
**  Objective  :  This function is when record is pressed or released
**
**  Parameters :  pressed - record button is pressed on the cassette
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_press_record ( int pressed ) {

	sio.cassette.record_pressed = pressed;

} /* end sio_press_record */

/******************************************************************************
**  Function   :  sio_mount_image
**                            
**  Objective  :  This function mounts an image to the device
**
**  Parameters :  dev_num       - if a floppy drive, which one (0-based)
**                dev_type      - either SIO_TYPE_FLOPPY or SIO_TYPE_CASSETTE
**                file          - the filename containing the image
**                write_protect - 1 if protected, 0 if not    
**                buffer        - buffer containing image
**                buffer_size   - size of buffer         
**                launch        - whether we are booting image or just 
**                                  attaching
**                                                
**  return     :  0 for success, otherwise failure
**      
******************************************************************************/ 
int sio_mount_image ( int dev_num, e_sio_type dev_type, char *file, int write_protect, 
                      unsigned char *buffer, int buffer_size, int launch ) {

	/*
	 * Check for an ATR File or just Raw (XFD) data for floppies
	 */
	if ( dev_type == SIO_TYPE_FLOPPY ) {

		strcpy ( sio.drives[dev_num].dev.filename, file );

		sio_unmount_image ( SIO_TYPE_FLOPPY, dev_num );

		sio.drives[dev_num].write_protect = write_protect;

		if ( launch )
			console_set_media_type ( CART_FLOPPY );

		if ( buffer[0] == SIO_ATR_MAGIC1 && buffer[1] == SIO_ATR_MAGIC2 )
			sio_mount_atr ( &sio.drives[dev_num], write_protect, buffer, buffer_size );
		else
			sio_mount_xfd ( &sio.drives[dev_num], write_protect, buffer, buffer_size );
	}

	/*
	 * Check for a CAS File or just Raw data for cassettes
	 */
	if ( dev_type == SIO_TYPE_CASSETTE ) {

		strcpy ( sio.cassette.dev.filename, file );

		sio_unmount_image ( SIO_TYPE_CASSETTE, 0 );

		if ( launch )
			console_set_media_type ( CART_CASSETTE );

		if ( !strncmp(buffer,"FUJI", 4) )
			sio_mount_cas ( &sio.cassette, write_protect, buffer, buffer_size );
		else
			sio_mount_raw ( &sio.cassette, write_protect, buffer, buffer_size );

		sio_rewind_cassette ( &sio.cassette, 0 );
	}

	return 0;

} /* end sio_mount_image */

/******************************************************************************
**  Function   :  sio_unmount_image
**                            
**  Objective  :  This function unmounts an image from the device
**
**  Parameters :  type    - wither SIO_TYPE_FLOPPY or SIO_TYPE_CASSETTE 
**                dev_num - if a floppy which drive (0-based)
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_unmount_image ( e_sio_type type, int dev_num ) {

	t_sio_device *dev;

	if ( type == SIO_TYPE_CASSETTE )
		dev = &sio.cassette.dev;
	else
		dev = &sio.drives[dev_num].dev;

	if ( dev->f_buffer )
		free ( dev->f_buffer );

	dev->f_buffer = 0;

} /* end sio_unmount_image */

/******************************************************************************
**  Function   :  sio_unmount_all
**                            
**  Objective  :  This function unmounts all images and frees allocated memory
**
**  Parameters :  NONE 
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_unmount_all ( void ) {

	int i;

	for ( i = 0; i < MAX_SIO_DEVICES; ++i ) {
		if ( sio.drives[i].dev.f_buffer ) {
			free ( sio.drives[i].dev.f_buffer );
		}
		sio.drives[i].dev.f_buffer = 0;
	}
	if ( sio.cassette.dev.f_buffer ) {
		free ( sio.cassette.dev.f_buffer );
	}
	sio.cassette.dev.f_buffer = 0;

} /* end sio_unmount_all */

/******************************************************************************
**  Function   :  sio_mount_atr
**                            
**  Objective  :  This function mounts an ATR image
**
**  Parameters :  drive         - pointer to disk drive struct 
**                write_protect - 1 if protected, 0 if not
**                buffer        - buffer containing image
**                buffer_size   - size of buffer         
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_mount_atr ( t_dsk_device *drive, int write_protect,unsigned char *buffer, 
                                               int buffer_size ) {

	t_sio_device *dev = &drive->dev;
	int i;

	dev->num_blocks = buffer[2] | (buffer[3]<<8) |
	                               (buffer[6]<<16) | (buffer[7]<<24);
	dev->num_blocks >>= 3;
	drive->sector_size = buffer[4] | (buffer[5]<<8);
	if ( !write_protect )
		drive->write_protect = buffer[15];

	/*
	 * Store away the header and get to the real data
	 */
	for ( i = 0; i < 16; ++i ) {
		dev->header[i] = buffer[i];
	}
	dev->header_size = 16;
	buffer += 16;
	buffer_size -=16;

	/* 
	 * Now let's determine what kind of boot sectors we have and adjust
	 *   if we have double density
	 */
	if ( drive->sector_size == 256 ) {

		/*
		 * Check for 128 bytes boot sectors
		 */
		if ( dev->num_blocks & 0x1 ) {
			dev->num_blocks +=3;
			drive->dsk_type = SIO_DSK_256_BOOT_128;
		}

		/*
		 *  Must be 256 bytes boot sectors
		 *   if data in second half it is a physical sector
		 */
		else {

			if ( buffer_size < (0x190 + 0x180) )
				return -1;

			drive->dsk_type = SIO_DSK_256_BOOT_SIO2PC;
			for ( i = 0x190; i < (0x190+0x180); ++i ) {
				if ( buffer[i] ) {
					drive->dsk_type = SIO_DSK_256_BOOT_256;
					break;
				}
			}
		}

		dev->num_blocks >>= 1;

	} /* end if 256 bytes sectors */

	/* 
	 * Else we have 128 byte sectors
	 */
	else {
		drive->dsk_type = SIO_DSK_128;
	}

	sio_allocate_disk_buffer ( drive, buffer, buffer_size );

	return 0;

} /* end sio_mount_atr */

/******************************************************************************
**  Function   :  sio_mount_xfd
**                            
**  Objective  :  This function mounts an XFD (raw floppy) image
**
**  Parameters :  drive         - pointer to disk drive struct 
**                write_protect - 1 if protected, 0 if not
**                buffer        - buffer containing image
**                buffer_size   - size of buffer         
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_mount_xfd ( t_dsk_device *drive, int write_protect,unsigned char *buffer, 
                                               int buffer_size ) {
	t_sio_device *dev = &drive->dev;

	dev->header_size = 0;

	/*
	 * If sectors are 128 bytes
	 */
	if ( buffer_size <= (1040 << 7) ) {
		drive->sector_size = 128;
		dev->num_blocks = buffer_size >> 7;
		drive->dsk_type = SIO_DSK_128;
	}

	/*
	 * Else sectors are 256 bytes
	 *   Figure out the size of the boot sectors (first 3)
	 */
	else {
		drive->sector_size = 256;
		if ( (buffer_size & 0xff) == 0 ) {
			dev->num_blocks = buffer_size >> 8;
			drive->dsk_type = SIO_DSK_256_BOOT_256;
		}
		else {
			drive->dsk_type = SIO_DSK_256_BOOT_128;
			dev->num_blocks = (buffer_size+384) >> 8;
		}
	}

	sio_allocate_disk_buffer ( drive, buffer, buffer_size );

	return 0;

} /* end sio_mount_xfd */

/******************************************************************************
**  Function   :  sio_mount_cas
**                            
**  Objective  :  This function mounts an CAS image
**
**  Parameters :  cas           - pointer to cassette drive struct 
**                write_protect - 1 if protected, 0 if not
**                buffer        - buffer containing image
**                buffer_size   - size of buffer         
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_mount_cas ( t_cas_device *cas, int write_protect,unsigned char *buffer, 
                                               int buffer_size ) {

	int i;
	int bytes,dev_data_bytes;
	int length,block;
	t_sio_device *dev = &cas->dev;

	/*
	 * Store away the header and get to the real data
	 */
	for ( i = 0; i < 8; ++i ) {
		dev->header[i] = buffer[i];
	}
	dev->header_size = 8;
	buffer += 8;
	buffer_size -=8;

	dev->f_buffer = calloc ( buffer_size, sizeof(BYTE) );
	dev->buffer_size = buffer_size * sizeof(BYTE);

	/*
	 * Store description 
	 */
	bytes = dev->header[4] + (dev->header[5] << 8);
	if ( bytes < 257 ) {
		strncpy ( cas->description, buffer, bytes );
		cas->description[bytes] = '\0';
	}
	else {
		strncpy ( cas->description, buffer, 256 );
		cas->description[256] = '\0';
	}

	/*
	 * Create pointers to each block of data
	 */
	dev_data_bytes = 0;
	block = 0;

	while ( bytes < buffer_size ) {
		
		if ( block >= MAX_CAS_BLOCKS )
			break;

		length = buffer[bytes+4] + (buffer[bytes+5] << 8);

		if ( !strncmp(&buffer[bytes],"data", 4) ) {
			if ( (length + bytes) > buffer_size )
				break;
			for ( i = 0; i < length; ++i ) {
				dev->f_buffer[dev_data_bytes + i] = buffer[bytes + 8 + i];
			}
			cas->blk[block].data_ptr = &dev->f_buffer[dev_data_bytes];
			cas->blk[block].size = length;
			cas->blk[block].mark_tone_size = (unsigned int)((15720 * (buffer[bytes+6] + (buffer[bytes+7] << 8))) / 1000) * 114;
			block++;
		}

		else if ( !strncmp(&buffer[bytes],"baud",4) ) {
			dev->baud = (157200 / (buffer[bytes+6] + (buffer[bytes+7] << 8))) * 114;
		}

		bytes += (length + 8);
		dev_data_bytes += length;
	}

	dev->num_blocks = block;

	return -1;

} /* end sio_mount_cas */

/******************************************************************************
**  Function   :  sio_mount_raw
**                            
**  Objective  :  This function mounts a raw cassette image
**
**  Parameters :  cas           - pointer to cassette drive struct 
**                write_protect - 1 if protected, 0 if not
**                buffer        - buffer containing image
**                buffer_size   - size of buffer         
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_mount_raw ( t_cas_device *cas, int write_protect,unsigned char *buffer, 
                                               int buffer_size ) {

	int i;
	int dev_bytes = 0;
	int bytes = 0;
	int length = 0;
	int block = 0;
	t_sio_device *dev = &cas->dev;

	strcpy ( cas->description, "" );

	dev->f_buffer = calloc ( (buffer_size/128 + 2) * 132, sizeof(BYTE) );
	dev->buffer_size = (buffer_size/128 + 2) * 132 * sizeof(BYTE);

	while ( bytes < buffer_size ) {

		if ( block >= MAX_CAS_BLOCKS )
			break;

		dev->f_buffer[dev_bytes] = dev->f_buffer[dev_bytes + 1] = 0x55;

		if ( (bytes + 128) <= buffer_size ) {
			length = 128;
			dev->f_buffer[dev_bytes + 2] = 0xfc;
		}
		else {
			length = buffer_size - bytes;
			dev->f_buffer[dev_bytes + 2] = 0xfa;
			dev->f_buffer[dev_bytes + 0x82] = length;
		}

		for ( i = 0; i < length; ++i ) {
			dev->f_buffer[dev_bytes + 3 + i] = buffer[bytes + i];
		}
		dev->f_buffer[dev_bytes + 0x83] = sio_checksum ( &dev->f_buffer[dev_bytes], 0x83 );

		cas->blk[block].data_ptr = &dev->f_buffer[dev_bytes];
		cas->blk[block].size = 0x84;
		dev->baud = 262 * 114;
		if ( block == 0 )
			cas->blk[block].mark_tone_size = 34407936;
		else
			cas->blk[block].mark_tone_size = (unsigned int)(15720 * 260 / 1000) * 114;

		block++;
		bytes += 128;
		dev_bytes += 132;
	}

	/*
	 * Write EOF record
	 */
	dev->f_buffer[dev_bytes] = dev->f_buffer[dev_bytes + 1] = 0x55;
	dev->f_buffer[dev_bytes + 2] = 0xfe;
	for ( i = 3; i < 0x83; ++i )
		dev->f_buffer[dev_bytes + i] = 0x00;
	dev->f_buffer[dev_bytes + 0x83] = sio_checksum ( &dev->f_buffer[dev_bytes], 0x83 );
	cas->blk[block].data_ptr = &dev->f_buffer[dev_bytes];
	cas->blk[block].size = 0x84;
	dev->baud = 262 * 114;
	cas->blk[block].mark_tone_size = (unsigned int)(15720 * 260 / 1000) * 114;

	dev->num_blocks = block + 1;

	return -1;

} /* end sio_mount_raw */

/******************************************************************************
**  Function   :  sio_allocate_disk_buffer
**                            
**  Objective  :  This function allocates memory for disk data and stores data
**
**  Parameters :  drive         - pointer to disk drive struct 
**                buffer        - buffer containing image
**                buffer_size   - size of buffer         
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_allocate_disk_buffer ( t_dsk_device *drive, unsigned char *buffer, 
                                                            int buffer_size ) {

	int real_data_start = 0;
	int i;

	/*
	 * For the 256 byte sectors with 128 boot, we make the boot 256 bytes
	 *   2nd half of data will be 0 so Atari "sees" it as 128
	 */
	switch ( drive->dsk_type ) {
		case SIO_DSK_256_BOOT_128:
			real_data_start = 384;
		case SIO_DSK_256_BOOT_SIO2PC:
			drive->dev.f_buffer = calloc ( buffer_size+384, sizeof(BYTE) );
			drive->dev.buffer_size = buffer_size+384 * sizeof(BYTE);
			for ( i = 0; i < 128; ++i ) {
				drive->dev.f_buffer[i] = buffer[i];
			}
			for ( i = 256; i < 384; ++i ) {
				if ( buffer_size < 129 )
					break;
				drive->dev.f_buffer[i] = buffer[i-128];
			}
			for ( i = 512; i < 640; ++i ) {
				if ( buffer_size < 257 )
					break;
				drive->dev.f_buffer[i] = buffer[i-256];
			}
			for ( i = 768; i < (buffer_size+384); ++i ) {
				drive->dev.f_buffer[i] = buffer[i-real_data_start];
			}
			break;
		case SIO_DSK_128:
		case SIO_DSK_256_BOOT_256:
			drive->dev.f_buffer = calloc ( buffer_size, sizeof(BYTE) );
			drive->dev.buffer_size = buffer_size * sizeof(BYTE);
			for ( i = 0; i < buffer_size; ++i ) {
				drive->dev.f_buffer[i] = buffer[i];
			}
			break;

	} /* end switch disk type */

	return 0;

} /* end sio_allocate_disk_buffer */

/******************************************************************************
**  Function   :  sio_write_disk_header
**                            
**  Objective  :  This function writes the disk header to file
**
**  Parameters :  fp          - file to write header to
**                num_sectors - number of sectors
**                disk_type   - type of disk (sector / boot sector size)
**                wp          - write protect
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_write_disk_header ( FILE *fp, int num_sectors, int disk_type, int wp ) {

	int i;

	rewind ( fp );

	fputc ( SIO_ATR_MAGIC1, fp );
	fputc ( SIO_ATR_MAGIC2, fp );

	switch ( disk_type ) {
		case SIO_DSK_128: break;
		case SIO_DSK_256_BOOT_128:
			num_sectors -= 3;
			num_sectors <<= 1;
			num_sectors += 3;
			num_sectors |= 1;
			break;
		case SIO_DSK_256_BOOT_256: 
		case SIO_DSK_256_BOOT_SIO2PC: 
			num_sectors <<= 1;
			break;
		default: break;
	}
	num_sectors <<= 3;

	fputc ( num_sectors & 0xff, fp );
	fputc ( (num_sectors >> 8) & 0xff, fp );

	if ( disk_type == SIO_DSK_128 ) {
		fputc ( 128, fp );
		fputc ( 0, fp );
	}
	else {
		fputc ( 0, fp );
		fputc ( 1, fp );
	}

	fputc ( (num_sectors >> 16) & 0xff, fp );
	fputc ( (num_sectors >> 24) & 0xff, fp );

	for ( i = 8; i < 15; ++i )
		fputc ( 0, fp );

	fputc ( wp, fp );

	return 0;

} /* end sio_write_disk_header */

/******************************************************************************
**  Function   :  sio_create_disk
**                            
**  Objective  :  This function creates an empty ATR file
**
**  Parameters :  filename    - file to create
**                num_sectors - number of sectors
**                disk_type   - type of disk (sector / boot sector size)
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_create_disk ( const char *filename, int num_sectors, int disk_type ) {

	FILE *fp;
	int size;
	int i;

	switch ( disk_type ) {
		case SIO_DSK_128: size = 128 * num_sectors; break;
		case SIO_DSK_256_BOOT_128: size = (256 * (num_sectors-3)) + (128 * 3); break;
		default: size = 256 * num_sectors; break;
	}

	fp = fopen ( filename, "wb" );

	if ( !fp )
		return -1;

	sio_write_disk_header ( fp, num_sectors, disk_type, 0 );

	for ( i = 0; i < size; ++i )
		fputc ( 0, fp );

	fclose ( fp );

	return 0;

} /* end sio_create_disk */

/******************************************************************************
**  Function   :  sio_write_disk
**                            
**  Objective  :  This function writes the disk to an ATR file
**
**  Parameters :  dsk - pointer to the disk
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_write_disk ( t_dsk_device *dsk ) {

	FILE *fp;
	int i;

	fp = fopen ( dsk->dev.filename, "wb" );

	if ( !fp )
		return -1;

	if ( dsk->write_protect )
		return 0;

	sio_write_disk_header ( fp, dsk->dev.num_blocks, dsk->dsk_type, 0 );

	/*
	 * Write the boot sectors based on the type
	 */
	switch ( dsk->dsk_type ) {
		case SIO_DSK_128: 
		case SIO_DSK_256_BOOT_256: 
			for ( i = 0; i < dsk->sector_size * 3; ++i ) {
				fputc ( dsk->dev.f_buffer[i], fp );
			}
			break;
		case SIO_DSK_256_BOOT_128: 
		case SIO_DSK_256_BOOT_SIO2PC: 
			for ( i = 0; i < 128; ++i )
				fputc ( dsk->dev.f_buffer[i], fp );
			for ( i = 256; i < 384; ++i )
				fputc ( dsk->dev.f_buffer[i], fp );
			for ( i = 512; i < 640; ++i )
				fputc ( dsk->dev.f_buffer[i], fp );
			if ( dsk->dsk_type == SIO_DSK_256_BOOT_SIO2PC ) {
				for ( i = 0; i < 384; ++i )
					fputc ( 0, fp );
			}
			break;
		default:
			for ( i = 0; i < dsk->sector_size * 3; ++i ) {
				fputc ( dsk->dev.f_buffer[i], fp );
			}
			break;

	} /* end switch disk type */

	/*
	 * Write the data
	 */
	for ( i = dsk->sector_size*3; i < dsk->sector_size * dsk->dev.num_blocks; ++i ) {
		fputc ( dsk->dev.f_buffer[i], fp );
	}

	fclose ( fp );

	return 0;

} /* end sio_write_disk */

/******************************************************************************
**  Function   :  sio_write_cas_header
**                            
**  Objective  :  This function writes the CAS header to file
**
**  Parameters :  filename    - file to create
**                description - title of cassette
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_write_cas_header ( FILE *fp, const char *description ) {

	if ( !fp ) 
		return -1;

	rewind ( fp );

	fputs ( "FUJI", fp );

	fputc ( strlen(description) & 0xff, fp );
	fputc ( (strlen(description)>>8) & 0xff, fp );
	fputc ( 0, fp );
	fputc ( 0, fp );
	fputs ( description, fp );

	fputs ( "baud", fp );
	fputc ( 0x00, fp );
	fputc ( 0x00, fp );
	fputc ( 0x58, fp );
	fputc ( 0x02, fp );

	return 0;

} /* end sio_write_cas_header */

/******************************************************************************
**  Function   :  sio_create_cassette
**                            
**  Objective  :  This function creates a new CAS file
**
**  Parameters :  filename    - file to create
**                description - title of the CAS
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_create_cassette ( const char *filename, const char *description ) {

	FILE *fp;

	fp = fopen ( filename, "wb" );

	if ( !fp )
		return -1;

	sio_write_cas_header ( fp, description );

	fclose ( fp );

	return 0;

} /* end sio_create_cassette */

/******************************************************************************
**  Function   :  sio_write_cassette
**                            
**  Objective  :  This function writes the CAS file
**
**  Parameters :  cas - pointer to the cassette
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
int sio_write_cassette ( t_cas_device *cas ) {

	FILE *fp;
	int i;
	unsigned int j;

	fp = fopen ( cas->dev.filename, "wb" );

	if ( !fp )
		return -1;

	sio_write_cas_header ( fp, cas->description );

	for ( i = 0; i < cas->dev.num_blocks; ++i ) {

		fputs ( "data", fp );

		/*
		 * Length
		 */
		fputc ( cas->blk[i].size & 0xff, fp );
		fputc ( (cas->blk[i].size >> 8) & 0xff, fp );

		/*
		 * Mark Tone Size - convert to ms
		 */
		fputc ( (((cas->blk[i].mark_tone_size / 114) * 1000) / 15720) & 0xff, fp );
		fputc ( ((((cas->blk[i].mark_tone_size / 114) * 1000) / 15720) >> 8) & 0xff, fp );

		/*
		 * Data with sync bytes and checksum 
		 */
		for ( j = 0; j < cas->blk[i].size; ++j )
			fputc ( cas->blk[i].data_ptr[j], fp );

	} /* end for number of blocks */

	fclose ( fp );

	return 0;

} /* end sio_write_cassette */

/******************************************************************************
**  Function   :  sio_rewind_cassette
**                            
**  Objective  :  This function rewinds the cassette to the specified block
**
**  Parameters :  dev   - pointer to cassette device struct 
**                block - cassette block to change to
**                                                
**  return     :  0 on success, otherwise failure
**      
******************************************************************************/ 
void sio_rewind_cassette ( t_cas_device *dev, int block ) {

	int gap_delay;
	int i;
	int offset = 0;

	sio.cassette.dev.current_block = block;
	sio.cassette.dev.current_byte = 0;
	sio.cassette.dev.status = SIO_STATUS_GAP;
	sio.cassette.dev.expected_bytes = sio.cassette.blk[sio.cassette.dev.current_block].size;
	gap_delay = sio.cassette.blk[sio.cassette.dev.current_block].mark_tone_size;
	sio.cassette.dev.action_time = (pokey_cpu_clock + gap_delay);

	for ( i = 0; i < block; ++i )
		offset += sio.cassette.blk[i].size;

	sio.cassette.dev.current_data = &sio.cassette.dev.f_buffer[offset];
}

/******************************************************************************
**  Function   :  sio_update_serial_in_bit
**                            
**  Objective  :  This function returns the bit in SKSTAT indicating the 
**                current state of the serial in to Pokey.
**
**  Parameters :  NONE
**                                                
**  return     :  SKSTAT bit
**      
******************************************************************************/ 
BYTE sio_update_serial_in_bit ( void ) {

	int bit;

	/*
	 * If Pokey not reading us, line is set to 1
	 */
	if ( sio.current_device->status != SIO_STATUS_READ )
		return SK_DIRECT_SERIN;

	/*
	 * Determine bit and return
	 *   The baud is the cpu cycles for 10 bits
	 */
	bit = ((sio.current_device->action_time - pokey_cpu_clock - 1) * 10) / sio.current_device->baud;

	if ( bit == 9 ) return 0;
	else if ( bit <= 0 || bit > 9 ) return SK_DIRECT_SERIN;
	else {
		return (*(sio.current_device->current_data) & (0x80 >> (bit-1))) ? SK_DIRECT_SERIN : 0;
	}

} /* end sio_update_serial_in_bit */

/******************************************************************************
**  Function   :  sio_check
**                            
**  Objective  :  This function checks the sio interface for any actions to
**                take.
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_check ( void ) {

	/*
	 * Go through each device that currently is used and check for 
	 *   action
	 */
	if ( sio.current_device->action_time == pokey_cpu_clock ) {
		switch ( sio.current_device->status ) {
			case SIO_STATUS_NONE:
				break;
			case SIO_STATUS_GAP: /* gap done, start data */
				sio.current_device->current_byte = 1;
				sio.current_device->status = SIO_STATUS_READ;
				sio.current_device->action_time = pokey_cpu_clock + sio.current_device->baud;
				break;
			case SIO_STATUS_READ:
				
				pokey_receive_SIO ( *sio.current_device->current_data );

				/*
				 * If done with current record update stuff
				 */
				if ( sio.current_device->current_byte >= 
				       sio.current_device->expected_bytes ) {

					if ( sio.current_device->type == SIO_TYPE_CASSETTE ) {
						if ( (++sio.current_device->current_block) >= sio.current_device->num_blocks ) {
							sio.current_device->action_time = 1;
							sio.current_device->status = SIO_STATUS_NONE;
							sio.current_device->current_block--;
						}
						else {
							t_cas_block *blk = &sio.cassette.blk[sio.current_device->current_block];
							sio.current_device->current_data = blk->data_ptr;
							sio.current_device->expected_bytes = blk->size;
							sio.current_device->current_byte = 1;
							sio.motor_on_time = pokey_cpu_clock;
							if ( blk->mark_tone_size ) {
								sio.current_device->status = SIO_STATUS_GAP;
								sio.current_device->action_time = pokey_cpu_clock + blk->mark_tone_size;
							}
							else {
								sio.current_device->status = SIO_STATUS_READ;
								sio.current_device->action_time = pokey_cpu_clock + sio.current_device->baud;
							}
						}
					}
					else {
						sio.current_device->action_time = 1;
						sio.current_device->status = SIO_STATUS_NONE;
					}

				} /* end if done with current record */

				/*
				 * Else just update the byte going out
				 */
				else {
					++sio.current_device->current_data;
					++sio.current_device->current_byte;
					sio.current_device->action_time = pokey_cpu_clock + sio.current_device->baud;
				}

				break;

			default:
				break;

		} /* end switch status */
	}

} /* end sio_check */

/******************************************************************************
**  Function   :  sio_set_motor
**                            
**  Objective  :  This function sets the state of the motor, if just turning
**                on, set up a data frame to transmit.
**
**  Parameters :  value - from PIA, 0x08 for motor off, 0 for motor on
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_set_motor ( BYTE value ) {

	/*
	 * Motor off - stop cassette
	 */
	if ( value ) {
		sio.cassette.motor_gap_delay = 0;
		if ( sio.motor_on ) {
			if ( sio.cassette.dev.status == SIO_STATUS_GAP ) {
				sio.cassette.motor_gap_delay = (pokey_cpu_clock - sio.motor_on_time);
			}
		}
		sio.motor_on = 0;
		sio.cassette.dev.status = SIO_STATUS_NONE;
		sio.cassette.dev.action_time = 1;
	}

	/*
	 * Motor on - start cassette
	 */
	else {
		if ( !sio.cassette.dev.f_buffer  || sio.motor_on )
			return;
		sio.motor_on = 1;
		sio.motor_on_time = pokey_cpu_clock;
		sio.current_device = &sio.cassette.dev;
		if ( sio.cassette.record_pressed ) {
			sio.cassette.dev.status = SIO_STATUS_WRITE;
		}
		else {
			t_cas_block *blk = &sio.cassette.blk[sio.cassette.dev.current_block];
			if ( blk->mark_tone_size ) {
				sio.cassette.dev.status = SIO_STATUS_GAP;
				sio.cassette.dev.action_time = pokey_cpu_clock + blk->mark_tone_size - sio.cassette.motor_gap_delay;
			}
			else {
				sio.cassette.dev.status = SIO_STATUS_READ;
				sio.cassette.dev.action_time = pokey_cpu_clock + sio.cassette.dev.baud;
			}
		}
	}

} /* end sio_set_motor */

/******************************************************************************
**  Function   :  sio_receive
**                            
**  Objective  :  This function receives a byte from Pokey
**
**  Parameters :  data - byte of data received 
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_receive ( BYTE data ) {

	/*
	 * What we do with the data depends on what we are currently doing
	 */
	switch ( sio.current_device->status ) {

		case SIO_STATUS_NONE:
		case SIO_STATUS_READ:
			if ( data < 0x31 || (data > 0x39) )
				break;
			sio.current_device = &sio.drives[data-0x31].dev;
			sio.current_device->status = SIO_STATUS_CMD;
			sio.cmd_frame[sio.cmd_index=0] = data;
			break;
		case SIO_STATUS_CMD:
			sio.cmd_frame[++sio.cmd_index] = data;
			if ( sio.cmd_index == 4 ) {
				sio.current_device->status = SIO_STATUS_GAP;
				sio.current_device->action_time = pokey_cpu_clock + SIO_FLOPPY_GAP_DELAY;
				sio_format_cmd_response();
			}
			break;
		case SIO_STATUS_WRITE:

			/*
			 * For Cassette we need to check for a gap
			 */
			if ( sio.current_device->type == SIO_TYPE_CASSETTE ) {
				t_cas_block *blk = &sio.cassette.blk[sio.cassette.dev.current_block];
				if ( (pokey_cpu_clock - sio.current_device->baud) > 
				        sio.motor_on_time ) {

					blk->size = sio.current_device->current_byte; 

					blk = &sio.cassette.blk[++sio.cassette.dev.current_block];
					blk->mark_tone_size = pokey_cpu_clock - sio.current_device->baud - sio.motor_on_time;
					blk->data_ptr = sio.current_device->current_data;
					sio.current_device->current_byte = 0; 
				}
				*sio.current_device->current_data = data;
				++sio.current_device->current_data;
				++sio.current_device->current_byte;
				sio_write_cassette ( &sio.cassette );
			}

			/*
			 * Else this is a floppy
			 */
			else {
				sio.current_device->data_frame[sio.current_device->current_byte++] = data;
				if ( sio.current_device->current_byte == sio.current_device->expected_bytes ) {

					int i;
					t_dsk_device *dsk = &sio.drives[0];
					for ( i = 0; i < MAX_SIO_DEVICES; ++i ) {
						if ( sio.current_device == &sio.drives[i].dev ) {
							dsk = &sio.drives[i];
						}
					}

					sio.current_device->data_frame[0] = sio_write_sector ( dsk, 
				                                 sio.current_device->current_block, 
				                                 sio.current_device->data_frame );

					switch ( sio.current_device->data_frame[0] ) {
						case SIO_RESP_ACK:
							sio.current_device->data_frame[1] = SIO_RESP_DONE;
							sio.current_device->expected_bytes = 2;
							break;
						case SIO_RESP_ERROR:
							sio.current_device->data_frame[1] = SIO_RESP_ERROR;
							sio.current_device->expected_bytes = 2;
							break;
						default:
							sio.current_device->expected_bytes = 1;
							break;
					}

					sio.current_device->action_time = pokey_cpu_clock + 114;
					sio.current_device->status = SIO_STATUS_GAP;
					sio.current_device->current_data = sio.current_device->data_frame;
					sio.current_device->current_byte = 0;
				}

			} /* end if floppy */
			break;

		default:
			break;

	} /* end switch status */

} /* end sio_receive */

/******************************************************************************
**  Function   :  sio_done_interrupt
**                            
**  Objective  :  This function is called when a tranmit done interrupt is
**                performed by pokey.  It checks that we are not in the 
**                middle of a command frame.
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_done_interrupt ( void ) {

	if ( sio.current_device->status == SIO_STATUS_CMD ) {
		sio.current_device->status = SIO_STATUS_NONE;
	}

} /* end sio_done_interrupt */

/******************************************************************************
**  Function   :  sio_format_cmd_response
**                            
**  Objective  :  This function checks the command and puts together the resp
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_format_cmd_response ( void ) {

	t_sio_device *dev = sio.current_device;
	t_dsk_device *dsk;
	int sector = ((sio.cmd_frame[3] << 8) | sio.cmd_frame[2]) - 1;

	/*
	 * This must be a floppy drive or don't respond
	 *   FIXME: Printer?
	 */
	if ( dev->type != SIO_TYPE_FLOPPY ) {
		dev->status = SIO_STATUS_NONE;
		return;
	}

	dev->current_byte = 0;
	dev->current_data = &dev->data_frame[0];

	dsk = &sio.drives[sio.cmd_frame[0]-0x31];

	/*
	 * Check the second byte of command from PC for type
	 */
	switch ( sio.cmd_frame[1] ) {
		case SIO_CMD_RD_STATUS_BLK:
			dev->data_frame[0] = sio_read_status_block ( dsk, dev->data_frame+2 );
			if ( dev->data_frame[0] == SIO_RESP_ACK ) {
				dev->data_frame[1] = SIO_RESP_DONE;
				dev->expected_bytes = 4;
				dev->data_frame[dev->expected_bytes+2] = sio_checksum ( dev->data_frame+2, dev->expected_bytes );
				dev->expected_bytes+=3;
			}
			break;
		case SIO_CMD_WT_STATUS_BLK:
			break;
		case SIO_CMD_READ:
		case SIO_CMD_READ_HI:
			dev->data_frame[0] = sio_read_sector ( dsk, sector, dev->data_frame+2 );
			if ( dev->data_frame[0] == SIO_RESP_ACK ) {
				dev->data_frame[1] = SIO_RESP_DONE;
				dev->expected_bytes = dsk->sector_size;
				dev->data_frame[dev->expected_bytes+2] = sio_checksum ( dev->data_frame+2, dev->expected_bytes );
				dev->expected_bytes+=3;
			}
			break;
		case SIO_CMD_PUT:
		case SIO_CMD_PUT_HI:
		case SIO_CMD_WRITE:
		case SIO_CMD_WRITE_HI:
			dev->status = SIO_STATUS_WRITE;
			dev->data_frame[0] = SIO_RESP_ACK;
			dev->expected_bytes = 1;
			dev->current_block = sector;
			break;
		case SIO_CMD_STATUS:
			dev->data_frame[0] = sio_read_status ( dsk, dev->data_frame+2 );
			if ( dev->data_frame[0] == SIO_RESP_ACK ) {
				if ( dev->f_buffer )
					dev->data_frame[1] = SIO_RESP_DONE;
				else
					dev->data_frame[1] = 0;
				dev->expected_bytes = 4;
				dev->data_frame[dev->expected_bytes+2] = sio_checksum ( dev->data_frame+2, dev->expected_bytes );
				dev->expected_bytes+=3;
			}
			break;
		case SIO_CMD_FORMAT:
		case SIO_CMD_FORMAT_HI:
			dev->data_frame[0] = sio_format_disk ( dsk, dsk->sector_size, dev->num_blocks );
			dev->data_frame[1] = SIO_RESP_DONE;
			if ( dev->data_frame[0] == SIO_RESP_ACK )
				dev->expected_bytes = 2;
			break;
		case SIO_CMD_DUAL_FORMAT:
		case SIO_CMD_DUAL_FORMAT_HI:
			dev->data_frame[0] = sio_format_disk ( dsk, 128, 1040 );
			dev->data_frame[1] = SIO_RESP_DONE;
			if ( dev->data_frame[0] == SIO_RESP_ACK )
				dev->expected_bytes = 2;
			break;
		case SIO_CMD_DOWNLOAD:
			break;
		case SIO_CMD_READADDR:
			break;
		case SIO_CMD_READ_SPIN:
			break;
		case SIO_CMD_MOTOR_ON:
			break;
		case SIO_CMD_VERIFY_SECTOR:
			break;
		default:
			dev->status = SIO_STATUS_NONE;
			break;
	}

	/*
	 * Check for a non ack and setup accordingly
	 */
	if ( dev->data_frame[0] != SIO_RESP_ACK ) {
		if ( dev->data_frame[0] == SIO_RESP_ERROR ) {
			dev->data_frame[0] = SIO_RESP_ACK;
			dev->data_frame[1] = SIO_RESP_ERROR;
			dev->expected_bytes = 2;
		}
		else
			dev->expected_bytes = 1;
	}

} /* end sio_format_cmd_response */

/******************************************************************************
**  Function   :  sio_read_sector
**                            
**  Objective  :  This function reads from our device to the frame buffer
**
**  Parameters :  dev    - which device to get data from
**                sector - which sector on the disk to read
**                frame  - buffer to store the data
**                                                
**  return     :  ACK or NAK to the command telling us to do this
**      
******************************************************************************/ 
BYTE sio_read_sector ( t_dsk_device *dsk, int sector, BYTE *frame ) {

	if ( !dsk->dev.f_buffer )
		return SIO_RESP_ERROR;
	if ( sector < 0 || sector >= dsk->dev.num_blocks )
		return SIO_RESP_NAK;

	memcpy ( frame, dsk->dev.f_buffer + (dsk->sector_size * sector), 
	         sizeof(BYTE) * dsk->sector_size );

	return SIO_RESP_ACK;

} /* end sio_read_sector */

/******************************************************************************
**  Function   :  sio_write_sector
**                            
**  Objective  :  This function writes from the frame buffer to our device
**
**  Parameters :  dev    - which device to get data from
**                sector - which sector on the disk to write
**                frame  - buffer to store the data
**                                                
**  return     :  ACK or NAK to the command telling us to do this
**      
******************************************************************************/ 
BYTE sio_write_sector ( t_dsk_device *dsk, int sector, BYTE *frame ) {

	if ( !dsk->dev.f_buffer )
		return SIO_RESP_ERROR;
	if ( sector < 0 || sector >= dsk->dev.num_blocks )
		return SIO_RESP_NAK;

	memcpy ( dsk->dev.f_buffer + (dsk->sector_size * sector), frame,
	         sizeof(BYTE) * dsk->sector_size );

	sio_write_disk ( dsk );

	return SIO_RESP_ACK;

} /* end sio_write_sector */

/******************************************************************************
**  Function   :  sio_read_status
**                            
**  Objective  :  This function gets the status message from the drive
**
**  Parameters :  dev    - which device to get status from
**                frame  - buffer to store the data
**                                                
**  return     :  ACK or NAK to the command telling us to do this
**      
******************************************************************************/ 
BYTE sio_read_status ( t_dsk_device *dsk, BYTE *frame ) {

	/*
	 * Status
	 */
	frame[0] = 0x10;
	/*frame[0] |= (dev->error) 0x4 : 0x0; */
	frame[0] |= 0x0;
	frame[0] |= (dsk->write_protect) ? 0x8 : 0x0;
	frame[0] |= (dsk->sector_size==256) ? 0x20 : 0x0;
	frame[0] |= (dsk->dev.num_blocks==1040) ? 0x80 : 0x0;

	/*
	 * Media Present?
	 */
	frame[1] = dsk->dev.f_buffer ? 0xff:0x7f;

	/*
	 * Constants
	 */
	frame[2] = 1;
	frame[3] = 0;

	return SIO_RESP_ACK;

} /* end sio_read_status */

/******************************************************************************
**  Function   :  sio_read_status_block
**                            
**  Objective  :  This function gets the status block from the drive
**
**  Parameters :  dsk    - which device to get status from
**                frame  - buffer to store the data
**                                                
**  return     :  ACK or NAK to the command telling us to do this
**      
******************************************************************************/ 
BYTE sio_read_status_block ( t_dsk_device *dsk, BYTE *frame ) {

	int tracks, heads, spt;

	/*
	 * Default
	 */
	tracks = 1;
	heads = 1;
	spt = dsk->dev.num_blocks;

	/*
	 * Now adjust if divisible by 40
	 */
	if (spt % 40 == 0) {
		/* standard disk */
		tracks = 40;
		spt /= 40;
		if (spt > 26 && spt % 2 == 0) {
			/* double-sided */
			heads = 2;
			spt >>= 1;
			if (spt > 26 && spt % 2 == 0) {
				/* double-sided, 80 tracks */
				tracks = 80;
				spt >>= 1;
			}
		}
	}

	/*
	 * Status
	 */
	frame[0] = tracks;              /* # of tracks */
	frame[1] = 1;                   /* step rate */
	frame[2] = (spt >> 8);  /* sectors per track. HI byte */
	frame[3] = spt;         /* sectors per track. LO byte */
	frame[4] = (heads - 1); /* # of heads minus 1 */
	frame[5] = (dsk->sector_size == 128 && dsk->dev.num_blocks <= 720) ? 0 : 4;
	frame[6] = (dsk->sector_size >> 8) & 0xff; /* bytes per sector. HI byte */
	frame[7] = dsk->sector_size & 0xff;        /* bytes per sector. LO byte */
	frame[8] = 1;                   /* drive is online */
	frame[9] = 192;                 /* transfer speed, whatever this means */
	frame[10] = 0;
	frame[11] = 0;

	return SIO_RESP_ACK;

} /* end sio_read_status_block */

/******************************************************************************
**  Function   :  sio_format_disk
**                            
**  Objective  :  This function formats the disk
**
**  Parameters :  dsk         - which device to get status from
**                sector_size - how big the sectors are
**                num_sectors - number of sectors
**                                                
**  return     :  ACK or NAK to the command telling us to do this
**      
******************************************************************************/ 
BYTE sio_format_disk ( t_dsk_device *dsk, int sector_size, int num_sectors ) {

	/* 
	 * If we get here, We must know the disk parameters
	 *   Just set the buffer to 0
	 */
	if ( !dsk->dev.f_buffer )
		return SIO_RESP_ERROR;
	if ( dsk->write_protect )
		return SIO_RESP_ERROR;
	
	if ( (dsk->dev.num_blocks != num_sectors) || (dsk->sector_size != sector_size) ) {
		free ( dsk->dev.f_buffer );
		dsk->dev.f_buffer = calloc ( sector_size*num_sectors, sizeof(BYTE) );
		dsk->sector_size = sector_size;
		dsk->dev.num_blocks = num_sectors;
		if ( sector_size == 128 )
			dsk->dsk_type = SIO_DSK_128;
		else
			dsk->dsk_type = SIO_DSK_256_BOOT_128;
	}
	memset ( dsk->dev.f_buffer, 0, sizeof(BYTE)*dsk->sector_size*dsk->dev.num_blocks );

	sio_write_disk ( dsk );

	return SIO_RESP_ACK;

} /* end sio_format_disk */

/******************************************************************************
**  Function   :  sio_checksum
**                            
**  Objective  :  This function figures out the checksum from input buffer
**
**  Parameters :  buffer - contains bytes to calculate checksum
**                num_bytes - number of bytes to use in calculation
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
BYTE sio_checksum ( BYTE *buffer, int num_bytes ) {

	int checksum = 0;

	while (--num_bytes >= 0)
		checksum += *buffer++;
	do
		checksum = (checksum & 0xff) + (checksum >> 8);
	while (checksum > 255);

	return checksum;

} /* end sio_checksum */

/******************************************************************************
**  Function   :  sio_patch
**                            
**  Objective  :  This function runs the SIO patch when 0xe459 is hit.  We'll
**                basically do what Atari800 does.
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void sio_patch ( void ) {

	unsigned int i;
	t_sio_device *dev = sio.current_device;
	t_dsk_device *dsk;
	t_cas_block *block;
	struct cpu *cpu = console_get_ptr()->cpu;
	BYTE gap = rd_6502(0x30b);
	unsigned int gap_delay = rd_6502(0x30b);
	unsigned int cas_delay = 0;
	//BYTE cmd = rd_6502(0x302);
	BYTE device = rd_6502(0x301) - 1;
	WORD sector = (rd_6502(0x30b) << 8) | (rd_6502(0x30a));
	//WORD size = (rd_6502(0x309) << 8) | (rd_6502(0x308));
	WORD addr = (rd_6502(0x305) << 8) | (rd_6502(0x304));

	sio.cmd_frame[0] = rd_6502(0x301) + rd_6502(0x300) - 1; /* device */
	sio.cmd_frame[1] = rd_6502(0x302);                      /* cmd    */
	sio.cmd_frame[2] = rd_6502(0x30a);                      /* sector */
	sio.cmd_frame[3] = rd_6502(0x30b);                      /* sector */

	/*
	 * This is a floppy drive
	 */
	if ( (sio.cmd_frame[0] > 0x30 && sio.cmd_frame[0] < 0x39) ) {

		dev = sio.current_device = &sio.drives[device].dev;
		dsk = &sio.drives[device];

		sio_format_cmd_response ( );

		switch ( sio.cmd_frame[1] ) {

			case SIO_CMD_RD_STATUS_BLK:
				if ( dev->data_frame[1] == SIO_RESP_DONE ) {
					for ( i = 0; i < 12; ++i ) {
						wr_6502((addr+i), dev->data_frame[2+i]);
					}
				}
				break;
			case SIO_CMD_WT_STATUS_BLK:
				break;
			case SIO_CMD_READ:
			case SIO_CMD_READ_HI:
				if ( dev->data_frame[1] == SIO_RESP_DONE ) {
					for ( i = 0; i < (unsigned int)dsk->sector_size; ++i ) {
						wr_6502((addr+i), dev->data_frame[2+i]);
					}
				}
				break;
			case SIO_CMD_PUT:
			case SIO_CMD_PUT_HI:
			case SIO_CMD_WRITE:
			case SIO_CMD_WRITE_HI:
				if ( dev->data_frame[0] == SIO_RESP_ACK ) {
					for ( i = 0; i < (unsigned int)dsk->sector_size; ++i ) {
						dev->data_frame[2+i] = rd_6502(addr+i);
					}

					dev->data_frame[0] = sio_write_sector ( dsk, sector, dev->data_frame+2 );

					if ( dev->data_frame[0] == SIO_RESP_ACK )
						dev->data_frame[1] = SIO_RESP_DONE;
					else if ( dev->data_frame[0] == SIO_RESP_ERROR )
						dev->data_frame[1] = SIO_RESP_ERROR;
				}
				break;
			case SIO_CMD_STATUS:
				if ( dev->data_frame[1] == SIO_RESP_DONE ) {
					for ( i = 0; i < 4; ++i ) {
						wr_6502((addr+i), dev->data_frame[2+i]);
					}
				}
				break;
			case SIO_CMD_FORMAT:
			case SIO_CMD_FORMAT_HI:
				break;
			case SIO_CMD_DUAL_FORMAT:
			case SIO_CMD_DUAL_FORMAT_HI:
				break;
			case SIO_CMD_DOWNLOAD:
				break;
			case SIO_CMD_READADDR:
				break;
			case SIO_CMD_READ_SPIN:
				break;
			case SIO_CMD_MOTOR_ON:
				break;
			case SIO_CMD_VERIFY_SECTOR:
				break;
			default:
				dev->data_frame[1] = SIO_RESP_NAK;
				break;
		}
	}

	/*
	 * Else this is a cassette
	 */
	else if ( rd_6502(0x300) == 0x60 ) {

		dev = &sio.cassette.dev;
		block = &sio.cassette.blk[dev->current_block];
		cas_delay = block->mark_tone_size + (dev->baud * block->size);

		dev->data_frame[1] = SIO_RESP_DONE;

		if ( gap )
			gap_delay = (unsigned int)(15720 * 260 / 1000) * 114;
		else 
			gap_delay = 34407936; /* 20 seconds */

		switch ( sio.cmd_frame[1] ) {
			case SIO_CMD_READ:
				/*
				while ( gap_delay > cas_delay ) {
					++(dev->current_block);
					block = &sio.cassette.blk[dev->current_block];
					cas_delay += block->mark_tone_size + (dev->baud * block->size);
				}
				*/
				//if ( !sio.motor_on )
				//	dev->data_frame[1] = 0;
				if ( dev->current_block < dev->num_blocks ) {
					for ( i = 0; i < block->size; ++i ) {
						wr_6502((addr+i), block->data_ptr[i]);
					}
					++(dev->current_block);
				}
				else {
					dev->data_frame[1] = SIO_RESP_ERROR;
				}
				break;
			case SIO_CMD_WRITE:
				if ( dev->current_block < dev->num_blocks ) {
					block->mark_tone_size = gap_delay;
					for ( i = 0; i < block->size; ++i ) {
						block->data_ptr[i] = rd_6502(addr+i);
					}
					++(dev->current_block);
				}
				else {
					dev->data_frame[1] = SIO_RESP_ERROR;
				}
				sio_write_cassette ( &sio.cassette );
				break;
			default:
				dev->data_frame[1] = SIO_RESP_NAK;
				break;
		}

	} /* end if cassette */

	/*
	 * Else issue a device timeout
	 */
	else {
		dev->data_frame[1] = 0;
	}

	/*
	 * Now set the result and the cpu registers
	 */
	switch ( dev->data_frame[1] ) {
		case 0x00:
			cpu->y_reg = 138;
			cpu->status |= B6502_SIGN_FLG;
			break;
		case SIO_RESP_ACK:
		case SIO_RESP_DONE:
			cpu->y_reg = 1;
			cpu->status &= NOT_B6502_SIGN_FLG;
			break;
		case SIO_RESP_NAK:
			cpu->y_reg = 144;
			cpu->status |= B6502_SIGN_FLG;
			break;
		case SIO_RESP_ERROR:
		default:
			cpu->y_reg = 146;
			cpu->status |= B6502_SIGN_FLG;
			break;
	}

	cpu->a_reg = 0;
	cpu->status |= B6502_CARRY_FLG;
	wr_6502(0x303, cpu->y_reg);
	wr_6502(0x42, 0);

} /* end sio_patch */

