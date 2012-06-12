/******************************************************************************
*
* FILENAME: states.c
*
* DESCRIPTION:  This handles saving and recall emulator states
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.2     12/10/05  bberlin      Creation
******************************************************************************/
//#include <SDL/SDL.h>
#include <stdlib.h>
#include <string.h>
#include "../core/gtia.h"
#include "../core/console.h"
#include "../core/memory.h"
#include "db_if.h"
#include "logger.h"
#include "media.h"
#include "util.h"
#include "kconfig.h"
#include "sound.h"

#define ANTIC_CHIP 1
#define GTIA_CHIP  2
#define POKEY_CHIP 3
#define PIA_CHIP   4

extern t_atari_sound g_sound;
extern BYTE memory[0x10000];

extern t_sio_interface sio;

void state_append_int_value ( char *statement, int value );

int callback_called;

/******************************************************************************
**  Function   :  states_save_sio_devices
**                            
**  Objective  :  This function saves the sio dev configuration to the database
**
**  Parameters :  dev       - device to write to database              
**                statement - statement to test for the id
**                a_values  - addition values based on what type of device
**                sioid     - where to store the id to insert into the State
**                                                
**  return     :  Error code
**      
******************************************************************************/
int states_save_sio_devices ( t_sio_device *dev, char *statement, char *a_value, int *devid ) {

	char values[1224];
	int status;
	int temp_int;
	char *p_tmp;

	*devid = 0;

	status = db_if_exec_sql ( statement, db_if_id_callback, devid );

	/*
	 * FIXME: Figure out what current_data should be
	 */ 
	temp_int = 0;

	if ( !dev->f_buffer )
		dev->buffer_size = 0;

	sprintf ( values, "'%s', NULL, %d, %d, %d, %d, %d, 0, 0, %d, %d, %d, %d, %d, %d, %s", 
	             p_tmp = util_replace_and_copy(dev->filename, "'", "''" ),
	             dev->buffer_size, dev->num_blocks, dev->current_block, 
	             dev->current_byte, dev->expected_bytes, dev->header_size, 
	             dev->baud, temp_int, dev->action_time, dev->status, dev->type, 
	             a_value );

	free ( p_tmp );

	db_if_insert_or_update ( statement, values, "StatesSioDevices", devid );

	if ( dev->f_buffer ) {
		sprintf ( statement, "UPDATE StatesSioDevices SET Fbuffer = ? WHERE StatesSioDevicesID = '%d'", *devid ); 
		status = db_if_exec_single_blob ( statement, dev->f_buffer, dev->buffer_size );
	}

	sprintf ( statement, "UPDATE StatesSioDevices SET Header = ? WHERE StatesSioDevicesID = '%d'", *devid ); 
	status = db_if_exec_single_blob ( statement, dev->header, 16 * sizeof(BYTE) );

	sprintf ( statement, "UPDATE StatesSioDevices SET DataFrame = ? WHERE StatesSioDevicesID = '%d'", *devid ); 
	status = db_if_exec_single_blob ( statement, dev->data_frame, 260 * sizeof(BYTE) );
	return 0;

} /* states_save_sio_devices */

/******************************************************************************
**  Function   :  states_save_sio
**                            
**  Objective  :  This function saves the sio configuration to the database
**
**  Parameters :  statement - statement to test for the id
**                sioid     - where to store the id to insert into the State
**                                                
**  return     :  Error code
**      
******************************************************************************/
int states_save_sio ( char *statement, int *sioid ) {

	char tstatement[1024];
	char values[512];
	char a_values[512];
	int i, status, temp_int;
	int id;

	/*
	 * Determine if there is a saved SIO state for this already
	 */
	status = db_if_exec_sql ( statement, db_if_id_callback, sioid );

	/*
	 * Figure out the current device begin used
	 */
	temp_int = MAX_SIO_DEVICES + 1;
	for ( i = 0; i < MAX_SIO_DEVICES; ++i ) {
		if ( sio.current_device == &sio.drives[i].dev )
			temp_int = i;
	}
	sprintf ( values, "%d, %d, %d, 0, %d", temp_int,
		            sio.motor_on, sio.motor_on_time, sio.cmd_index );	

	/*
	 * Save the 8 disk drives
	 */
	for ( i = 0; i < MAX_SIO_DEVICES; ++i ) {
		sprintf ( tstatement, "SELECT Disk%d FROM StatesSio WHERE StatesSioID='%d'", i, *sioid );
		sprintf ( a_values, "%d, %d, %d, NULL, NULL, NULL, NULL", 
		              sio.drives[i].write_protect, sio.drives[i].sector_size, 
		              sio.drives[i].dsk_type );
		states_save_sio_devices ( &sio.drives[i].dev, tstatement, a_values, &id );
		sprintf ( values, "%s, %d", values, id );
	}

	/*
	 * Save the Cassette
	 */
	sprintf ( tstatement, "SELECT Cas FROM StatesSio WHERE StatesSioID='%d'", *sioid );
	sprintf ( a_values, "NULL, NULL, NULL, 0, %d, %d, '%s'", 
	                  sio.cassette.record_pressed, sio.cassette.motor_gap_delay, 
	                  sio.cassette.description );
	states_save_sio_devices ( &sio.cassette.dev, tstatement, a_values, &id );
	sprintf ( values, "%s, %d", values, id );

	/*
	 * Insert or Update the SIO state and update the cmd frame blob
	 */
	db_if_insert_or_update ( statement, values, "StatesSio", sioid );

	sprintf ( statement, "UPDATE StatesSio SET CmdFrame = ? WHERE StatesSioID = '%d'", *sioid ); 
	status = db_if_exec_single_blob ( statement, sio.cmd_frame, 6 * sizeof(BYTE) );

	return 0;

} /* states_save_sio */

