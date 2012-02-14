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

#include <stdio.h>
#include <sys/dir.h>

#include <wiiuse/wpad.h>
#include <gccore.h>
#include <SDL.h>

#include "pngu/pngu.h"
#ifdef WII_NETTRACE
#include <network.h>
#include "net_print.h"  
#endif

#ifdef WII_BIN2O
#include "about_png.h"
#endif

#include "wii_main.h"
#include "wii_app.h"
#include "wii_hw_buttons.h"
#include "wii_input.h"
#include "wii_file_io.h"
#include "wii_freetype.h"
#include "wii_video.h"

#define ABOUT_Y 20

#define MENU_STARTY     160
#define MENU_HEADERX    30
#define MENU_LINESIZE   20
#define MENU_PAGESIZE   11

// The vsync mode
int wii_vsync = -1;
// The last rom that was successfully executed
char *wii_last_rom = NULL;

// The average FPS from testing vsync 
static float test_fps = 0.0;
// Whether the Wii is PAL or NTSC
BOOL wii_is_pal = 0;
// Whether the wiimote (for the menu) is horizontal or vertical
BOOL wii_mote_menu_vertical = TRUE;
// Whether to return to the top menu after exiting a game
BOOL wii_top_menu_exit = TRUE;
// The stack containing the menus the user has navigated
TREENODEPTR wii_menu_stack[4];
// The head of the menu stack
s8 wii_menu_stack_head = -1;
// Two framebuffers (double buffer our menu display)
u32 *wii_xfb[2] = { NULL, NULL };
// The root of the menu
TREENODE *wii_menu_root;
// Whether to quite the menu loop
BOOL wii_menu_quit_loop = 0;
// Forces the menu to be redrawn
BOOL wii_menu_force_redraw = 1;
// The current menu selection
s16 menu_cur_idx = -1;
// Offset of the selection bar in the menu
int wii_menu_sel_offset = 0;
// The menu selection color
RGBA wii_menu_sel_color = { 0, 0, 0xC0, 0 };

// Buffer containing the about image
static u32* about_buff = NULL;
// Properties of the about image
static PNGUPROP about_props;
// The current frame buffer
static u32 cur_xfb = 0;
// The first item to display in the menu (paging, etc.)
static s16 menu_start_idx = 0;

// The main args
static int main_argc;
static char **main_argv;

// Forward refs
static void wii_free_node( TREENODE* node );

/*
 * Test to see if the machine is PAL or NTSC
 */
static void wii_test_pal()
{
  // TODO: Can we simply use the current TV mode to determine this?
  int start = SDL_GetTicks();
  int i;
  for( i = 0; i < 40; i++ )
  {
    VIDEO_WaitVSync();
  }
  int time = SDL_GetTicks() - start;

  test_fps = time / 40.0;

  wii_is_pal = test_fps > 17.5;

#ifdef WII_NETTRACE
  char val[256];
  sprintf( val, "pal test: [%f>17.5]=%d\n", test_fps, wii_is_pal );
  net_print_string(__FILE__,__LINE__, val );  
#endif

}

/*
 * Sets whether to enable VSYNC or not 
 *
 * param    sync Whether to enable VSYNC or not
 */
void wii_set_vsync( BOOL sync )
{
  wii_vsync = ( sync ? VSYNC_ENABLED : VSYNC_DISABLED );
}

/*
 * Returns the current menu index
 *
 * return   The current menu index
 */
s16 wii_menu_get_current_index()
{
  return menu_cur_idx;
}

/*
 * Resets the menu indexes when an underlying menu in change (push/pop)
 */
void wii_menu_reset_indexes()
{
  menu_cur_idx = -1;
  menu_start_idx = 0;
}

/*
 * Creates and returns a new menu tree node
 *
 * type     The type of the node
 * name     The name for the node
 * return   The newly created node
 */
TREENODE* wii_create_tree_node( enum NODETYPE type, const char *name )
{
  TREENODE* nodep = (TREENODE*)malloc( sizeof( TREENODE ) );
  memset( nodep, 0, sizeof( TREENODE ) );
  nodep->node_type = type;
  nodep->name = strdup( name );
  nodep->child_count = 0;
  nodep->max_children = 0;
  nodep->x = nodep->value_x = -1;

  return nodep;
}

