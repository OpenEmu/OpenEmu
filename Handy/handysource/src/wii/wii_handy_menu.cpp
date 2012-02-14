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

#include <stdio.h>
#include <stdlib.h>

#include "system.h"

#include "wii_app.h"
#include "wii_resize_screen.h"
#include "wii_sdl.h"
#include "wii_snapshot.h"
#include "wii_util.h"

#include "wii_handy.h"
#include "wii_handy_emulation.h"
#include "wii_handy_sdl.h"
#include "wii_handy_snapshot.h"

// Have we read the games list yet?
static BOOL games_read = FALSE;
// Have we read the save state list yet?
static BOOL save_states_read = FALSE;
// The index of the last rom that was run
static s16 last_rom_index = 1;

// Forward refs
static void wii_read_save_state_list( TREENODE *menu );
static void wii_read_game_list( TREENODE *menu );

/*
 * Saves the current games state to the specified save file
 *
 * savefile The name of the save file to write state to. If this value is NULL,
 *          the default save name for the last rom is used.
 */
static void save_snapshot( const char *savefile, BOOL status_update )
{
  wii_save_snapshot( savefile, status_update );
}

/*
 * Initializes the menu
 */
void wii_handy_menu_init()
{  
  //
  // The root menu
  //

  wii_menu_root = wii_create_tree_node( NODETYPE_ROOT, "root" );

  TREENODE* child = NULL;
  child = wii_create_tree_node( NODETYPE_RESUME, "Resume" );
  wii_add_child( wii_menu_root, child );

  child = NULL;
  child = wii_create_tree_node( NODETYPE_RESET, "Reset" );
  wii_add_child( wii_menu_root, child );

  child = wii_create_tree_node( NODETYPE_LOAD_ROM, "Load cartridge" );
  wii_add_child( wii_menu_root, child );

  child = wii_create_tree_node( NODETYPE_CARTRIDGE_SETTINGS_CURRENT_SPACER, "" );
  wii_add_child( wii_menu_root, child );

  TREENODE *cart_settings = wii_create_tree_node( 
    NODETYPE_CARTRIDGE_SETTINGS_CURRENT, "Cartridge settings (current cartridge)" );
  wii_add_child( wii_menu_root, cart_settings );    

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( wii_menu_root, child );

  //
  // Save state management
  //

  TREENODE *state = wii_create_tree_node( 
    NODETYPE_SAVE_STATE_MANAGEMENT, "Save state management" );
  wii_add_child( wii_menu_root, state );

  child = wii_create_tree_node( NODETYPE_AUTO_LOAD_STATE, 
    "Auto load " );
  child->x = -2; child->value_x = -3;
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_AUTO_SAVE_STATE, 
    "Auto save " );
  child->x = -2; child->value_x = -3;
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_LOAD_STATE, 
    "Load saved state" );
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_SAVE_STATE, 
    "Save state (current cartridge)" );
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_DELETE_STATE, 
    "Delete saved state (current cartridge)" );
  wii_add_child( state, child );

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( wii_menu_root, child );

  //
  // The cartridge settings (current) menu
  //   

  child = wii_create_tree_node( NODETYPE_CART_ORIENT, 
    "Orientation " );
  child->x = -2; child->value_x = -3;
  wii_add_child( cart_settings, child );

  child = wii_create_tree_node( NODETYPE_MAX_FRAMES_CART, 
    "Maximum frame rate " );
  child->x = -2; child->value_x = -3;
  wii_add_child( cart_settings, child );

  // Save/Revert/Delete

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( cart_settings, child );

  child = wii_create_tree_node( 
    NODETYPE_SAVE_CARTRIDGE_SETTINGS, "Save settings" );
  wii_add_child( cart_settings, child );  

  child = wii_create_tree_node( 
    NODETYPE_REVERT_CARTRIDGE_SETTINGS, "Revert to saved settings" );
  wii_add_child( cart_settings, child );  

  child = wii_create_tree_node( 
    NODETYPE_DELETE_CARTRIDGE_SETTINGS, "Delete settings" );
  wii_add_child( cart_settings, child );  

  //
  // The display settings menu
  //

  TREENODE *display = wii_create_tree_node( NODETYPE_DISPLAY_SETTINGS, 
    "Display settings" );
  wii_add_child( wii_menu_root, display );

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( wii_menu_root, child );

  child = wii_create_tree_node( NODETYPE_RESIZE_SCREEN, 
    "Screen size (horizontal) " );      
  child->x = -2; child->value_x = -3;
  wii_add_child( display, child );     

  child = wii_create_tree_node( NODETYPE_RESIZE_SCREEN_VERT, 
    "Screen size (vertical) " );      
  child->x = -2; child->value_x = -3;
  wii_add_child( display, child );   

  child = wii_create_tree_node( NODETYPE_SPACER, "" );
  wii_add_child( display, child );

  child = wii_create_tree_node( NODETYPE_VSYNC, 
    "Vertical sync " );                        
  child->x = -2; child->value_x = -3;
  wii_add_child( display, child );   

  child = wii_create_tree_node( NODETYPE_MAX_FRAMES, 
    "Maximum frame rate " );
  child->x = -2; child->value_x = -3;
  wii_add_child( display, child );

  //
  // The advanced menu
  //

  TREENODE *advanced = wii_create_tree_node( NODETYPE_ADVANCED, 
    "Advanced" );
  wii_add_child( wii_menu_root, advanced );    

  child = wii_create_tree_node( NODETYPE_DEBUG_MODE, 
    "Debug mode " );
  child->x = -2; child->value_x = -3;
  wii_add_child( advanced, child );

  child = wii_create_tree_node( NODETYPE_TOP_MENU_EXIT, 
    "Top menu exit " );
  child->x = -2; child->value_x = -3;
  wii_add_child( advanced, child );

  child = wii_create_tree_node( NODETYPE_WIIMOTE_MENU_ORIENT, 
    "Wiimote (menu) " );
  child->x = -2; child->value_x = -3;
  wii_add_child( advanced, child );

  wii_menu_push( wii_menu_root );	
}