/******************************************************************************
**  Function   :  save_state
**                            
**  Objective  :  This function saves the machine state to the database
**
**  Parameters :  name   - name of state to save to
**                atari  - pointer to system struct to save
**                                                
**  return     :  Error code
**      
******************************************************************************/
int save_state ( char *name, struct atari *atari ) {

	unsigned int tmp_hex_data;
	unsigned int *temp_mem;
	char temp[20];
	int i, status;
	int sioid = 0;
	int portb, mem_image;
	t_media *p_media;

	char statement[3000];

	p_media = media_get_ptr();

	db_if_exec_sql ( "BEGIN", 0, 0 );

	/*
	 * Save SIO stuff and determine ID first
	 */
	sprintf ( statement, "SELECT StatesSioID FROM States WHERE Name='%s'", name );
	states_save_sio ( statement, &sioid );

	/*
	 * Now save the State
	 */
	sprintf ( statement, "INSERT OR REPLACE INTO States VALUES ( '%s'", name );

	/*
	 * Save Cart and Machine and RAM
	 */
	sprintf ( temp, ", %lu", p_media->crc );
	strcat ( statement, temp );
	state_append_int_value ( statement, console_get_machine_type() );
	state_append_int_value ( statement, sioid );
	state_append_int_value ( statement, console_get_ptr()->ram_size );
	state_append_int_value ( statement, console_get_ptr()->cart->loaded );
	state_append_int_value ( statement, console_get_ptr()->basic_enable );
	state_append_int_value ( statement, console_get_ptr()->option_key_pressed );
	state_append_int_value ( statement, console_get_ptr()->start_key_pressed );

	/*
	 * Memory Stuff
	 */
	mem_get_state_values ( &portb, &mem_image  );
	state_append_int_value ( statement, portb );
	state_append_int_value ( statement, mem_image );

	/*
	 * Save 6502
	 */
	state_append_int_value ( statement, atari->cpu->pc );
	state_append_int_value ( statement, atari->cpu->a_reg );
	state_append_int_value ( statement, atari->cpu->x_reg );
	state_append_int_value ( statement, atari->cpu->y_reg );
	state_append_int_value ( statement, atari->cpu->status );
	state_append_int_value ( statement, atari->cpu->stack_ptr );
	state_append_int_value ( statement, 0 );

	/*
	 * Save ANTIC
	 */
	state_append_int_value ( statement, atari->antic->msc );
	state_append_int_value ( statement, atari->antic->dmactl_reg );
	state_append_int_value ( statement, atari->antic->chactl_reg );
	state_append_int_value ( statement, atari->antic->dlistl_reg );
	state_append_int_value ( statement, atari->antic->dlisth_reg );
	state_append_int_value ( statement, (32-atari->antic->hscroll_reg_value) );
	state_append_int_value ( statement, atari->antic->vscroll_reg_value );
	state_append_int_value ( statement, atari->antic->pm_base_address >> 8 );
	state_append_int_value ( statement, atari->antic->ch_base_address >> 8 );
	state_append_int_value ( statement, atari->antic->vcount );
	state_append_int_value ( statement, 0 );
	state_append_int_value ( statement, 0 );
	tmp_hex_data = atari->antic->nmi_dli_en | atari->antic->nmi_vbi_en | atari->antic->nmi_reset_en;
	state_append_int_value ( statement, tmp_hex_data );
	state_append_int_value ( statement, atari->antic->nmi_status_reg );

	/*
	 * Save GTIA
	 */
	state_append_int_value ( statement, atari->gtia->hpos_p0_reg );
	state_append_int_value ( statement, atari->gtia->hpos_p1_reg );
	state_append_int_value ( statement, atari->gtia->hpos_p2_reg );
	state_append_int_value ( statement, atari->gtia->hpos_p3_reg );
	state_append_int_value ( statement, atari->gtia->hpos_m0_reg );
	state_append_int_value ( statement, atari->gtia->hpos_m1_reg );
	state_append_int_value ( statement, atari->gtia->hpos_m2_reg );
	state_append_int_value ( statement, atari->gtia->hpos_m3_reg );
	state_append_int_value ( statement, atari->gtia->w_reg[SIZEP0] );
	state_append_int_value ( statement, atari->gtia->w_reg[SIZEP1] );
	state_append_int_value ( statement, atari->gtia->w_reg[SIZEP2] );
	state_append_int_value ( statement, atari->gtia->w_reg[SIZEP3] );
	state_append_int_value ( statement, atari->gtia->w_reg[SIZEM] );
	state_append_int_value ( statement, atari->gtia->graph_p0_reg );
	state_append_int_value ( statement, atari->gtia->graph_p1_reg );
	state_append_int_value ( statement, atari->gtia->graph_p2_reg );
	state_append_int_value ( statement, atari->gtia->graph_p3_reg );
	state_append_int_value ( statement, atari->gtia->graph_m_reg );
	state_append_int_value ( statement, atari->gtia->color_pf0_reg );
	state_append_int_value ( statement, atari->gtia->color_pf1_reg );
	state_append_int_value ( statement, atari->gtia->color_pf2_reg );
	state_append_int_value ( statement, atari->gtia->color_pf3_reg );
	state_append_int_value ( statement, atari->gtia->color_pm0_reg );
	state_append_int_value ( statement, atari->gtia->color_pm1_reg );
	state_append_int_value ( statement, atari->gtia->color_pm2_reg );
	state_append_int_value ( statement, atari->gtia->color_pm3_reg );
	state_append_int_value ( statement, atari->gtia->color_bk_reg );
	state_append_int_value ( statement, gtia_read_register(M0PF) );
	state_append_int_value ( statement, gtia_read_register(M1PF) );
	state_append_int_value ( statement, gtia_read_register(M2PF) );
	state_append_int_value ( statement, gtia_read_register(M3PF) );
	state_append_int_value ( statement, gtia_read_register(P0PF) );
	state_append_int_value ( statement, gtia_read_register(P1PF) );
	state_append_int_value ( statement, gtia_read_register(P2PF) );
	state_append_int_value ( statement, gtia_read_register(P3PF) );
	state_append_int_value ( statement, gtia_read_register(M0PL) );
	state_append_int_value ( statement, gtia_read_register(M1PL) );
	state_append_int_value ( statement, gtia_read_register(M2PL) );
	state_append_int_value ( statement, gtia_read_register(M3PL) );
	state_append_int_value ( statement, gtia_read_register(P0PL) );
	state_append_int_value ( statement, gtia_read_register(P1PL) );
	state_append_int_value ( statement, gtia_read_register(P2PL) );
	state_append_int_value ( statement, gtia_read_register(P3PL) );
	state_append_int_value ( statement, atari->gtia->trig[0] );
	state_append_int_value ( statement, atari->gtia->trig[1] );
	state_append_int_value ( statement, atari->gtia->trig[2] );
	state_append_int_value ( statement, atari->gtia->trig[3] );
	state_append_int_value ( statement, atari->gtia->prior_reg );
	tmp_hex_data = atari->gtia->latch_triggers | atari->gtia->player_dma | atari->gtia->missle_dma;
	state_append_int_value ( statement, tmp_hex_data );
	state_append_int_value ( statement, atari->gtia->vdelay_reg );
	state_append_int_value ( statement, atari->gtia->pal_or_ntsc );
	if ( console_get_machine_type() == MACHINE_TYPE_5200 )
		state_append_int_value ( statement, atari->gtia->console_reg );
	else
		state_append_int_value ( statement, gtia_read_register(CONSOL) );

	/*
	 * Save POKEY
	 */
	state_append_int_value ( statement, atari->pokey->w_reg[AUDF1] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDF2] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDF3] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDF4] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDC1] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDC2] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDC3] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDC4] );
	state_append_int_value ( statement, atari->pokey->w_reg[AUDCTL] );
	state_append_int_value ( statement, atari->pokey->w_reg[STIMER] );
	state_append_int_value ( statement, atari->pokey->sk_status );
	state_append_int_value ( statement, atari->pokey->sk_ctl );
	state_append_int_value ( statement, atari->pokey->serial_out );
	state_append_int_value ( statement, atari->pokey->serial_in );
	state_append_int_value ( statement, atari->pokey->irq_status );
	state_append_int_value ( statement, atari->pokey->irq_enable );
	state_append_int_value ( statement, atari->pokey->kbcode );
	state_append_int_value ( statement, atari->pokey->r_reg[POT0] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT1] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT2] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT3] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT4] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT5] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT6] );
	state_append_int_value ( statement, atari->pokey->r_reg[POT7] );
	state_append_int_value ( statement, atari->pokey->r_reg[ALLPOT] );

	/*
	 * Save PIA
	 */
	state_append_int_value ( statement, pia_read_register(PORTA) );
	state_append_int_value ( statement, pia_read_register(PORTB) );
	state_append_int_value ( statement, atari->pia->direction[PORTA] );
	state_append_int_value ( statement, atari->pia->direction[PORTB] );
	state_append_int_value ( statement, atari->pia->port_control[PACTL&0x1] );
	state_append_int_value ( statement, atari->pia->port_control[PBCTL&0x1] );

	/*
	 * Save Binary BLOBs
	 */
	state_append_int_value ( statement, 0 );
	state_append_int_value ( statement, 0 );
	state_append_int_value ( statement, 0 );
	state_append_int_value ( statement, 0 );
	state_append_int_value ( statement, 0 );
	
	strcat ( statement, " )" );

	status = db_if_exec_sql ( statement, 0, 0 );

	if ( status ) {
		db_if_exec_sql ( "END", 0, 0 );
		return -1;
	}

	/*
	 * Now update with the actual BLOBS
	 */
	sprintf ( statement, "UPDATE States SET CPU_STACK = ? WHERE Name = '%s'", name ); 
	status = db_if_exec_single_blob ( statement, atari->cpu->stack, 0x100 * sizeof(BYTE) );

	sprintf ( statement, "UPDATE States SET Memory = ? WHERE Name = '%s'", name ); 
	status = db_if_exec_single_blob ( statement, memory, 0x10000 * sizeof(BYTE) );

	sprintf ( statement, "UPDATE States SET BankSwitchCart = (SELECT Image FROM CurrentMedia WHERE Name = 'Cart') WHERE Name = '%s'", name ); 
	status = db_if_exec_sql ( statement, 0, 0 );

	/*
	 * For the XL/XE, save OS, BASIC, and Banked RAM seperately
	 */
	if ( console_get_machine_type() == MACHINE_TYPE_XL ) {

		temp_mem = malloc ( 0x10000 * sizeof(BYTE) );
		for ( i = 0; i < 0x10000; ++i ) {
			temp_mem[i] = read_xe_bank_state(i);
		}
		sprintf ( statement, "UPDATE States SET XEBankedRAM = ? WHERE Name = '%s'", name ); 
		status = db_if_exec_single_blob ( statement, temp_mem, 0x10000 * sizeof(BYTE) );

		for ( i = 0; i < 0x4000; ++i ) {
			temp_mem[i] = read_os_rom(i);
		}
		sprintf ( statement, "UPDATE States SET BIOS = ? WHERE Name = '%s'", name ); 
		status = db_if_exec_single_blob ( statement, temp_mem, 0x4000 * sizeof(BYTE) );

		for ( i = 0; i < 0x2000; ++i ) {
			temp_mem[i] = read_basic_rom(i);
		}
		sprintf ( statement, "UPDATE States SET BASIC = ? WHERE Name = '%s'", name ); 
		status = db_if_exec_single_blob ( statement, temp_mem, 0x2000 * sizeof(BYTE) );

		free ( temp_mem );

	} /* end if an XL/XE system */

	db_if_exec_sql ( "END", 0, 0 );

	return 0;

} /* end save_state */