/*
 * Adds the specified child node to the specified parent
 *
 * parent   The parent
 * child    The child to add to the parent
 */
void wii_add_child( TREENODE *parent, TREENODE *childp  )
{
  // Do we have room?
  if( parent->child_count == parent->max_children )
  {
    parent->max_children += 10;
    parent->children = (TREENODEPTR*)
      realloc( parent->children, parent->max_children * 
        sizeof(TREENODEPTR));		
  }

  parent->children[parent->child_count++] = childp;
}

/*
 * Clears the children for the specified node
 *
 * node     The node to clear the children for
 */
void wii_menu_clear_children( TREENODE* node )
{
  int i;
  for( i = 0; i < node->child_count; i++ )
  {
    wii_free_node( node->children[i] );
    node->children[i] = NULL;
  }
  node->child_count = 0;
}

/*
 * Frees the specified tree node
 *
 * node     The node to free
 */
static void wii_free_node( TREENODE* node )
{
  wii_menu_clear_children( node );    

  free( node->children );
  free( node->name );	
  free( node );
}

/*
 * Pushes the specified menu onto the menu stack (occurs when the user
 * navigates to a sub-menu)
 *
 * menu     The menu to push on the stack
 */
void wii_menu_push( TREENODE *menu )
{    
  wii_menu_stack[++wii_menu_stack_head] = menu;

  wii_menu_reset_indexes();
  wii_menu_move( menu, 1 );
}

/*
 * Pops the latest menu off of the menu stack (occurs when the user leaves 
 * the current menu.
 *
 * return   The pop'd menu
 */
TREENODE* wii_menu_pop()
{    
  if( wii_menu_stack_head > 0 )
  {
    TREENODE *oldmenu = wii_menu_stack[wii_menu_stack_head--];
    wii_menu_reset_indexes();        
    wii_menu_move( wii_menu_stack[wii_menu_stack_head], 1 );
    return oldmenu;
  }

  return NULL;
}

/*
 * Updates the buffer with the header message for the current menu
 *
 * menu     The menu
 * buffer   The buffer to update with the header message for the
 *          current menu.
 */
static void wii_menu_get_header( TREENODE* menu, char *buffer )
{
  wii_menu_handle_get_header( menu, buffer );

  if( strlen( buffer ) == 0 )
  {
    snprintf( buffer, WII_MENU_BUFF_SIZE, 
      "U/D = Scroll%s, A = Select%s, Home = Exit", 
      ( menu->child_count > MENU_PAGESIZE ? ", L/R = Page" : "" ),
      ( wii_menu_stack_head > 0 ? ", B = Back" : "" )            
      );
  }
}

/*
 * Returns the number of visible children in the specified menu
 *
 * menu     The menu
 * return   The number of visible children in the specified menu
 */
static int get_visible_child_count( TREENODE* menu )
{
  int count = 0;
  int i;
  for( i = 0; i < menu->child_count; i++ )
  {
    TREENODE* node = menu->children[i];    
    if( wii_menu_handle_is_node_visible( node ) )
    {
      ++count;
    }
  }
  return count;
}

/*
 * Updates the buffer with the footer message for the current menu
 *
 * menu     The menu
 * buffer   The buffer to update with the footer message for the
 *          current menu.
 */
static void wii_menu_get_footer( TREENODE* menu, char *buffer )
{    
  buffer[0] = '\0';

  // Any status messages? If so display it in the footer
  if( wii_status_message_count > 0 )
  {        
    wii_status_message_count--;
    snprintf( buffer, WII_MENU_BUFF_SIZE, wii_status_message );
  }
  else
  {
    wii_menu_handle_get_footer( menu, buffer );        

    if( buffer[0] == '\0' )
    {
      int visible = get_visible_child_count( menu );
      if( visible > MENU_PAGESIZE )
      {
        s16 end_idx = menu_start_idx + MENU_PAGESIZE;
        if( end_idx > visible ) end_idx = visible;
        s16 start_idx = ( end_idx - MENU_PAGESIZE ) + 1;
        if( start_idx <= 0 ) start_idx = 1;
        
        snprintf( buffer, WII_MENU_BUFF_SIZE, 
          "%d items found, displaying %d to %d.",
          visible, start_idx, end_idx );                
      }
    }          
  }
}

/*
 * Determines whether the node is selectable
 *
 * node     The node
 * return   Whether the node is selectable
 */