/*
 * Updates the buffer with the header message for the current menu
 *
 * menu     The menu
 * buffer   The buffer to update with the header message for the
 *          current menu.
 */
void wii_menu_handle_get_header( TREENODE* menu, char *buffer )
{
  switch( menu->node_type )
  {
    case NODETYPE_LOAD_ROM:
      if( !games_read )
      {
        snprintf( buffer, WII_MENU_BUFF_SIZE, "Reading game list..." );                
      }
      break;
    case NODETYPE_LOAD_STATE:
      if( !save_states_read )
      {
        snprintf( buffer, WII_MENU_BUFF_SIZE, "Reading saved state list..." );                            
      }
      break;
    default:
      /* do nothing */
      break;
  }
}

/*
 * Updates the buffer with the footer message for the current menu
 *
 * menu     The menu
 * buffer   The buffer to update with the footer message for the
 *          current menu.
 */
void wii_menu_handle_get_footer( TREENODE* menu, char *buffer )
{
  switch( menu->node_type )
  {
    case NODETYPE_LOAD_ROM:
      if( games_read )
      {
        wii_get_list_footer( menu, "cartridge", buffer );
      }
      break;
    case NODETYPE_LOAD_STATE:
      wii_get_list_footer( menu, "state save", buffer );
      break;
    default:
      break;
  }
}

/*
 * Updates the buffer with the name of the specified node
 *
 * node     The node
 * name     The name of the specified node
 * value    The value of the specified node
 */
