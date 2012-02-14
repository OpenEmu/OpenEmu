/*
WiiHandy : Port of the Handy Emulator for the Wii

Copyright (C) 2011
raz0red (www.twitchasylum.com)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.
*/

#include "system.h"

#include "wii_app.h"
#include "wii_gx.h"
#include "wii_main.h"
#include "wii_sdl.h"

#include "wii_handy.h"
#include "wii_handy_input.h"
#include "wii_handy_main.h"
#include "wii_handy_sdl.h"
#include "wii_handy_sound.h"
#include "wiierrorhandler.h"

#ifdef WII_NETTRACE
#include <network.h>
#include "net_print.h"  
#endif

extern "C" 
{
void WII_VideoStart();
void WII_VideoStop();
void WII_ChangeSquare(int xscale, int yscale, int xshift, int yshift);
void WII_SetRenderCallback( void (*cb)(void) );
Mtx gx_view;
}

// The count of sound buffer overlaps (Wii Audio)
extern int mixoverlap_count;
// The count of sound buffer overlaps (Handy)
int gAudioSkipCount = 0;

// The Handy instance
CSystem *mpLynx = NULL;
// The Handy buffer
Uint32 *mpLynxBuffer = NULL;

// Timer information
static u64 timer_currentTime;
static u64 timer_nextTime;
static u32 timer_frameTime;
static u64 timer_timerCount;
static u64 timer_timerCount2;
static u64 timer_startTime;
static float timer_fpsCounter = 0.0;
static float timer_fpsCounter2 = 0.0;

// Whether emulation is currently running
int emulationRunning = 0;

// Forward references
static void gxrender_callback();
static int handy_init_audio();
static UBYTE *handy_display_callback( ULONG objref );

/*
 * Initializes WiiHandy
 */
void wii_handy_init()
{
  gError = new CWiiErrorHandler();
  InitialiseAudio();
  gAudioEnabled = 1;
  gThrottleMaxPercentage = 100;
}

/*
 * Resets timing information
 */
static void handy_timer_reset( ) {
  timer_frameTime = (1000.0 / ((double)wii_get_max_frames() - 0.1)) * 1000;
  timer_startTime = timer_currentTime = ((u64)SDL_GetTicks()) * 1000;
  timer_timerCount = timer_timerCount2 = 0;
  timer_fpsCounter = timer_fpsCounter2 = 0.0;
  timer_nextTime = timer_currentTime + timer_frameTime;
}

/*
 * Plays current handy sounds from buffer
 */
static inline void handy_play_sound()
{
  if( gAudioBufferPointer != 0 && !gSystemHalt ) 
  {		
    PlaySound( gAudioBuffer, gAudioBufferPointer );
    memmove(gAudioBuffer, gAudioBuffer + gAudioBufferPointer, 0);
    gAudioBufferPointer = 0;
  }
}

/*
 * Update loop for Handy
 */
static inline int handy_update()
{
  // Throttling code
  //
  if(gSystemCycleCount>gThrottleNextCycleCheckpoint)
  {
    int overrun=gSystemCycleCount-gThrottleNextCycleCheckpoint;
    int nextstep=(((HANDY_SYSTEM_FREQ/HANDY_FPS)*gThrottleMaxPercentage)/100);

    // We've gone thru the checkpoint, so therefore the
    // we must have reached the next timer tick, if the
    // timer hasnt ticked then we've got here early. If
    // so then put the system to sleep by saying there
    // is no more idle work to be done in the idle loop
    if(gThrottleLastTimerCount==gTimerCount)
    {
      return 0;
    }

    //Set the next control point
    gThrottleNextCycleCheckpoint+=nextstep;

    // Set next timer checkpoint
    gThrottleLastTimerCount=gTimerCount;

    // Check if we've overstepped the speed limit
    if(overrun>nextstep)
    {
      // We've exceeded the next timepoint, going way too
      // fast (sprite drawing) s8o reschedule.
      return 0;
    }
  }

  return 1;
}

/*
 * The main WiiHandy emulator loop
 */
void wii_handy_emu_loop()
{
  handy_timer_reset();  

  // Whether the orientation is horizontal
  BOOL horiz = ( wii_handy_orientation() == MIKIE_NO_ROTATE );

  // Set the surface (horiz or vert)
  wii_handy_set_surface( horiz );

  wii_sdl_black_screen();
  
  mpLynx->DisplaySetAttributes( 
    wii_handy_orientation(), 
    MIKIE_PIXEL_FORMAT_16BPP_565, 
    (ULONG)blit_surface->pitch, 
    handy_display_callback, 
    (ULONG)mpLynxBuffer
  );

  WII_SetRenderCallback( &gxrender_callback );
  
  WII_ChangeSquare( 
    ( horiz ? wii_screen_x : wii_screen_x_vert ), 
    ( horiz ? wii_screen_y : wii_screen_y_vert ), 0, 0 );

  WII_VideoStart();

  // Are we going to wait or rely on VSYNC?
  int wait = ( !wii_vsync_enabled() || wii_get_max_frames() != 60 );

  emulationRunning = 1; // Set the emulation as running 
  while( emulationRunning )
  {
    // Update controls
    wii_handy_update_controls();

    // Update TimerCount
    gTimerCount++;    

    while( handy_update() )
    {
      if( !gSystemHalt )
      {
        for( ULONG loop=1024; loop; loop-- )
        {
          mpLynx->Update();
        }
      }
      else
      {
        gTimerCount++;
      }
    }

    // Play sound
    handy_play_sound(); 

    // Wait for VSync signal 
    wii_sync_video();

    if( wait )
    {
      do
      {
        timer_currentTime = ((u64)SDL_GetTicks()) * 1000;
      }
      while( timer_currentTime < timer_nextTime );
    }

    timer_nextTime += timer_frameTime;
    timer_timerCount++;
  }

  WII_VideoStop();
  StopAudio();
}