static BOOL wii_menu_is_node_selectable( TREENODE *node )
{
  if( !wii_menu_handle_is_node_visible( node ) )
  {
    return FALSE;
  }

  if( node->node_type == NODETYPE_SPACER )
  {
    return FALSE;      
  }

  return wii_menu_handle_is_node_selectable( node );
}

/*
 * Renders the specified menu
 */
static void wii_menu_render( TREENODE *menu )
{	
  // Swap buffers
  cur_xfb ^= 1;
  u32 *fb = wii_xfb[cur_xfb];

  VIDEO_ClearFrameBuffer( vmode, fb, COLOR_BLACK );

  // Bind the console to the appropriate frame buffer
#if 0
  wii_console_init( fb );
#endif

  // Render the about image (header)
  if( about_buff != NULL )
  {
    wii_video_draw_image( 
      &about_props, 
      about_buff, 
      fb, 
      ( vmode->fbWidth - about_props.imgWidth ) >> 1,  
      ABOUT_Y );
  }

  // Draw the menu items (text)
  if( menu )
  {	
    char buffer[WII_MENU_BUFF_SIZE];
    char value[WII_MENU_BUFF_SIZE];
    char buffer2[WII_MENU_BUFF_SIZE];

    buffer[0] = '\0';
    wii_menu_get_header( menu, buffer );

#if 0
    snprintf( buffer2, sizeof(buffer2), "\x1b[%d;%dH %s", 
      MENU_HEAD_Y, MENU_HEAD_X, buffer );		
    wii_write_vt( buffer2 );
#endif

    wii_ft_set_fontcolor( 96, 96, 96 );
    wii_ft_drawtext( fb, -1, MENU_STARTY, buffer );    
    wii_ft_set_fontcolor( 255, 255, 255 );

    int displayed = 0; 
    int i;
    for( i = menu_start_idx; 
      i < menu->child_count && displayed < MENU_PAGESIZE; 
      i++ )
    {		
      buffer[0] = '\0';
      value[0] = '\0';

      TREENODE* node = menu->children[i];
      if( wii_menu_handle_is_node_visible( node ) )
      {
        wii_menu_handle_get_node_name( node, buffer, value );

#if 0
        snprintf( buffer2, sizeof(buffer2), "\x1b[%d;%dH%s %s %s", 				
          MENU_START_Y + displayed, 
          MENU_START_X, 
          ( menu_cur_idx == i ? "\x1b[41m\x1b[37m" : "" ),
          buffer,
          ( menu_cur_idx == i ? "\x1b[40m\x1b[37m" : "" )              
          );

        wii_write_vt( buffer2 );
#endif
        if( menu_cur_idx == i )
        {
          int j;
          for( j = 0; j < 20; j++ )
          {
            wii_video_draw_line( 
              fb, 30, 610, 
              MENU_STARTY + ( ( displayed + 1 ) * MENU_LINESIZE ) + j + 5 + wii_menu_sel_offset,
              wii_menu_sel_color.R, wii_menu_sel_color.G, wii_menu_sel_color.B );
          }
        }

        wii_ft_drawtext( fb, node->x, 
          MENU_STARTY + ( ( displayed + 2 ) * MENU_LINESIZE ), 
          buffer );

        if( value[0] != '\0' )
        {
          sprintf( buffer2, ": %s", value );
          
          wii_ft_drawtext( fb, node->value_x, 
            MENU_STARTY + ( ( displayed + 2 ) * MENU_LINESIZE ), 
            buffer2 );
        }

        ++displayed;
      }
    }

    buffer[0] = '\0';
    wii_menu_get_footer( menu, buffer );

#if 0
    snprintf( buffer2, sizeof(buffer2), "\x1b[%d;%dH %s", 
      MENU_FOOT_Y, MENU_HEAD_X, buffer );		
    wii_write_vt( buffer2 );
#endif

    wii_ft_set_fontcolor( 96, 96, 96 );
    wii_ft_drawtext( fb, 
      -1,  
      MENU_STARTY + ( ( MENU_PAGESIZE + 3 ) * MENU_LINESIZE ), 
      buffer );    
  }

  VIDEO_SetNextFramebuffer( fb );
  VIDEO_SetBlack(FALSE);
  VIDEO_Flush();	
}

