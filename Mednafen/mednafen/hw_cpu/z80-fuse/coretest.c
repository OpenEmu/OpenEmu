/* coretest.c: Test program for Fuse's Z80 core
   Copyright (c) 2003 Philip Kendall

   $Id: coretest.c,v 1.22 2004/06/09 10:55:08 pak21 Exp $

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

   Author contact information:

   E-mail: pak21-fuse@srcf.ucam.org
   Postal address: 15 Crescent Road, Wokingham, Berks, RG40 2DB, England

*/

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spectrum.h"
#include "ui/ui.h"
#include "z80.h"
#include "z80_macros.h"

static const char *progname;		/* argv[0] */

static int init_dummies( void );

uint32 z80_tstates;
uint32 event_next_event;

/* 64Kb of RAM */
static uint8 initial_memory[ 0x10000 ], memory[ 0x10000 ];

uint8 Z80_RB_MACRO( uint16 address );
uint8 Z80_RB_MACRO( uint16 address );

void Z80_WB_MACRO( uint16 address, uint8 b );
void Z80_WB_MACRO_internal( uint16 address, uint8 b );

static int read_test_file( const char *filename,
			   uint32 *end_z80_tstates );

static void dump_z80_state( void );
static void dump_memory_state( void );

int
main( int argc, char **argv )
{
  size_t i;

  progname = argv[0];

  if( argc < 2 ) {
    fprintf( stderr, "Usage: %s <testfile>\n", progname );
    return 1;
  }

  if( init_dummies() ) return 1;

  /* Initialise the tables used by the Z80 core */
  z80_init();

  /* Get ourselves into a known state */
  z80_reset(); z80_tstates = 0;
  for( i = 0; i < 0x10000; i += 4 ) {
    memory[ i     ] = 0xde; memory[ i + 1 ] = 0xad;
    memory[ i + 2 ] = 0xbe; memory[ i + 3 ] = 0xef;
  }

  if( read_test_file( argv[1], &event_next_event ) ) return 1;

  /* Grab a copy of the memory for comparision at the end */
  memcpy( initial_memory, memory, 0x10000 );

  z80_do_opcodes();

  /* And dump our final state */
  dump_z80_state();
  dump_memory_state();

  return 0;
}

uint8
Z80_RB_MACRO( uint16 address )
{
  printf( "%5d MC %04x\n", z80_tstates, address );
  z80_tstates += 3;
  return Z80_RB_MACRO( address );
}

uint8
Z80_RB_MACRO( uint16 address )
{
  printf( "%5d MR %04x %02x\n", z80_tstates, address, memory[ address ] );
  return memory[ address ];
}

void
Z80_WB_MACRO( uint16 address, uint8 b )
{
  printf( "%5d MC %04x\n", z80_tstates, address );
  z80_tstates += 3;
  Z80_WB_MACRO_internal( address, b );
}

void
Z80_WB_MACRO_internal( uint16 address, uint8 b )
{
  printf( "%5d MW %04x %02x\n", z80_tstates, address, b );
  memory[ address ] = b;
}

void
ula_contend_port( uint16 port )
{
  printf( "%5d PC %04x\n", z80_tstates, port );
  z80_tstates += 4;
}

void
contend_read( uint16 address, uint32 time )
{
  printf( "%5d MC %04x\n", z80_tstates, address );
  z80_tstates += time;
}

void
contend_write( uint16 address, uint32 time )
{
  printf( "%5d MC %04x\n", z80_tstates, address );
  z80_tstates += time;
}

uint8
Z80_RP_MACRO( uint16 port )
{
  /* For now, just return 0xff. May need to make this more complicated later */
  printf( "%5d PR %04x %02x\n", z80_tstates, port, 0xff );
  return 0xff;
}

void
Z80_WP_MACRO( uint16 port, uint8 b )
{
  /* Don't need to do anything here */
  printf( "%5d PW %04x %02x\n", z80_tstates, port, b );
}

