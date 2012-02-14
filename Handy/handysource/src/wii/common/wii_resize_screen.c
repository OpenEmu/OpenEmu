/*
Copyright (C) 2010
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

#include "wii_app.h"
#include "wii_gx.h"
#include "wii_hw_buttons.h"
#include "wii_input.h"
#include "wii_resize_screen.h"
#include "wii_sdl.h"

extern void WII_SetRenderCallback( void (*cb)(void) );
extern void WII_ChangeSquare(int xscale, int yscale, int xshift, int yshift);
extern void WII_VideoStart();
extern void WII_VideoStop();

// Whether the aspect ratio is locked
static BOOL arlocked = TRUE;
// The current aspect ratio
static float aratio = 0.0f;
// The x increment
static float xinc = 0.0f;
// The y increment
static float yinc = 0.0f;

#define DELAY_FRAMES 6
#define DELAY_STEP 1
#define DELAY_MIN 0

/*
 * Resets the aspect ratio
 *
 * currentX   The current X value
 * currentY   The current Y value
 */
static void reset_aspect_ratio( float currentX, float currentY )
{  
  if( arlocked )
  {
    aratio = currentX / currentY;
    xinc = aratio;
  }
  else
  {
    xinc = 1.0f;
  }

  yinc = 1.0f;  
}

extern Mtx gx_view;

/*
 * GX render callback
 */
void wii_resize_render_callback()
{
  GX_SetVtxDesc( GX_VA_POS, GX_DIRECT );
  GX_SetVtxDesc( GX_VA_CLR0, GX_DIRECT );
  GX_SetVtxDesc( GX_VA_TEX0, GX_NONE );

  Mtx m;      // model matrix.
  Mtx mv;     // modelview matrix.

  guMtxIdentity( m ); 
  guMtxTransApply( m, m, 0, 0, -100 );
  guMtxConcat( gx_view, m, mv );
  GX_LoadPosMtxImm( mv, GX_PNMTX0 ); 

  GXColor black = (GXColor) { 0x0, 0x0, 0x0, 0xff };

  wii_gx_drawrectangle( 
    -141, 81, 282, 162, black, FALSE );
  wii_gx_drawrectangle( 
    -140, 80, 280, 160, (GXColor){ 0x99, 0x99, 0x99, 0xff }, TRUE );    

  int fontsize = 18;
  int spacing = 20;
  int largespacing = 30;
  int y = 70;

  u16 right = ( FTGX_ALIGN_TOP | FTGX_JUSTIFY_RIGHT );
  u16 left = ( FTGX_ALIGN_TOP | FTGX_JUSTIFY_LEFT );

  wii_gx_drawtext( 0, y, fontsize, "D-pad/Analog :", black, right );
  wii_gx_drawtext( 0, y, fontsize, " Resize screen", black, left );
  y-=spacing;

  wii_gx_drawtext( 0, y, fontsize, "A/2 button :", black, right );
  wii_gx_drawtext( 0, y, fontsize, " Accept changes", black, left );
  y-=spacing;

  wii_gx_drawtext( 0, y, fontsize, "B/1 button :", black, right );
  wii_gx_drawtext( 0, y, fontsize, " Cancel changes", black, left );
  y-=largespacing;

  wii_gx_drawtext( 0, y, fontsize, "Minus/LTrigger :", black, right );
  wii_gx_drawtext( 0, y, fontsize, " Toggle A/R lock", black, left );
  y-=spacing;

  wii_gx_drawtext( 0, y, fontsize, "Plus/RTrigger :", black, right );
  wii_gx_drawtext( 0, y, fontsize, " Reset to defaults", black, left );
  y-=largespacing;

  wii_gx_drawtext( 
    0, y, fontsize, 
    ( arlocked ? "(Aspect ratio : Locked)" : "(Aspect ratio : Unlocked)" ), 
    black, FTGX_ALIGN_TOP | FTGX_JUSTIFY_CENTER  );
}

/*
 * Displays the resize user interface
 *
 * rinfo  Information for the resize operation
 */