/*
 * Returns the index to the start of the page from the currently selected
 * index.
 *
 * return   The index to the start of the page from the currently selected
 *          index.
 */
static s16 get_page_start_idx( TREENODE *menu )
{
  int count = MENU_PAGESIZE - 1;
  s16 index = menu_cur_idx;
  while( count > 0 )
  {
    if( --index < 0 ) return 0;

    TREENODE* node = menu->children[index];
    if( wii_menu_handle_is_node_visible( node ) )
    {
      count--;
    }
  }

  return index;
}

/*
 * Move the current selection in the menu by the specified step count
 *
 * menu     The current menu
 * steps    The number of steps to move the selection
 */
void wii_menu_move( TREENODE *menu, s16 steps )
{	
  if( !menu ) return;

  s16 new_idx = menu_cur_idx;

  //
  // Attempt to move to the new location
  //

  while( 1 )
  {
    s16 prev_idx = new_idx;

    //
    // Move the requested number of steps, skipping items that
    // are not visible
    //
    int stepCount = steps < 0 ? -steps : steps;
    int stepSize = steps < 0 ? -1 : 1;
    while( stepCount > 0 )
    { 
      new_idx += stepSize;

      if( ( new_idx >= menu->child_count ) ||
          ( new_idx < 0 ) )
      {
        break;
      }

      TREENODE* curchild = menu->children[ new_idx ];
      if( wii_menu_handle_is_node_visible( curchild ) )
      {
        stepCount -= 1;
      }
    }    

    //
    // Handle edges
    //
    if( new_idx >= menu->child_count )
    {
      new_idx = menu->child_count - 1;
    }		

    if( new_idx < 0 )
    {
      new_idx = 0;
    }

    //
    // Make sure the item we ended up on can be selected
    //
    if( new_idx < menu->child_count )
    {
      TREENODE* curchild = menu->children[new_idx];
      if( ( !wii_menu_is_node_selectable( curchild ) ) &&
        prev_idx != new_idx )
      {
        steps = steps > 0 ? 1 : -1;
      }
      else
      {
        break;
      } 
    }
    else
    {
      break;
    }
  }

  //
  // Is the new location valid? If so make updates.
  //
  if( new_idx >= 0 && 
    new_idx < menu->child_count &&
    wii_menu_is_node_selectable( menu->children[new_idx] ) )
  {
    menu_cur_idx = new_idx;

    if( menu_cur_idx < menu_start_idx )
    {
      menu_start_idx = menu_cur_idx;
    }
    else
    {
      s16 min_start = get_page_start_idx( menu );
      if( min_start > menu_start_idx )
      {
        menu_start_idx = min_start;
      }
    }
  }
}

#define DELAY_FRAMES 6
#define DELAY_STEP 1
#define DELAY_MIN 0

/*
 * Displays the menu 
 */