/******************************************************************************
**  Function   :  states_sio_dev_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  dev       - pointer to device struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int states_sio_dev_callback ( void *dev, int argc, char **argv, char **azColName ) {

	int i;
	int temp_int;
	t_sio_device *p_dev = dev;

	if ( argc < 15 )
		return -1;

	i = 0;
	if ( argv[++i] ) strcpy ( p_dev->filename, argv[i] );
	++i;
	if ( argv[++i] ) p_dev->buffer_size = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->num_blocks = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->current_block = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->current_byte = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->expected_bytes = atoi ( argv[i] );
	++i;
	++i;
	if ( argv[++i] ) p_dev->header_size = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->baud = atoi ( argv[i] );
	if ( argv[++i] ) temp_int = atoi ( argv[i] ); /* FIXME: Current Data Pointer */
	if ( argv[++i] ) p_dev->action_time = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->status = atoi ( argv[i] );
	if ( argv[++i] ) p_dev->type = atoi ( argv[i] );

	return 0;

} /* end states_sio_dev_callback */

/******************************************************************************
**  Function   :  states_sio_disk_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  disk      - pointer to dsk device struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int states_sio_disk_callback ( void *disk, int argc, char **argv, char **azColName ) {

	int i;
	t_dsk_device *p_disk = disk;

	states_sio_dev_callback ( &p_disk->dev, argc, argv, azColName );

	i = 15;
	if ( argv[++i] ) p_disk->write_protect = atoi ( argv[i] );
	if ( argv[++i] ) p_disk->sector_size = atoi ( argv[i] );
	if ( argv[++i] ) p_disk->dsk_type = atoi ( argv[i] );

	return 0;

} /* end states_sio_disk_callback */

