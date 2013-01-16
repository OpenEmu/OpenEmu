/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - osd.h                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Nmn, Ebenblues                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __OSD_H__
#define __OSD_H__

#include "main/list.h"

/******************************************************************
   osd_corner
   0    1    2 |
    \ __|__/   | Offset always effects the same:
     |     |   |  +X = Leftward   +Y = Upward
   3-|  4  |-5 |  With no offset, the text will touch the border.
     |_____|   |
    /   |   \  |
   6    7    8 |
*******************************************************************/
enum osd_corner {
    OSD_TOP_LEFT,       // 0 in the picture above
    OSD_TOP_CENTER,     // 1 in the picture above
    OSD_TOP_RIGHT,      // 2 in the picture above

    OSD_MIDDLE_LEFT,    // 3 in the picture above
    OSD_MIDDLE_CENTER,  // 4 in the picture above
    OSD_MIDDLE_RIGHT,   // 5 in the picture above

    OSD_BOTTOM_LEFT,    // 6 in the picture above
    OSD_BOTTOM_CENTER,  // 7 in the picture above
    OSD_BOTTOM_RIGHT,   // 8 in the picture above

    OSD_NUM_CORNERS
};

enum osd_message_state {
    OSD_APPEAR,     // OSD message is appearing on the screen
    OSD_DISPLAY,    // OSD message is being displayed on the screen
    OSD_DISAPPEAR,  // OSD message is disappearing from the screen

    OSD_NUM_STATES
};

enum osd_animation_type {
    OSD_NONE,
    OSD_FADE,

    OSD_NUM_ANIM_TYPES
};

typedef struct {
    char *text;        // Text that this object will have when displayed
    enum osd_corner corner; // One of the 9 corners
    float xoffset;     // Relative X position
    float yoffset;     // Relative Y position
    float color[3];    // Red, Green, Blue values
    float sizebox[4];  // bounding box (xmin, ymin, xmax, ymax)
    int state;         // display state of current message
    enum osd_animation_type animation[OSD_NUM_STATES]; // animations for each display state
    unsigned int timeout[OSD_NUM_STATES]; // timeouts for each display state
#define OSD_INFINITE_TIMEOUT 0xffffffff
    unsigned int frames; // number of frames in this state
    int user_managed; // structure managed by caller and not to be freed by us
    struct list_head list;
} osd_message_t;

enum { R, G, B }; // for referencing color array

#ifdef __cplusplus
extern "C" {
#endif

#ifdef M64P_OSD

void osd_init(int width, int height);
void osd_exit(void);
void osd_render(void);
osd_message_t * osd_new_message(enum osd_corner, const char *, ...);
void osd_update_message(osd_message_t *, const char *, ...);
void osd_delete_message(osd_message_t *);
void osd_message_set_static(osd_message_t *);
void osd_message_set_user_managed(osd_message_t *);

#else

static inline void osd_init(int width, int height)
{
}

static inline void osd_exit(void)
{
}

static inline void osd_render(void)
{
}

static inline osd_message_t * osd_new_message(enum osd_corner eCorner, const char *fmt, ...)
{
	return NULL;
}

static inline void osd_update_message(osd_message_t *msg, const char *fmt, ...)
{
}

static inline void osd_delete_message(osd_message_t *msg)
{
}

static inline void osd_message_set_static(osd_message_t *msg)
{
}

static inline void osd_message_set_user_managed(osd_message_t *msg)
{
}

#endif

#ifdef __cplusplus
}
#endif

#endif // __OSD_H__