void wii_menu_handle_get_node_name( 
  TREENODE* node, char *buffer, char* value )
{
  const char* strmode = NULL;
  int index;

  snprintf( buffer, WII_MENU_BUFF_SIZE, "%s", node->name );

  switch( node->node_type )
  {
    case NODETYPE_RESIZE_SCREEN:
      snprintf( value, WII_MENU_BUFF_SIZE, "%s", 
        ( ( wii_screen_x == DEFAULT_SCREEN_X && 
          wii_screen_y == DEFAULT_SCREEN_Y ) ? "(default)" : "Custom" ) );
      break;
    case NODETYPE_RESIZE_SCREEN_VERT:
      snprintf( value, WII_MENU_BUFF_SIZE, "%s", 
        ( ( wii_screen_x_vert == DEFAULT_SCREEN_X_VERT && 
          wii_screen_y_vert == DEFAULT_SCREEN_Y_VERT ) ? "(default)" : "Custom" ) );
      break;
    case NODETYPE_MAX_FRAMES:
      snprintf( value, WII_MENU_BUFF_SIZE, "%d", wii_max_frames );
      break;
    case NODETYPE_MAX_FRAMES_CART:
      if( wii_handy_db_entry.maxFrames == MAX_FRAMES_DEFAULT )
      {
        snprintf( value, WII_MENU_BUFF_SIZE, "(default)" );
      }
      else
      {
        snprintf( value, WII_MENU_BUFF_SIZE, "%d", 
          wii_handy_db_entry.maxFrames );
      }      
      break;
    case NODETYPE_DEBUG_MODE:
    case NODETYPE_TOP_MENU_EXIT:
    case NODETYPE_AUTO_LOAD_STATE:
    case NODETYPE_AUTO_SAVE_STATE:
    case NODETYPE_VSYNC:
      {
        BOOL enabled = FALSE;
        switch( node->node_type )
        {
          case NODETYPE_VSYNC:
            enabled = ( wii_vsync == VSYNC_ENABLED );
            break;
          case NODETYPE_DEBUG_MODE:
            enabled = wii_debug;
            break;
          case NODETYPE_TOP_MENU_EXIT:
            enabled = wii_top_menu_exit;
            break;
          case NODETYPE_AUTO_LOAD_STATE:
            enabled = wii_auto_load_state;
            break;
          case NODETYPE_AUTO_SAVE_STATE:
            enabled = wii_auto_save_state;
            break;
          default:
            /* do nothing */
            break;
        }
        snprintf( value, WII_MENU_BUFF_SIZE, "%s", 
          enabled ? "Enabled" : "Disabled" );
        break;
    }
    case NODETYPE_WIIMOTE_MENU_ORIENT:
      if( wii_mote_menu_vertical )
      {
        strmode="Upright";
      }
      else
      {
        strmode="Sideways";
      }
      snprintf( value, WII_MENU_BUFF_SIZE, "%s", strmode );
      break;
    case NODETYPE_CART_ORIENT:
      switch( wii_handy_db_entry.orient )
      {
        case MIKIE_BAD_MODE:
          strmode="(auto)";
          break;
        case MIKIE_NO_ROTATE:
          strmode="Horizontal";
          break;
        case MIKIE_ROTATE_L:
          strmode="Vertical (left)";
          break;
        case MIKIE_ROTATE_R:
          strmode="Vertical (right)";
          break;
      }
      snprintf( value, WII_MENU_BUFF_SIZE, "%s", strmode );
      break;
    default:
      break;
  }
}

/*
 * React to the "select" event for the specified node
 *
 * node     The node
 */