void wii_resize_screen_gui( resize_info* rinfo )
{ 
  float currentX = rinfo->currentX;
  float currentY = rinfo->currentY;

  arlocked = TRUE;

  reset_aspect_ratio( currentX, currentY );

  WII_ChangeSquare( currentX, currentY, 0, 0 );
  WII_SetRenderCallback( &wii_resize_render_callback );  

  WII_VideoStart();   

  // Allows for incremental speed when scaling
  // (Scales faster the longer the directional pad is held)
  s16 delay_frames = -1;
  s16 delay_factor = -1;

  BOOL loop = TRUE;
  while( loop && !wii_hw_button )
  {
    WII_ChangeSquare(  currentX, currentY, 0, 0 );

    // Scan the Wii and Gamecube controllers
    WPAD_ScanPads();
    PAD_ScanPads();        

    // Check the state of the controllers
    u32 down = WPAD_ButtonsDown( 0 );
    u32 held = WPAD_ButtonsHeld( 0 );
    u32 gcDown = PAD_ButtonsDown( 0 );
    u32 gcHeld = PAD_ButtonsHeld( 0 );

    // Analog controls
    expansion_t exp;
    WPAD_Expansion( 0, &exp );        
    float expX = wii_exp_analog_val( &exp, TRUE, FALSE );
    float expY = wii_exp_analog_val( &exp, FALSE, FALSE );
    s8 gcX = PAD_StickX( 0 );
    s8 gcY = PAD_StickY( 0 );

    // Classic or Nunchuck?
    bool isClassic = ( exp.type == WPAD_EXP_CLASSIC );

    if( ( ( held & (
        WII_BUTTON_LEFT | WII_BUTTON_RIGHT | 
        WII_BUTTON_DOWN | WII_BUTTON_UP |
        ( isClassic ? 
          ( WII_CLASSIC_BUTTON_LEFT | WII_CLASSIC_BUTTON_UP ) : 0 ) 
            ) ) == 0 ) &&
      ( ( gcHeld & (
        GC_BUTTON_LEFT | GC_BUTTON_RIGHT |
        GC_BUTTON_DOWN | GC_BUTTON_UP ) ) == 0 ) &&
      ( !wii_analog_left( expX, gcX ) &&
        !wii_analog_right( expX, gcX ) &&
        !wii_analog_up( expY, gcY )&&
        !wii_analog_down( expY, gcY ) ) )
    {
      delay_frames = -1;
      delay_factor = -1;
    }
    else
    {
      if( delay_frames < 0 )
      {
        if( wii_digital_left( !wii_mote_menu_vertical, isClassic, held ) || 
          ( gcHeld & GC_BUTTON_LEFT ) ||                       
          wii_analog_left( expX, gcX ) )
        {
          // Left
          if( arlocked )
          {
            currentX -= xinc;
            currentY -= yinc;
          }
          else
          {
            currentX -= xinc;
          }
        }
        else if( 
          wii_digital_right( !wii_mote_menu_vertical, isClassic, held ) ||
          ( gcHeld & GC_BUTTON_RIGHT ) ||
          wii_analog_right( expX, gcX ) )
        {
          // Right
          if( arlocked )
          {
            currentX += xinc;
            currentY += yinc;
          }
          else
          {
             currentX += xinc;
          }
        }
        else if( 
          wii_digital_down( !wii_mote_menu_vertical, isClassic, held ) ||
          ( gcHeld & GC_BUTTON_DOWN ) ||
          wii_analog_down( expY, gcY ) )
        {
          // Down
          if( arlocked )
          {
            currentY += yinc;
            currentX += xinc;
          }
          else
          {
            currentY += yinc;
          }
        }
        else if( 
          wii_digital_up( !wii_mote_menu_vertical, isClassic, held ) || 
          ( gcHeld & GC_BUTTON_UP ) ||
          wii_analog_up( expY, gcY ) )
        {
          // Up
          if( arlocked )
          {
            currentY -= yinc;
            currentX -= xinc;
          }
          else
          {
            currentY -= yinc;
          }
        }

        delay_frames = 
          DELAY_FRAMES - (DELAY_STEP * ++delay_factor);

        if( delay_frames < DELAY_MIN ) 
        {
          delay_frames = DELAY_MIN;
        }
      }
      else
      {
        delay_frames--;
      }
    }

    if( currentY < 0 || currentX < 0 )
    {
      if( arlocked || currentX < 0 )
      {
        currentX += xinc;
      }
      if( arlocked || currentY < 0 )
      {
        currentY += yinc;
      }
    }

    // Confirm (A)
    if( ( down & ( WII_BUTTON_A | 
          ( isClassic ? WII_CLASSIC_BUTTON_A : WII_NUNCHUK_BUTTON_A ) ) ) ||
        ( gcDown & GC_BUTTON_A ) )
    {	
      rinfo->currentX = currentX;
      rinfo->currentY = currentY;
      loop = FALSE;
    }
    // Cancel (B or Home)
    if( ( down & ( WII_BUTTON_B | 
          ( isClassic ? WII_CLASSIC_BUTTON_B : WII_NUNCHUK_BUTTON_B ) |
          WII_BUTTON_HOME ) ) || 
        ( gcDown & ( GC_BUTTON_B | GC_BUTTON_HOME ) ) )
    {
      loop = FALSE;
    }    
    // Toggle aspect ratio (Plus or LTrigger/GC)
    if( ( down & ( WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS | 
          WPAD_CLASSIC_BUTTON_FULL_L) ) ||
        ( gcDown & PAD_TRIGGER_L ) )
    {
      arlocked = !arlocked;
      reset_aspect_ratio( currentX, currentY );
    }    
    // Reset to defaults (Minus or RTrigger/GC)
    if( ( down & ( WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS |
          WPAD_CLASSIC_BUTTON_FULL_R ) ) ||
        ( gcDown & PAD_TRIGGER_R ) )
    {
      currentX = rinfo->defaultX;
      currentY = rinfo->defaultY;
      reset_aspect_ratio( currentX, currentY );
    }    

    VIDEO_WaitVSync();
  } 

  WII_VideoStop();
  WII_SetRenderCallback( NULL );  
}

/*
 * Draws a border around the surface that is to be scaled.
 *
 * surface  The surface to scale
 * startY   The Y offset into the surface to scale
 * height   The height to scale
 */
void wii_resize_screen_draw_border( SDL_Surface* surface, int startY, int height )
{
  wii_sdl_draw_rectangle(
    surface, 0, startY, surface->w, height, 
    SDL_MapRGB( surface->format, 0xff, 0xff, 0xff ), FALSE ); 

  wii_sdl_draw_rectangle(
    surface, 1, 1 + startY, surface->w-2, height-2,
    SDL_MapRGB( surface->format, 0, 0, 0 ), FALSE ); 
}