/******************************************************************************
**  Function   :  states_sio_cas_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  cas       - pointer to cas device struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int states_sio_cas_callback ( void *cas, int argc, char **argv, char **azColName ) {

	int i;
	t_cas_device *p_cas = cas;

	states_sio_dev_callback ( &p_cas->dev, argc, argv, azColName );

	i = 19;
	if ( argv[++i] ) p_cas->record_pressed = atoi ( argv[i] );
	if ( argv[++i] ) p_cas->motor_gap_delay = atoi ( argv[i] );
	if ( argv[++i] ) strcpy ( p_cas->description, argv[i] );

	return 0;

} /* end states_sio_cas_callback */

/******************************************************************************
**  Function   :  states_sio_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  sio       - pointer to sio struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int states_sio_callback ( void *sio, int argc, char **argv, char **azColName ) {

	int i,j;
	int temp_int = 0;
	int devid = 0;
	char statement[257];
	t_sio_interface *p_sio = sio;

	if ( argc < 15 )
		return -1;

	i = 0;
	if ( argv[++i] ) temp_int = atoi ( argv[i] );

	if ( temp_int < 0 || temp_int >= MAX_SIO_DEVICES )
		p_sio->current_device = &p_sio->cassette.dev;
	else
		p_sio->current_device = &p_sio->drives[temp_int].dev;

	if ( argv[++i] ) p_sio->motor_on = atoi ( argv[i] );
	if ( argv[++i] ) p_sio->motor_on_time = atoi ( argv[i] );
	++i;
	if ( argv[++i] ) p_sio->cmd_index = atoi ( argv[i] );

	/*
	 * Load up the Disks
	 */
	for ( j = 0; j < 8; ++j ) {
		if ( argv[++i] ) devid = atoi ( argv[i] );
		sprintf ( statement, "SELECT * FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
		db_if_exec_sql ( statement, states_sio_disk_callback, &p_sio->drives[j] );

		if ( p_sio->drives[j].dev.buffer_size ) {
			p_sio->drives[j].dev.f_buffer = malloc ( p_sio->drives[j].dev.buffer_size );
			sprintf ( statement, "SELECT Fbuffer FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
			db_if_query_single_blob ( statement, p_sio->drives[j].dev.f_buffer, p_sio->drives[j].dev.buffer_size );
		}

		sprintf ( statement, "SELECT Header FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
		db_if_query_single_blob ( statement, p_sio->drives[j].dev.header, 16 * sizeof(BYTE) );

		sprintf ( statement, "SELECT DataFrame FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
		db_if_query_single_blob ( statement, p_sio->drives[j].dev.data_frame, 260 * sizeof(BYTE) );
	}

	/*
	 * Load up the Cassette
	 */
	if ( argv[++i] ) devid = atoi ( argv[i] );
	sprintf ( statement, "SELECT * FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
	db_if_exec_sql ( statement, states_sio_cas_callback, &p_sio->cassette );

	if ( p_sio->cassette.dev.buffer_size ) {
		p_sio->cassette.dev.f_buffer = malloc ( p_sio->cassette.dev.buffer_size );
		sprintf ( statement, "SELECT Fbuffer FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
		db_if_query_single_blob ( statement, p_sio->cassette.dev.f_buffer, p_sio->cassette.dev.buffer_size );
	}

	sprintf ( statement, "SELECT Header FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
	db_if_query_single_blob ( statement, p_sio->cassette.dev.header, 16 * sizeof(BYTE) );

	sprintf ( statement, "SELECT DataFrame FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
	db_if_query_single_blob ( statement, p_sio->cassette.dev.data_frame, 260 * sizeof(BYTE) );

	sprintf ( statement, "SELECT Blk FROM StatesSioDevices WHERE StatesSioDevicesID='%d'", devid );
	db_if_query_single_blob ( statement, p_sio->cassette.blk, sizeof(t_cas_block) * MAX_CAS_BLOCKS );

	return 0;

} /* end states_sio_callback */

/******************************************************************************
**  Function   :  recall_states_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  atari     - pointer to console struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int recall_states_callback ( void *atari, int argc, char **argv, char **azColName ) {

	char statement[257];
	int i;
	int sioid = 0;
	struct atari *p_atari = atari;
	int portb = 0xff, mem_image = 0;
	t_media *p_media = media_get_ptr();
	t_config *p_config = config_get_ptr();

	if ( argc < 120 )
		return -1;

	callback_called = 1;

	/*
	 * THIS DEPENDS ON MATCHING THE ORDER IN THE DATABASE!
	 */
	i = 0;
	if ( argv[++i] ) p_media->crc = strtoul ( argv[i], NULL, 0 );
	if ( argv[++i] ) console_set_machine_type ( atoi( argv[i]) );
	if ( argv[++i] ) sioid = atoi ( argv[i] );
	if ( argv[++i] ) console_get_ptr()->ram_size = atoi( argv[i] );
	console_reset ( console_get_machine_type(), p_config->system_type, 
	                console_get_ptr()->ram_size, g_sound.freq );

	if ( argv[++i] ) console_get_ptr()->cart->loaded = atoi ( argv[i] );
	if ( argv[++i] ) console_get_ptr()->basic_enable = atoi ( argv[i] );
	if ( argv[++i] ) console_get_ptr()->option_key_pressed = atoi ( argv[i] );
	if ( argv[++i] ) console_get_ptr()->start_key_pressed = atoi ( argv[i] );

	if ( argv[++i] ) portb = atoi ( argv[i] );
	if ( argv[++i] ) mem_image = atoi ( argv[i] );
	mem_set_state_values ( portb, mem_image  );

	/*
	 * CPU
	 */
	if ( argv[++i] ) p_atari->cpu->pc = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->cpu->a_reg = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->cpu->x_reg = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->cpu->y_reg = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->cpu->status = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->cpu->stack_ptr = atoi ( argv[i] );
	++i;

	/*
	 * ANTIC
	 */
	if ( argv[++i] ) p_atari->antic->msc = atoi ( argv[i] );
	if ( argv[++i] ) antic_write_register ( DMACTL, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( CHACTL, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( DLISTL, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( DLISTH, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( HSCROL, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( VSCROL, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( PMBASE, atoi ( argv[i] ) );
	if ( argv[++i] ) antic_write_register ( CHBASE, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->antic->vcount = atoi ( argv[i] );
	i+=2;
	if ( argv[++i] ) antic_write_register ( NMIEN, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->antic->nmi_status_reg = atoi ( argv[i] );

	/*
	 * GTIA
	 */
	if ( argv[++i] ) gtia_write_register ( HPOSP0, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSP1, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSP2, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSP3, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSM0, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSM1, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSM2, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( HPOSM3, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( SIZEP0, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( SIZEP1, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( SIZEP2, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( SIZEP3, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( SIZEM, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( GRAFP0, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( GRAFP1, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( GRAFP2, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( GRAFP3, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( GRAFM, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPF0, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPF1, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPF2, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPF3, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPM0, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPM1, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPM2, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLPM3, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( COLBK, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_collision ( M0PF, p_atari->gtia->m0pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M1PF, p_atari->gtia->m1pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M2PF, p_atari->gtia->m2pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M3PF, p_atari->gtia->m3pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P0PF, p_atari->gtia->p0pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P1PF, p_atari->gtia->p1pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P2PF, p_atari->gtia->p2pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P3PF, p_atari->gtia->p3pf_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M0PL, p_atari->gtia->m0pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M1PL, p_atari->gtia->m1pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M2PL, p_atari->gtia->m2pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( M3PL, p_atari->gtia->m3pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P0PL, p_atari->gtia->p0pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P1PL, p_atari->gtia->p1pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P2PL, p_atari->gtia->p2pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) gtia_write_collision ( P3PL, p_atari->gtia->p3pl_reg = atoi(argv[i]) );
	if ( argv[++i] ) p_atari->gtia->trig[0] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->gtia->trig[1] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->gtia->trig[2] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->gtia->trig[3] = atoi ( argv[i] );
	if ( argv[++i] ) gtia_write_register ( PRIOR, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( GRACTL, atoi ( argv[i] ) );
	if ( argv[++i] ) gtia_write_register ( VDELAY, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->gtia->pal_or_ntsc = atoi ( argv[i] );
	if ( argv[++i] ) {
		if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
			gtia_write_register ( CONSOL, atoi ( argv[i] ) );
		}
		else {
			gtia_write_console ( atoi ( argv[i] ) ); 
		}
	}

	/*
	 * POKEY
	 */
	if ( argv[++i] ) pokey_write_register ( AUDF1, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDF2, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDF3, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDF4, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDC1, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDC2, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDC3, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDC4, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( AUDCTL, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->pokey->sk_status = atoi ( argv[i] );
	if ( argv[++i] ) pokey_write_register ( SKCTL, atoi ( argv[i] ) );
	if ( argv[++i] ) pokey_write_register ( SEROUT, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->pokey->serial_in = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->irq_status = atoi ( argv[i] );
	if ( argv[++i] ) pokey_write_register ( IRQEN, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->pokey->kbcode = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT0] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT1] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT2] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT3] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT4] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT5] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT6] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[POT7] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pokey->r_reg[ALLPOT] = atoi ( argv[i] );

	/*
	 * PIA
	 */
	if ( argv[++i] ) pia_recall_port_registers ( PORTA, atoi ( argv[i] ) );
	if ( argv[++i] ) pia_recall_port_registers ( PORTB, atoi ( argv[i] ) );
	if ( argv[++i] ) p_atari->pia->direction[PORTA] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pia->direction[PORTB] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pia->port_control[PACTL&0x1] = atoi ( argv[i] );
	if ( argv[++i] ) p_atari->pia->port_control[PBCTL&0x1] = atoi ( argv[i] );

	/*
	 * SIO
	 */
	sio_unmount_all ( );
	sprintf ( statement, "SELECT * FROM StatesSio WHERE StatesSioID='%d'", sioid );
	db_if_exec_sql ( statement, states_sio_callback, &sio );
	sprintf ( statement, "SELECT CmdFrame FROM StatesSio WHERE StatesSioID='%d'", sioid );
	db_if_query_single_blob ( statement, sio.cmd_frame, 6 * sizeof(BYTE) );

	return 0;

} /* end recall_states_callback */

/******************************************************************************
**  Function   :  recall_state
**                            
**  Objective  :  This function loads the machine state from the database
**
**  Parameters :  name   - name of state to recall
**                atari  - pointer to system struct to save
**                                                
**  return     :  Error code
**      
******************************************************************************/
int recall_state ( const char *name, struct atari *atari ) {

	char statement[1024];
	BYTE *temp_mem;
	int i, status;
	t_config *p_config = config_get_ptr();

	/*
	 * Set it to 5200 in case of old state files
	 */
	console_set_machine_type ( MACHINE_TYPE_5200 );

	/*
	 * Reset the machine in case we do this right at start
	 */
	console_reset ( p_config->machine_type, p_config->system_type, 
	                p_config->ram_size[p_config->machine_type], g_sound.freq );

	callback_called = 0;

	db_if_exec_sql ( "BEGIN", 0, 0 );

	sprintf ( statement, "SELECT * FROM States WHERE Name='%s'", name );
	status = db_if_exec_sql ( statement, recall_states_callback, atari );
	if ( status || !callback_called ) {
		db_if_exec_sql ( "END", 0, 0 );
		return -1;
	}

	sprintf ( statement, "SELECT CPU_STACK FROM States WHERE Name='%s'", name );
	db_if_query_single_blob ( statement, atari->cpu->stack, 0x100 * sizeof(BYTE) );

	sprintf ( statement, "SELECT Memory FROM States WHERE Name='%s'", name );
	db_if_query_single_blob ( statement, memory, 0x10000 * sizeof(BYTE) );

	sprintf ( statement, "UPDATE CurrentMedia SET Image = (SELECT BankSwitchCart FROM States WHERE Name = '%s') WHERE Name = 'Cart'", name ); 
	status = db_if_exec_sql ( statement, 0, 0 );

	/*
	 * For the XL/XE, save OS, BASIC, and Banked RAM seperately
	 */
	if ( console_get_machine_type() == MACHINE_TYPE_XL ) {

		temp_mem = malloc ( 0x10000 * sizeof(BYTE) );

		sprintf ( statement, "SELECT XEBankedRAM FROM States WHERE Name='%s'", name );
		db_if_query_single_blob ( statement, temp_mem, 0x10000 * sizeof(BYTE) );
		for ( i = 0; i < 0x10000; ++i ) {
			write_xe_bank_state(i, temp_mem[i]);
		}

		sprintf ( statement, "SELECT BIOS FROM States WHERE Name='%s'", name );
		db_if_query_single_blob ( statement, temp_mem, 0x4000 * sizeof(BYTE) );
		for ( i = 0; i < 0x4000; ++i ) {
			write_os_rom(i, temp_mem[i]);
		}

		sprintf ( statement, "SELECT BASIC FROM States WHERE Name='%s'", name );
		db_if_query_single_blob ( statement, temp_mem, 0x2000 * sizeof(BYTE) );
		for ( i = 0; i < 0x2000; ++i ) {
			write_basic_rom(i, temp_mem[i]);
		}

		free ( temp_mem );

	} /* end if an XL/XE system */

	sprintf ( statement, "recall_state: Loaded state \"%s\"", name );
	logger_log_message ( LOG_INFO, statement, "" );

	db_if_exec_sql ( "END", 0, 0 );

	return 0;

} /* end recall_state */

/******************************************************************************
**  Function   :  save_quick_state
**                            
**  Objective  :  This function saves the state to the selected slot
**
**  Parameters :  slot   - name of file to save state to
**                atari  - pointer to system struct to save
**                                                
**  return     :  Error code
**      
******************************************************************************/
int save_quick_state ( int slot, struct atari *atari ) {

	char name[50];

	sprintf ( name, "QS%d", slot );

	return save_state ( name, atari );

} /* end save_quick_state */

/******************************************************************************
**  Function   :  recall_quick_state
**                            
**  Objective  :  This function loads the state from a slot
**
**  Parameters :  slot   - slot to use when accessing the state
**                atari  - pointer to system struct to load
**                                                
**  return     :  Error code
**      
******************************************************************************/
int recall_quick_state ( int slot, struct atari *atari ) {

	char name[50];

	sprintf ( name, "QS%d", slot );

	return recall_state ( name, atari );

} /* end recall_quick_state */

int states_exists_callback ( void *atari, int argc, char **argv, char **azColName ) {
	callback_called = 1;
	return 0;
}

/******************************************************************************
**  Function   :  state_get_qs_existance
**                            
**  Objective  :  This function determines which quick states are in the db
**
**  Parameters :  exists - an array to store 1 for exists, 0 otherwise
**                                                
**  return     :  Error code
**      
******************************************************************************/
int states_get_qs_existance ( int *exists ) {

	char statement[80];
	int i, status;

	for ( i = 0; i < 9; ++i ) {
		exists[i] = 0;
	}

	db_if_exec_sql ( "BEGIN", 0, 0 );

	for ( i = 0; i < 9; ++i ) {
		callback_called = 0;
		sprintf ( statement, "SELECT * FROM States WHERE Name='QS%d'", i+1 );
		status = db_if_exec_sql ( statement, states_exists_callback, 0 );
		if ( status )
			break;

		exists[i] = callback_called;
	}

	db_if_exec_sql ( "END", 0, 0 );

	return status;

} /* end states_get_qs_existance */

/******************************************************************************
**  Function   :  convert_atari800_state
**                            
**  Objective  :  This function converts the input atari800 state file to 
**                kat5200 file
**
**  Parameters :  filename  - name of file to save
**                                                
**  return     :  Error code
**      
******************************************************************************/
int convert_atari800_state ( char *filename ) {

	struct atari5200_console *atari;

	atari = malloc ( sizeof(struct atari) );

	free (atari);

	return 0;

} /* end convert_atari800_state */

/******************************************************************************
**  Function   :  state_append_int_value
**                            
**  Objective  :  This function appends an integer value to the SQL statement
**
**  Parameters :  statement - SQL statement to append to
**                value     - integer value to append
**                                                
**  return     :  Error code
**      
******************************************************************************/
void state_append_int_value ( char *statement, int value ) {

	char temp[20];

	sprintf ( temp, ", %d", value );
	strcat ( statement, temp );

} /* end state_append_int_value */