void wii_menu_handle_select_node( TREENODE *node )
{
  char buff[WII_MAX_PATH];

  switch( node->node_type )
  {
    case NODETYPE_RESIZE_SCREEN:
    case NODETYPE_RESIZE_SCREEN_VERT:
      {
        wii_sdl_black_back_surface();
        BOOL horiz = ( node->node_type == NODETYPE_RESIZE_SCREEN );
        wii_handy_set_surface( horiz ); 
        wii_resize_screen_draw_border( blit_surface, 0, blit_surface->h );
        wii_sdl_put_image_normal( 1 );
        wii_sdl_flip(); 
        if( horiz )
        {
          resize_info rinfo = 
            { DEFAULT_SCREEN_X, DEFAULT_SCREEN_Y, 
              wii_screen_x, wii_screen_y };
          wii_resize_screen_gui( &rinfo );
          wii_screen_x = rinfo.currentX;
          wii_screen_y = rinfo.currentY;
        }
        else
        {
          resize_info rinfo =
            { DEFAULT_SCREEN_X_VERT, DEFAULT_SCREEN_Y_VERT, 
              wii_screen_x_vert, wii_screen_y_vert };
          wii_resize_screen_gui( &rinfo );
          wii_screen_x_vert = rinfo.currentX;
          wii_screen_y_vert = rinfo.currentY;
        }
      }
      break;
    case NODETYPE_MAX_FRAMES:
      ++wii_max_frames;
      if( wii_max_frames > 80 )
      {
        wii_max_frames = 30;
      }
      break;
    case NODETYPE_CART_ORIENT:
      ++wii_handy_db_entry.orient;
      if( wii_handy_db_entry.orient > MIKIE_ROTATE_R )
      {
        wii_handy_db_entry.orient = MIKIE_BAD_MODE;
      }
      break;
    case NODETYPE_MAX_FRAMES_CART:
      ++wii_handy_db_entry.maxFrames;
      if( wii_handy_db_entry.maxFrames > 80 )
      {
        wii_handy_db_entry.maxFrames = MAX_FRAMES_DEFAULT;
      }
      else if( wii_handy_db_entry.maxFrames < 30 )
      {
        wii_handy_db_entry.maxFrames = 30;
      }
      break;
    case NODETYPE_VSYNC:
      wii_set_vsync( wii_vsync ^ 1 );
      break;
    case NODETYPE_ROM:            
      snprintf( 
        buff, sizeof(buff), "%s%s", wii_get_roms_dir(), node->name ); 
      last_rom_index = wii_menu_get_current_index();
      wii_start_emulation( buff, NULL, false, false );
      break;
    case NODETYPE_RESUME:
      wii_resume_emulation();
      break;
    case NODETYPE_RESET:
      wii_reset_emulation();
      break;
    case NODETYPE_STATE_SAVE:
      snprintf( 
        buff, sizeof(buff), "%s%s", wii_get_saves_dir(), node->name );  
      wii_start_snapshot( buff );
      break;
    case NODETYPE_TOP_MENU_EXIT:
      wii_top_menu_exit ^= 1;
      break;
    case NODETYPE_WIIMOTE_MENU_ORIENT:
      wii_mote_menu_vertical ^= 1;
      break;
    case NODETYPE_DEBUG_MODE:
      wii_debug ^= 1;
      break;
    case NODETYPE_AUTO_LOAD_STATE:
      wii_auto_load_state ^= 1;
      break;
    case NODETYPE_AUTO_SAVE_STATE:
      wii_auto_save_state ^= 1;
      break;
    case NODETYPE_SAVE_STATE_MANAGEMENT:
    case NODETYPE_ADVANCED:
    case NODETYPE_LOAD_ROM:     
    case NODETYPE_DISPLAY_SETTINGS:
    case NODETYPE_CARTRIDGE_SETTINGS_CURRENT:
      wii_menu_push( node );
      if( node->node_type == NODETYPE_LOAD_ROM )
      {
        wii_menu_move( node, last_rom_index );
      }
      break;
    case NODETYPE_LOAD_STATE:
      wii_menu_clear_children( node );
      wii_menu_push( node );
      save_states_read = FALSE;
      break;
    case NODETYPE_SAVE_STATE:
      save_snapshot( NULL, TRUE );
      break;
    case NODETYPE_DELETE_STATE:
      wii_delete_snapshot();
      break;
    case NODETYPE_SAVE_CARTRIDGE_SETTINGS:
      if( wii_handy_db_entry.name[0] == '\0' )
      {
        char cartname[WII_MAX_PATH];        
        Util_splitpath( wii_last_rom, NULL, cartname );
        char *ptr = strrchr( cartname, '.' );
        if( ptr ) *ptr = '\0';
        Util_strlcpy( 
          wii_handy_db_entry.name, cartname, 
          sizeof( wii_handy_db_entry.name ) );
      }
      if( wii_handy_db_write_entry( 
        wii_cartridge_hash, &wii_handy_db_entry ) )
      {
        wii_handy_db_entry.loaded = 1;
        wii_set_status_message( "Successfully saved cartridge settings." );
      }
      else
      {
        wii_set_status_message( 
          "An error occurred saving cartridge settings." );
      }
      break;
    case NODETYPE_DELETE_CARTRIDGE_SETTINGS:
      if( wii_handy_db_delete_entry( wii_cartridge_hash ) )
      {
        wii_menu_reset_indexes();
        wii_menu_move( wii_menu_stack[wii_menu_stack_head], 1 );
        wii_set_status_message( "Successfully deleted cartridge settings." );
      }
      else
      {
        wii_set_status_message( 
          "An error occurred deleting cartridge settings." );
      }
      // Load the values for the entry
      wii_handy_db_get_entry( wii_cartridge_hash, &wii_handy_db_entry );
      break;
    case NODETYPE_REVERT_CARTRIDGE_SETTINGS:
      wii_handy_db_get_entry( wii_cartridge_hash, &wii_handy_db_entry );
      wii_set_status_message( "Successfully reverted to saved settings." );
      break;
    default:
      /* do nothing */
      break;
  }
}

/*
 * Determines whether the node is currently visible
 *
 * node     The node
 * return   Whether the node is visible
 */
BOOL wii_menu_handle_is_node_visible( TREENODE *node )
{
  switch( node->node_type )
  {
    case NODETYPE_SAVE_STATE:
    case NODETYPE_RESET:
    case NODETYPE_RESUME:
    case NODETYPE_CARTRIDGE_SETTINGS_CURRENT:
    case NODETYPE_CARTRIDGE_SETTINGS_CURRENT_SPACER:
      return wii_last_rom != NULL;
      break;
    case NODETYPE_DELETE_CARTRIDGE_SETTINGS:
    case NODETYPE_REVERT_CARTRIDGE_SETTINGS:
      return wii_last_rom != NULL && wii_handy_db_entry.loaded;
      break;
    case NODETYPE_DELETE_STATE:
      if( wii_last_rom != NULL )
      {
        char savename[WII_MAX_PATH];
        wii_snapshot_handle_get_name( wii_last_rom, savename );
        return Util_fileexists( savename );
      }
      return FALSE;
      break;          
    default:
      /* do nothing */
      break;
  }

  return TRUE;
}