static int
read_test_file( const char *filename, uint32 *end_z80_tstates )
{
  FILE *f;

  unsigned af, bc, de, hl, af_, bc_, de_, hl_, ix, iy, sp, pc;
  unsigned i, r, iff1, iff2, im;
  unsigned end_z80_tstates2;
  unsigned address;

  f = fopen( filename, "r" );
  if( !f ) {
    fprintf( stderr, "%s: couldn't open `%s': %s\n", progname, filename,
	     strerror( errno ) );
    return 1;
  }

  /* FIXME: how should we read/write our data types? */
  if( fscanf( f, "%x %x %x %x %x %x %x %x %x %x %x %x", &af, &bc,
	      &de, &hl, &af_, &bc_, &de_, &hl_, &ix, &iy, &sp, &pc ) != 12 ) {
    fprintf( stderr, "%s: first registers line in `%s' corrupt\n", progname,
	     filename );
    fclose( f );
    return 1;
  }

  AF  = af;  BC  = bc;  DE  = de;  HL  = hl;
  AF_ = af_; BC_ = bc_; DE_ = de_; HL_ = hl_;
  IX  = ix;  IY  = iy;  SP  = sp;  PC  = pc;

  if( fscanf( f, "%x %x %u %u %u %d %d", &i, &r, &iff1, &iff2, &im,
	      &z80.halted, &end_z80_tstates2 ) != 7 ) {
    fprintf( stderr, "%s: second registers line in `%s' corrupt\n", progname,
	     filename );
    fclose( f );
    return 1;
  }

  I = i; R = R7 = r; IFF1 = iff1; IFF2 = iff2; IM = im;
  *end_z80_tstates = end_z80_tstates2;

  while( 1 ) {

    if( fscanf( f, "%x", &address ) != 1 ) {

      if( feof( f ) ) break;

      fprintf( stderr, "%s: no address found in `%s'\n", progname, filename );
      fclose( f );
      return 1;
    }

    while( 1 ) {

      unsigned byte;

      if( fscanf( f, "%x", &byte ) != 1 ) {

	if( feof( f ) ) break;

	fprintf( stderr, "%s: no data byte found in `%s'\n", progname,
		 filename );
	fclose( f );
	return 1;
      }
    
      if( byte < 0 || byte > 255 ) break;

      memory[ address++ ] = byte;

    }
  }

  if( fclose( f ) ) {
    fprintf( stderr, "%s: couldn't close `%s': %s\n", progname, filename,
	     strerror( errno ) );
    return 1;
  }

  return 0;
}

static void
dump_z80_state( void )
{
  printf( "%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
	  AF, BC, DE, HL, AF_, BC_, DE_, HL_, IX, IY, SP, PC );
  printf( "%02x %02x %d %d %d %d %d\n", I, ( R7 & 0x80 ) | ( R & 0x7f ),
	  IFF1, IFF2, IM, z80.halted, z80_tstates );
}

static void
dump_memory_state( void )
{
  size_t i;

  for( i = 0; i < 0x10000; i++ ) {

    if( memory[ i ] == initial_memory[ i ] ) continue;

    printf( "%04x ", (unsigned)i );

    while( i < 0x10000 && memory[ i ] != initial_memory[ i ] )
      printf( "%02x ", memory[ i++ ] );

    printf( "-1\n" );
  }
}

/* Error 'handing': dump core as these should never be called */

int
fuse_abort( void )
{
  abort();
}

int
ui_error( ui_error_level severity, const char *format, ... )
{
  va_list ap;

  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );

  abort();
}

/*
 * Stuff below here not interesting: dummy functions and variables to replace
 * things used by Fuse, but not by the core test code
 */

#include "debugger/debugger.h"
#include "machine.h"
#include "scld.h"
#include "settings.h"

uint8 *slt[256];
size_t slt_length[256];

int
tape_load_trap( void )
{
  /* Should never be called */
  abort();
}

int
tape_save_trap( void )
{
  /* Should never be called */
  abort();
}

scld scld_last_dec;

size_t rzx_instruction_count;
int rzx_playback;
int rzx_instructions_offset;

enum debugger_mode_t debugger_mode;

uint8 **ROM = NULL;
memory_page memory_map[8];
memory_page *memory_map_home[8];
memory_page memory_map_rom[8];
int memory_contended[8] = { 1 };
uint8 spectrum_contention[ 80000 ] = { 0 };

int
debugger_check( debugger_breakpoint_type type, uint32 value )
{
  /* Should never be called */
  abort();
}

int
debugger_trap( void )
{
  /* Should never be called */
  abort();
}

int
slt_trap( uint16 address, uint8 level )
{
  return 0;
}

int trdos_available = 0;
int trdos_active = 0;

void
trdos_page( void )
{
  abort();
}

void
trdos_unpage( void )
{
  abort();
}

int
event_add( uint32 event_time, int type )
{
  /* Do nothing */
  return 0;
}

fuse_machine_info *machine_current;
static fuse_machine_info dummy_machine;

const int LIBSPECTRUM_MACHINE_CAPABILITY_EVEN_M1 = 1;

settings_info settings_current;

/* Initialise the dummy variables such that we're running on a clean a
   machine as possible */
static int
init_dummies( void )
{
  size_t i;

  for( i = 0; i < 8; i++ ) {
    memory_map[i].page = &memory[ i * MEMORY_PAGE_SIZE ];
  }

  debugger_mode = DEBUGGER_MODE_INACTIVE;
  dummy_machine.capabilities = 0;
  dummy_machine.ram.current_rom = 0;
  machine_current = &dummy_machine;
  rzx_playback = 0;
  scld_last_dec.name.intdisable = 0;
  settings_current.slt_traps = 0;

  return 0;
}