// The boot rom
static char boot_rom_path[WII_MAX_PATH] = "";

/*
 * Returns the boot rom path
 *
 * return   The boot rom path
 */
static char* get_boot_rom_path()
{
  if( boot_rom_path[0] == '\0' )
  {
    // Attempt to find the boot rom in the application data
    // directory.
    snprintf( 
      boot_rom_path, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), 
      WII_BOOT_ROM_FILE );

#ifdef WII_NETTRACE
    {
      char val[256];
      sprintf( val, "looking for boot rom: %s\n", boot_rom_path );
      net_print_string(__FILE__,__LINE__, val );  
    }
#endif

    FILE *fp = fopen( boot_rom_path, "r" );
    if( fp ) 
    {
#ifdef WII_NETTRACE
      net_print_string(__FILE__,__LINE__, "unable to find boot rom\n" );  
#endif
      fclose( fp );
    }
    else
    {
      // Attempt to find the boot rom in the application directory
      wii_get_app_relative( WII_BOOT_ROM_NAME, boot_rom_path );
    }

#ifdef WII_NETTRACE
    {
      char val[256];
      sprintf( val, "boot rom path set to: %s\n", boot_rom_path );
      net_print_string(__FILE__,__LINE__, val );  
    }
#endif
  }

  return boot_rom_path;
}

/*
 * Loads the specified ROM
 *
 * gamefile The ROM to load
 */
void wii_handy_load_rom( char* gamefile )
{
  gAudioSkipCount = 0;
  mixoverlap_count = 0;

  ResetAudio();

  if( mpLynx != NULL )
  {
    delete mpLynx;
    mpLynx = NULL;
  }

  mpLynx = new CSystem( gamefile, get_boot_rom_path() );

  Uint32 buffSize = blit_surface->w * blit_surface->h * sizeof(Uint32) * 4;
  if( mpLynxBuffer == NULL )
  {
    mpLynxBuffer = (Uint32*)malloc( buffSize );
  }
  memset( mpLynxBuffer, 0, buffSize ); 
}

/*
 * GX render callback
 */
static void gxrender_callback()
{
  GX_SetVtxDesc( GX_VA_POS, GX_DIRECT );
  GX_SetVtxDesc( GX_VA_CLR0, GX_DIRECT );
  GX_SetVtxDesc( GX_VA_TEX0, GX_NONE );

  Mtx m;    // model matrix.
  Mtx mv;   // modelview matrix.

  guMtxIdentity( m ); 
  guMtxTransApply( m, m, 0, 0, -100 );
  guMtxConcat( gx_view, m, mv );
  GX_LoadPosMtxImm( mv, GX_PNMTX0 ); 

  if( wii_debug )
  {    
    static char text[256] = "";

    int pixelSize = 14;
    int h = pixelSize;
    int padding = 2;
                           
    if( timer_timerCount % wii_get_max_frames() == 0 )
    {
      sprintf( text, "FPS: %0.2f (%0.2f), VS: %s, audio: %d %d %d, hash: %s", 
        timer_fpsCounter,
        timer_fpsCounter2, 
        ( wii_vsync_enabled() ? "on" : "off" ),
        gAudioBufferPointer, 
        gAudioSkipCount,
        mixoverlap_count,
        wii_cartridge_hash );

      if( timer_timerCount > ( wii_get_max_frames() * 5 ) )
      {
        u64 now = ((u64)SDL_GetTicks()) * 1000;
        timer_fpsCounter =
          (((float)timer_timerCount/(now-timer_startTime))*1000000.0),
        timer_fpsCounter2 =
          (((float)timer_timerCount2/(now-timer_startTime))*1000000.0), 
        timer_timerCount = timer_timerCount2 = 0;
        timer_startTime = now;
      }
    }

    GXColor color = (GXColor){0x0, 0x0, 0x0, 0x80};
    int w = wii_gx_gettextwidth( pixelSize, text );    
    int x = -310;
    int y = 196;

    wii_gx_drawrectangle( 
      x + -padding, 
      y + h + padding, 
      w + (padding<<1), 
      h + (padding<<1), 
      color, TRUE );

    wii_gx_drawtext( x, y, pixelSize, text, ftgxWhite, FTGX_ALIGN_BOTTOM ); 
  }
}

/*
 * Updates the display 
 */
UBYTE *handy_display_callback( ULONG objref )
{
  memcpy( 
    blit_surface->pixels, 
    mpLynxBuffer, 
    blit_surface->w * blit_surface->h * blit_surface->format->BytesPerPixel );

  wii_sdl_put_image_normal( 1 );
  wii_sdl_flip();

  timer_timerCount2++;

  return (UBYTE*)mpLynxBuffer;
}
