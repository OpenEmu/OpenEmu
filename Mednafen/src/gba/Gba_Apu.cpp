
// Gba_Snd_Emu 0.1.1. http://www.slack.net/~ant/nes-emu/

#include "Gba_Apu.h"

#include <stdio.h>
#include <string.h>

/* Library Copyright (C) 2003-2004 Shay Green. Gba_Snd_Emu is free
software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.
Gba_Snd_Emu is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details. You should have received a copy of the GNU General Public
License along with Gba_Snd_Emu; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

#ifndef GAME_SND_EMU_DEBUG
	#define GAME_SND_EMU_DEBUG 0
#endif

Gba_Apu::Gba_Apu() {
	square1.synth = &square_synth;
	square2.synth = &square_synth;
	square1.has_sweep = true;
	
	oscs [0] = &square1;
	oscs [1] = &square2;
	oscs [2] = &wave;
	oscs [3] = &noise;
	
	for(int x = 0; x < 4; x++)
		oscs[x]->setME(0x80);
	volume( 1.0 );
	reset();
}

void Gba_Apu::setgba(int s)
{
 int x;
 
 gbamode = s ? 1 : 0;

 for(x=0; x<4; x++)
 {
  oscs[x]->gbamode = s;
  oscs[x]->peradjust = s ? 2 : 0;
 }
}

Gba_Apu::~Gba_Apu() {
}

void Gba_Apu::treble_eq( const blip_eq_t& eq ) {
	square_synth.treble_eq( eq );
	wave.synth.treble_eq( eq );
	noise.synth.treble_eq( eq );
}

void Gba_Apu::volume( double vol ) {
	vol *= 0.60 / osc_count;
	square_synth.volume( vol );
	wave.synth.volume( vol );
	noise.synth.volume( vol );
}

void Gba_Apu::output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right ) {
	for ( int i = 0; i < osc_count; i++ )
		osc_output( i, center, left, right );
}

void Gba_Apu::reset() {
	next_frame_time = 0;
	last_time = 0;
	frame_count = 0;
	stereo_found = false;
	
	square1.reset();
	square2.reset();
	wave.reset();
	noise.reset();
	
	memset( regs, 0, sizeof regs );
}

void Gba_Apu::osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	assert(0 <= index && index < osc_count );
	
	Gba_Osc& osc = *oscs [index];
	if ( center && !left && !right ) {
		// mono
		left = center;
		right = center;
	}
	else {
		// must be silenced or stereo
		assert( (!left && !right) || (left && right) );
	}
	osc.outputs [1] = right;
	osc.outputs [2] = left;
	osc.outputs [3] = center;
	osc.output = osc.outputs [osc.output_select];
}

static FILE* log_file;

static int log_time( int t ) {
	if ( !GAME_SND_EMU_DEBUG )
		return 0;
	static gb_time_t last_time;
	if ( last_time > t ) {
		if ( log_file ) {
			static int counter;
			fprintf( log_file, "\n%d -- ", ++counter );
		}
		last_time = 0;
	}
	int diff = t - last_time;
	last_time = t;
	return diff;
}

void Gba_Apu::begin_debug_log() {
	if ( GAME_SND_EMU_DEBUG && !log_file )
		log_file = fopen( "log", "w" );
}

void Gba_Apu::run_until( gb_time_t end_time )
{
	if ( end_time == last_time )
		return;
	assert(last_time < end_time);
	
	while ( true )
	{
		gb_time_t time = next_frame_time;
		if ( time > end_time )
			time = end_time;
		
		// run oscillators
		for ( int i = 0; i < osc_count; ++i ) {
			Gba_Osc& osc = *oscs [i];
			if ( osc.output ) {
				if ( osc.output != osc.outputs [3] )
					stereo_found = true;
				osc.run( last_time, time );
			}
		}
		last_time = time;
		
		if ( time == end_time )
			break;
		
		if ( GAME_SND_EMU_DEBUG && log_file )
			fprintf( log_file, "%d\n\t\t\t\tframe %d  ", log_time( time ), frame_count );
		
		next_frame_time += (4194304 << (gbamode << 1)) / 256; // 256 Hz
		
		// 256 Hz actions
		square1.clock_length();
		square2.clock_length();
		wave.clock_length();
		noise.clock_length();
		
		frame_count = (frame_count + 1) & 3;
		if ( frame_count == 0 ) {
			// 64 Hz actions
			square1.clock_envelope();
			square2.clock_envelope();
			noise.clock_envelope();
			//printf("%d\n", square1.volume);
		}
		
		if ( frame_count & 1 )
		{
			square1.clock_sweep(); // 128 Hz action
		}
	}
}

bool Gba_Apu::end_frame( gb_time_t end_time )
{
	run_until( end_time );
	
	next_frame_time -= end_time;
	assert( next_frame_time >= 0 );
	last_time = 0;
	
	bool result = stereo_found;
	stereo_found = false;
	return result;
}

void Gba_Apu::write_register( gb_time_t time, gb_addr_t addr, int data )
{
	assert( addr > 0x30 );
	assert(0 <= data && data <= 0xFF);
	
	int reg = addr - start_addr;
	if ( unsigned (reg) > end_addr - start_addr )
		return;
	
	run_until( time );
	
	if ( GAME_SND_EMU_DEBUG && log_file ) {
		int index = reg / 5;
		if ( index >= osc_count || oscs [index]->outputs [3] )
			fprintf( log_file, "%d\n%.2X %.2X  ", log_time( time ), reg + 0xff10, data );
	}
	
	regs [reg] = data;
	
	if ( addr < 0xff24 ) {
		// oscillator
		int index = reg / 5;
		oscs [index]->write_register( reg - index * 5, data ); 
	}
	//else if( addr == 0xff24)
	//{
	// printf("%02x\n", data);
	//}
	else if ( addr == 0xff25 ) {
		// left/right assignments
		for ( int i = 0; i < osc_count; i++ ) {
			Gba_Osc& osc = *oscs [i];
			int flags = data >> i;
			Blip_Buffer* old_output = osc.output;
			osc.output_select = ((flags >> 3) & 2) | (flags & 1);
			osc.output = osc.outputs [osc.output_select];
			if ( osc.output != old_output && osc.last_amp ) {
				if ( old_output )
					square_synth.offset( time, -osc.last_amp, old_output );
				osc.last_amp = 0;
			}
		}
	}
	else if ( addr >= 0xff30 ) {
		// separate samples now (simplifies oscillator)
		//printf("Write: %04x %02x, %d\n",addr,data, wave.ws_offset);
		int index = ((((addr & 0x0f) + (wave.ws_offset ^ 0x10)) * 2) % wave.ws_mask); //% ws_mask) * 2;
		wave.wave [index] = data >> 4;
		wave.wave [index + 1] = data & 0x0f;
	}
	else if (addr == 0xff26)
	{
		int x;
	        //printf("Oops: %04x %02x\n",addr,data);
		for(x=0;x<4;x++)
		 oscs[x]->setME(data & 0x80);
	}
	else
	{
	 //printf("Oops: %04x %02x\n",addr,data);
	}
}

void Gba_Apu::dirty(void)
{
	int data = regs[0xff25 - start_addr];

                for ( int i = 0; i < osc_count; i++ ) {
                        Gba_Osc& osc = *oscs [i];
                        int flags = data >> i;
                        osc.output_select = ((flags >> 3) & 2) | (flags & 1);
                        osc.output = osc.outputs [osc.output_select];
                }
}

int Gba_Apu::read_register( gb_time_t time, gb_addr_t addr )
{
	assert( addr > 0x30 );
	assert(start_addr <= addr && addr <= end_addr );
	
	run_until( time );
	
	int data = regs [addr - start_addr];
	
	if ( addr == 0xff26 ) { // status
		data &= 0xf0;
		for ( int i = 0; i < osc_count; i++ ) {
			const Gba_Osc& osc = *oscs [i];
			if ( osc.enabled && (osc.length || !osc.length_enabled) )
				data |= 1 << i;
		}
	}
	else if(addr >= 0xff30)
	{
                int index = ((addr & 0x0f) + wave.ws_offset) * 2;
		data = ((wave.wave [index] << 4) | wave.wave [index + 1]);
	}
	//printf("Read: %04x %02x\n", addr, data);
	return data;
}


