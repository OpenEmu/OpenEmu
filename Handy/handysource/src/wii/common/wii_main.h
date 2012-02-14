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

#ifndef WII_MAIN_H
#define WII_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gccore.h>
#include <sys/dir.h>
#include <unistd.h>

#include "wii_app_common.h"
#include "wii_util.h"

#define WII_WIDTH 640
#define WII_HEIGHT 480

struct treenode;
typedef struct treenode *TREENODEPTR;

/*
 * Simple hierarchical menu structure
 */
typedef struct treenode {
  char *name;
  enum NODETYPE node_type;
  TREENODEPTR *children;
  u16 child_count;
  u16 max_children;
  s16 x;
  s16 value_x;
} TREENODE;

#define WII_MAX_PATH MAXPATHLEN + 1
#define WII_MENU_BUFF_SIZE 256

// vsync modes
#define VSYNC_DISABLED 0
#define VSYNC_ENABLED 1

// The root of the menu
extern TREENODE *wii_menu_root;
// Whether the wiimote (for the menu) is horizontal or vertical
extern BOOL wii_mote_menu_vertical;
// Whether to return to the top menu after exiting a game
extern BOOL wii_top_menu_exit;
// The vsync mode
extern int wii_vsync;
// The stack containing the menus the user has navigated
extern TREENODEPTR wii_menu_stack[4];
// The head of the menu stack
extern s8 wii_menu_stack_head;
// Two framebuffers (double buffered)
extern u32 *wii_xfb[2];
// Whether to quite the menu loop
extern BOOL wii_menu_quit_loop;
// Forces the menu to be redrawn
extern BOOL wii_menu_force_redraw;
// The last rom that was successly executed
extern char *wii_last_rom;
// Offset of the selection bar in the menu
extern int wii_menu_sel_offset;
// The menu selection color
extern RGBA wii_menu_sel_color;
// Whether the Wii is PAL or NTSC
extern BOOL wii_is_pal;

// The display mode (from SDL)
extern GXRModeObj *vmode;	

/*
 * Creates and returns a new menu tree node
 *
 * type     The type of the node
 * name     The name for the node
 * return   The newly created node
 */
extern TREENODE* wii_create_tree_node( enum NODETYPE type, const char *name );

/*
 * Clears the children for the specified node
 *
 * node     The node to clear the children for
 */
extern void wii_menu_clear_children( TREENODE* node );

/*
 * Adds the specified child node to the specified parent
 *
 * parent   The parent
 * child    The child to add to the parent
 */
extern void wii_add_child( TREENODE *parent, TREENODE *childp );

/*
 * Pushes the specified menu onto the menu stack (occurs when the user
 * navigates to a sub-menu)
 *
 * menu     The menu to push on the stack
 */
extern void wii_menu_push( TREENODE *menu );

/*
 * Resets the menu indexes when an underlying menu in change (push/pop)
 */
extern void wii_menu_reset_indexes();

/*
 * Move the current selection in the menu by the specified step count
 *
 * menu     The current menu
 * steps    The number of steps to move the selection
 */
extern void wii_menu_move( TREENODE *menu, s16 steps );

/*
 * Pops the latest menu off of the menu stack (occurs when the user leaves 
 * the current menu.
 *
 * return   The pop'd menu
 */
extern TREENODE* wii_menu_pop();

/*
 * Used for comparing menu names when sorting (qsort)
 *
 * a        The first tree node to compare
 * b        The second tree node to compare
 * return   The result of the comparison
 */
extern int wii_menu_name_compare( const void *a, const void *b );

/*
 * Returns the standard list footer for snapshots, games, etc.
 *
 * menu     The current menu
 * listname The name of the list (snapshot/game, etc.)
 * buffer   The buffer to write the footer to
 */
extern void wii_get_list_footer( 
  TREENODE* menu, const char *listname, char *buffer );

/*
 * Sets whether to enable VSYNC or not 
 *
 * param    sync Whether to enable VSYNC or not
 */
extern void wii_set_vsync( BOOL sync );

/*
 * Syncs the video 
 */
extern void wii_sync_video();

/*
 * Returns whether VSYNC is enabled
 *
 * return Whether VSYNC is enabled
 */
extern int wii_vsync_enabled();

/*
 * Displays the menu 
 */
extern void wii_menu_show();

/*
 * Returns the current menu index
 *
 * return   The current menu index
 */
extern s16 wii_menu_get_current_index();

//
// Methods to be implemented by application
//

/*
 * Initializes the application
 */
extern void wii_handle_init();

/*
 * Frees resources prior to the application exiting
 */
extern void wii_handle_free_resources();

/*
 * Runs the application (main loop)
 */
extern void wii_handle_run();

//
// Methods to be implemented by application
//

/*
 * Returns the (current) max frame rate
 *
 * return The (current) max frame rate
 */
extern u8 wii_get_max_frames();

/*
 * Updates the buffer with the header message for the current menu
 *
 * menu     The menu
 * buffer   The buffer to update with the header message for the
 *          current menu.
 */
extern void wii_menu_handle_get_header( TREENODE* menu, char *buffer );

/*
 * Updates the buffer with the footer message for the current menu
 *
 * menu     The menu
 * buffer   The buffer to update with the footer message for the
 *          current menu.
 */
extern void wii_menu_handle_get_footer( TREENODE* menu, char *buffer );

/*
 * Updates the buffer with the name of the specified node
 *
 * node     The node
 * name     The name of the specified node
 * value    The value of the specified node
 */
extern void wii_menu_handle_get_node_name( 
  TREENODE* node, char *name, char *value );

/*
 * React to the "select" event for the specified node
 *
 * node     The node
 */
extern void wii_menu_handle_select_node( TREENODE *node );

/*
 * Determines whether the node is currently visible
 *
 * node     The node
 * return   Whether the node is visible
 */
extern BOOL wii_menu_handle_is_node_visible( TREENODE *node );

/*
 * Provides an opportunity for the specified menu to do something during 
 * a display cycle.
 *
 * menu     The menu
 */
extern void wii_menu_handle_update( TREENODE *menu );

/*
 * Invoked prior to entering the menu loop
 */
extern void wii_menu_handle_pre_loop();

/*
 * Invoked after exiting the menu loop
 */
extern void wii_menu_handle_post_loop();

/*
 * Invoked when the home button is pressed when the  menu is being 
 * displayed
 */
extern void wii_menu_handle_home_button();

/*
 * Determines whether the node is selectable
 *
 * node     The node
 * return   Whether the node is selectable
 */
extern BOOL wii_menu_handle_is_node_selectable( TREENODE *node );

#ifdef __cplusplus
}
#endif

#endif