void wii_menu_show()
{
  // Allows for incremental speed when scrolling the menu 
  // (Scrolls faster the longer the directional pad is held)
  s16 delay_frames = -1;
  s16 delay_factor = -1;

  // Invoke the menu pre loop handler
  wii_menu_handle_pre_loop();

  wii_menu_quit_loop = 0;

  while( !wii_menu_quit_loop )
  {		
    // Scan the Wii and Gamecube controllers
    WPAD_ScanPads();
    PAD_ScanPads();        

    // Check the state of the controllers
    u32 down = WPAD_ButtonsDown( 0 );
    u32 held = WPAD_ButtonsHeld( 0 );
    u32 gcDown = PAD_ButtonsDown( 0 );
    u32 gcHeld = PAD_ButtonsHeld( 0 );

    if( ( down & WII_BUTTON_HOME ) ||
      ( gcDown & GC_BUTTON_HOME ) || 
      wii_hw_button )
    {
      // Handle the home button being pressed
      wii_menu_handle_home_button();

      wii_menu_quit_loop = 1;
      continue;
    }

    // Analog controls
    expansion_t exp;
    WPAD_Expansion( 0, &exp );        
    float expX = wii_exp_analog_val( &exp, TRUE, FALSE );
    float expY = wii_exp_analog_val( &exp, FALSE, FALSE );
    s8 gcX = PAD_StickX( 0 );
    s8 gcY = PAD_StickY( 0 );

    // Classic or Nunchuck?
    bool isClassic = ( exp.type == WPAD_EXP_CLASSIC );

    TREENODE *menu = 
      wii_menu_stack_head >= 0 ?			
      wii_menu_stack[wii_menu_stack_head] : NULL;

    if( wii_menu_force_redraw )
    {
      wii_menu_force_redraw = 0;
      wii_menu_render( menu );
    }

    if( menu )
    {                    
      wii_menu_handle_update( menu );

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
          if( 
            wii_digital_left( !wii_mote_menu_vertical, isClassic, held ) || 
            ( gcHeld & GC_BUTTON_LEFT ) ||                       
            wii_analog_left( expX, gcX ) )
          {
            wii_menu_move( menu, -MENU_PAGESIZE );
            wii_menu_force_redraw = 1;
          }
          else if( 
            wii_digital_right( !wii_mote_menu_vertical, isClassic, held ) ||
            ( gcHeld & GC_BUTTON_RIGHT ) ||
            wii_analog_right( expX, gcX ) )
          {
            wii_menu_move( menu, MENU_PAGESIZE );
            wii_menu_force_redraw = 1;
          }
          else if( 
            wii_digital_down( !wii_mote_menu_vertical, isClassic, held ) ||
            ( gcHeld & GC_BUTTON_DOWN ) ||
            wii_analog_down( expY, gcY ) )
          {
            wii_menu_move( menu, 1 );
            wii_menu_force_redraw = 1;
          }
          else if( 
            wii_digital_up( !wii_mote_menu_vertical, isClassic, held ) || 
            ( gcHeld & GC_BUTTON_UP ) ||
            wii_analog_up( expY, gcY ) )
          {
            if( menu )
            {
              wii_menu_move( menu, -1 );						
              wii_menu_force_redraw = 1;
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

      if( ( down & ( WII_BUTTON_A | 
            ( isClassic ? WII_CLASSIC_BUTTON_A : WII_NUNCHUK_BUTTON_A ) ) ) ||
          ( gcDown & GC_BUTTON_A ) )
      {	
        wii_menu_handle_select_node( menu->children[menu_cur_idx] );
        wii_menu_force_redraw = 1;            
      }
      if( ( down & ( WII_BUTTON_B | 
            ( isClassic ? WII_CLASSIC_BUTTON_B : WII_NUNCHUK_BUTTON_B ) ) ) || 
          ( gcDown & GC_BUTTON_B ) )
      {
        wii_menu_pop();
        wii_menu_force_redraw = 1;
      }
    }

    VIDEO_WaitVSync();
  }

  // Invoke post loop handler
  wii_menu_handle_post_loop();
}

/*
 * Used for comparing menu names when sorting (qsort)
 *
 * a        The first tree node to compare
 * b        The second tree node to compare
 * return   The result of the comparison
 */
int wii_menu_name_compare( const void *a, const void *b )
{
  TREENODE** aptr = (TREENODE**)a;
  TREENODE** bptr = (TREENODE**)b;

  return stricmp( (*aptr)->name, (*bptr)->name );
}

/*
 * Returns the standard list footer for snapshots, games, etc.
 *
 * menu     The current menu
 * listname The name of the list (snapshot/game, etc.)
 * buffer   The buffer to write the footer to
 */
void wii_get_list_footer( 
  TREENODE* menu, const char *listname, char *buffer )
{
  if( menu->child_count == 0 )
  {
    snprintf( buffer, WII_MENU_BUFF_SIZE, "No %ss found.", listname );              
  }
  else if( menu->child_count == 1 )
  {
    snprintf( buffer, WII_MENU_BUFF_SIZE, "1 %s found.", listname );                
  }
  else
  {
    s16 end_idx = menu_start_idx + MENU_PAGESIZE;
    snprintf( buffer, WII_MENU_BUFF_SIZE, 
      "%d %ss found, displaying %d to %d.",
      menu->child_count,
      listname,
      menu_start_idx + 1,
      ( end_idx < menu->child_count ? end_idx : menu->child_count )                            
      );                
  }
}

/*
 * Loads the resource for the menu
 */
static void init_app()
{
  // Load the about image (header)
#ifdef WII_BIN2O
  IMGCTX about_ctx = PNGU_SelectImageFromBuffer( about_png );	
#else
  char about_loc[WII_MAX_PATH];
  wii_get_app_relative( "gfx/about.png", about_loc );
  IMGCTX about_ctx = PNGU_SelectImageFromDevice( about_loc );	
#endif

  if( about_ctx != NULL )
  {
    if( PNGU_GetImageProperties( about_ctx, &about_props ) != PNGU_OK )
    {
      PNGU_ReleaseImageContext( about_ctx );
      about_ctx = NULL;
    }
    else
    {
      u32 about_buff_size = 
        about_props.imgWidth * about_props.imgHeight * 2;
      about_buff = (u32*)malloc( about_buff_size );

      PNGU_DecodeToYCbYCr(
        about_ctx, 
        about_props.imgWidth, 
        about_props.imgHeight, 
        about_buff,
        0
      );

      PNGU_ReleaseImageContext( about_ctx );
      about_ctx = NULL;
    }
  }

  // Initialize the freetype library
  wii_ft_init();

  // Initialize the application
  wii_handle_init();
}

/*
 * Frees the menu resources 
 */
static void free_resources()
{
  //
  // Probably completely unnecessary but seems like a good time
  //

  if( about_buff != NULL )
  {
    free( about_buff );
    about_buff = NULL;
  }

  if( wii_menu_root != NULL )
  {
    wii_free_node( wii_menu_root );
    wii_menu_root = NULL;
  }

  if( wii_last_rom != NULL )
  {
    free( wii_last_rom );
    wii_last_rom = NULL;
  }
  
  // Free application resources
  wii_handle_free_resources();
}

/*
 * Flush and syncs the video 
 */
void wii_sync_video()
{     
  if( wii_vsync_enabled() )
  {
    VIDEO_WaitVSync();
  }
}

/*
 * Returns whether VSYNC is enabled
 *
 * return Whether VSYNC is enabled
 */
int wii_vsync_enabled()
{
  return 
    ( wii_vsync == VSYNC_ENABLED ) && 
    ( ( !wii_is_pal && wii_get_max_frames() <= 60 ) ||
      ( wii_is_pal && wii_get_max_frames() <= 50 ) );
}

/*
 * Main 
 */
int main(int argc,char *argv[])
{
#ifdef WII_NETTRACE
  char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};	
	if_config ( localip, netmask, gateway, TRUE);
  
  // First arg represents IP address for remote tracing
  net_print_init( ( argc > 1 ? argv[1] : NULL ), 0 );

  int i;
  for( i = 0; i < argc; i++ )
  {
    char val[256];
    sprintf( val, "arg[%d]: %s\n", i, argv[i] );
    net_print_string(__FILE__,__LINE__, val );
  }
#endif

  printf( "\x1b[5;0H" );

  main_argc = argc;
  main_argv = argv;

  // Initialize the Wii
  wii_set_app_path( argc, argv );  

  // Try to mount the file system
  if( !wii_mount() ) 
  {
    printf( "Unable to mount %s\n\n", wii_get_fs_prefix() );
    printf( "Press A to exit..." );
    wii_pause();
    exit( 0 ); // unable to mount file system
  }

  WPAD_Init();
  PAD_Init();

  // Set the hardware callbacks
  wii_register_hw_buttons();

  // Clear the stack
  memset( &wii_menu_stack, 0, sizeof(wii_menu_stack) );

  // Initializes the application
  init_app(); 

  // Allocate frame buffers
  wii_xfb[0] = (u32*)MEM_K0_TO_K1( SYS_AllocateFramebuffer( vmode ) );
  wii_xfb[1] = (u32*)MEM_K0_TO_K1( SYS_AllocateFramebuffer( vmode ) );	

  // Test for PAL/NTSC
  wii_test_pal();
  
  if( wii_vsync == -1 )
  {
    // Set the vsync based on whether or not we are PAL or NTSC
    wii_set_vsync( !wii_is_pal );
  }

  // Runs the application
  wii_handle_run();

  // Frees the application resources
  free_resources();    

#ifdef WII_NETTRACE
  net_print_close();
#endif

  if( wii_hw_button )
  {
    // They pressed a HW button, reset based on its type
    SYS_ResetSystem( wii_hw_button, 0, 0 );
  }
  else if( !!*(u32*)0x80001800 ) 
  {
    // Were we launched via HBC?
    exit(1);
  }
  else
  {
    // Wii channel support
    SYS_ResetSystem( SYS_RETURNTOMENU, 0, 0 );
  }

  return 0;
}