/*
 * Determines whether the node is selectable
 *
 * node     The node
 * return   Whether the node is selectable
 */
BOOL wii_menu_handle_is_node_selectable( TREENODE *node )
{
  if( node->node_type == NODETYPE_CARTRIDGE_SETTINGS_CURRENT_SPACER )
  {
    return FALSE;
  }

  return TRUE;
}

/*
 * Provides an opportunity for the specified menu to do something during 
 * a display cycle.
 *
 * menu     The menu
 */
void wii_menu_handle_update( TREENODE *menu )
{
  switch( menu->node_type )
  {
    case NODETYPE_LOAD_ROM:
      if( !games_read )
      {
        wii_read_game_list( menu );  
        wii_menu_reset_indexes();    
        wii_menu_move( menu, 1 );
        wii_menu_force_redraw = 1;
      }
      break;
    case NODETYPE_LOAD_STATE:
      if( !save_states_read )
      {
        wii_read_save_state_list( menu );    
        wii_menu_reset_indexes();    
        wii_menu_move( menu, 1 );
        wii_menu_force_redraw = 1;            
      }
      break;
    default:
      /* do nothing */
      break;
  }
}

/*
 * Reads the list of games into the specified menu
 *
 * menu     The menu to read the games into
 */
static void wii_read_game_list( TREENODE *menu )
{
  DIR_ITER *romdir = diropen( wii_get_roms_dir() );
  if( romdir != NULL)
  {
    struct stat statbuf;
    char filepath[WII_MAX_PATH];
    while( dirnext( romdir, filepath, &statbuf ) == 0 )
    {               
      if( strcmp( ".", filepath ) != 0 && 
        strcmp( "..", filepath ) != 0 )
      {				                
        if( !S_ISDIR( statbuf.st_mode ) )
        {
          TREENODE *child = 
            wii_create_tree_node( NODETYPE_ROM, filepath );

          wii_add_child( menu, child );
        }				
      }
    }

    dirclose( romdir );
  }
  else
  {
    wii_set_status_message( "Error opening roms directory." );
  }

  // Sort the games list
  qsort( menu->children, menu->child_count, 
    sizeof(*(menu->children)), wii_menu_name_compare );

  games_read = 1;
}

/*
 * Reads the list of snapshots into the specified menu
 *
 * menu     The menu to read the snapshots into
 */
static void wii_read_save_state_list( TREENODE *menu )
{
  DIR_ITER *ssdir = diropen( wii_get_saves_dir() );
  if( ssdir != NULL)
  {   
    struct stat statbuf;
    char ext[WII_MAX_PATH];
    char filepath[WII_MAX_PATH];
    while( dirnext( ssdir, filepath, &statbuf ) == 0 ) 
    {            
      if( strcmp( ".", filepath ) != 0 && 
        strcmp( "..", filepath ) != 0 )
      {			                
        Util_getextension( filepath, ext );
        if( strcmp( ext, WII_SAVE_GAME_EXT ) == 0 )
        {                    
          if( !S_ISDIR( statbuf.st_mode ) )
          {
            // TODO: Check to see if a rom exists for the snapshot
            // TODO: Provide option to display cart info from 
            //       header
            TREENODE *child = 
              wii_create_tree_node( NODETYPE_STATE_SAVE, filepath );

            wii_add_child( menu, child );
          }				
        }
      }
    }

    dirclose( ssdir );
  }
  else
  {
    wii_set_status_message( "Error opening state saves directory." );
  }

  // Sort the games list
  qsort( menu->children, menu->child_count, 
    sizeof(*(menu->children)), wii_menu_name_compare );

  save_states_read = TRUE;
}

/*
 * Invoked after exiting the menu loop
 */
void wii_menu_handle_post_loop()
{
}

/*
 * Invoked prior to entering the menu loop
 */
void wii_menu_handle_pre_loop()
{
}

/*
 * Invoked when the home button is pressed when the 
 * menu is being displayed
 */
void wii_menu_handle_home_button()
{
}