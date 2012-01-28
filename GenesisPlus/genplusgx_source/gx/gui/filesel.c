/*
 * filesel.c
 * 
 *   ROM File Browser
 *
 *   Eke-Eke (2009,2010)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

#include "shared.h"
#include "filesel.h"
#include "font.h"
#include "gui.h"
#include "file_load.h"
#include "file_slot.h"

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif

#define BG_COLOR_1 {0x49,0x49,0x49,0xff}
#define BG_COLOR_2 {0x66,0x66,0x66,0xff}

extern const u8 Browser_dir_png[];
extern const u8 Snap_empty_png[];
extern const u8 Snap_frame_png[];

FILEENTRIES filelist[MAXFILES];

static int offset        = 0;
static int selection     = 0;
static int maxfiles      = 0;
static int string_offset = 0;
static char prev_folder[MAXJOLIET];
static void selector_cb(void);

/*****************************************************************************/
/*  GUI Buttons data                                                         */
/*****************************************************************************/
static butn_data arrow_up_data =
{
  {NULL,NULL},
  {Button_up_png,Button_up_over_png}
};

static butn_data arrow_down_data =
{
  {NULL,NULL},
  {Button_down_png,Button_down_over_png}
};

/*****************************************************************************/
/*  GUI Arrows button                                                        */
/*****************************************************************************/

static gui_butn arrow_up = {&arrow_up_data,BUTTON_VISIBLE|BUTTON_ACTIVE|BUTTON_OVER_SFX,{0,0,0,0},14,76,360,32};
static gui_butn arrow_down = {&arrow_down_data,BUTTON_VISIBLE|BUTTON_ACTIVE|BUTTON_OVER_SFX,{0,0,0,0},14,368,360,32};

/*****************************************************************************/
/*  GUI helpers                                                              */
/*****************************************************************************/
static gui_item action_cancel =
{
  NULL,Key_B_png,"","Previous Directory",10,422,28,28
};

static gui_item action_select =
{
  NULL,Key_A_png,"","Load ROM file",602,422,28,28
};

/*****************************************************************************/
/*  GUI Background images                                                    */
/*****************************************************************************/
static gui_image bg_filesel[10] =
{
  {NULL,Bg_main_png,IMAGE_VISIBLE,374,140,284,288,255},
  {NULL,Bg_overlay_png,IMAGE_VISIBLE|IMAGE_REPEAT,0,0,640,480,255},
  {NULL,Banner_top_png,IMAGE_VISIBLE,0,0,640,108,255},
  {NULL,Banner_bottom_png,IMAGE_VISIBLE,0,380,640,100,255},
  {NULL,Main_logo_png,IMAGE_VISIBLE,466,40,152,44,255},
  {NULL,Frame_s1_png,IMAGE_VISIBLE,8,70,372,336,152},
  {NULL,Frame_s2_png,0,384,264,248,140,152},
  {NULL,Snap_empty_png,IMAGE_VISIBLE,422,114,164,116,255},
  {NULL,NULL,0,424,116,160,112,255},
  {NULL,Snap_frame_png,IMAGE_VISIBLE,388,112,236,148,255}
};

/*****************************************************************************/
/*  GUI Descriptor                                                           */
/*****************************************************************************/
static gui_menu menu_selector =
{
  "Game Selection",
  -1,-1,
  0,0,10,0,
  NULL,
  NULL,
  bg_filesel,
  {&action_cancel, &action_select},
  {&arrow_up,&arrow_down},
  selector_cb
};


static void selector_cb(void)
{
  int i;
  char text[MAXPATHLEN];
  int yoffset = 108;

  /* Initialize directory icon */
  gui_image dir_icon;
  dir_icon.texture = gxTextureOpenPNG(Browser_dir_png,0);
  dir_icon.w = dir_icon.texture->width;
  dir_icon.h = dir_icon.texture->height;
  dir_icon.x = 26;
  dir_icon.y = (26 - dir_icon.h)/2;

  /* Initialize selection bar */
  gui_image bar_over;
  bar_over.texture = gxTextureOpenPNG(Overlay_bar_png,0);
  bar_over.w = bar_over.texture->width;
  bar_over.h = bar_over.texture->height;
  bar_over.x = 16;
  bar_over.y = (26 - bar_over.h)/2;

  /* Draw browser array */
  gxDrawRectangle(15, 108, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 134, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 160, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 186, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 212, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 238, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 264, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 290, 358, 26, 127, (GXColor)BG_COLOR_2);
  gxDrawRectangle(15, 316, 358, 26, 127, (GXColor)BG_COLOR_1);
  gxDrawRectangle(15, 342, 358, 26, 127, (GXColor)BG_COLOR_2);

  /* Draw Files list */
  for (i = offset; (i < (offset + 10)) && (i < maxfiles); i++)
  {
    if (i == selection)
    {
      /* selection bar */
      gxDrawTexture(bar_over.texture,bar_over.x,yoffset+bar_over.y,bar_over.w,bar_over.h,255);

      /* scrolling text */
      if ((string_offset/10) >= strlen(filelist[i].filename))
      {
        string_offset = 0;
      }

      if (string_offset)
      {
        sprintf(text,"%s ",filelist[i].filename+string_offset/10);
        strncat(text, filelist[i].filename, string_offset/10);
      }
      else
      {
        strcpy(text, filelist[i].filename);
      }

      /* print text */
      if (filelist[i].flags)
      {
        /* directory icon */
        gxDrawTexture(dir_icon.texture,dir_icon.x,yoffset+dir_icon.y,dir_icon.w,dir_icon.h,255);
        if (FONT_write(text,18,dir_icon.x+dir_icon.w+6,yoffset+22,bar_over.w-dir_icon.w-26,(GXColor)WHITE))
        {
          /* string is too large -> scroll text */
          string_offset ++;
        }
      }
      else
      {
        if (FONT_write(text,18,dir_icon.x,yoffset+22,bar_over.w-20,(GXColor)WHITE))
        {
          /* text scrolling */
          string_offset ++;
        }
      }
    }
    else
    {
      if (filelist[i].flags)
      {
        /* directory icon */
        gxDrawTexture(dir_icon.texture,dir_icon.x,yoffset+dir_icon.y,dir_icon.w,dir_icon.h,255);
        FONT_write(filelist[i].filename,18,dir_icon.x+dir_icon.w+6,yoffset+22,bar_over.w-dir_icon.w-26,(GXColor)WHITE);
      }
      else
      {
        FONT_write(filelist[i].filename,18,dir_icon.x,yoffset+22,bar_over.w-20,(GXColor)WHITE);
      }
    }

    yoffset += 26;
  }

  gxTextureClose(&bar_over.texture);
  gxTextureClose(&dir_icon.texture);
}  
  

/****************************************************************************
 * FileSelector
 *
 * Browse directories and select a file from the file listing
 * return ROM size
 *
 ****************************************************************************/ 
int FileSelector(void)
{
  short p;
  int i;
  int size = 0;
  int old = -1;
  char fname[MAXPATHLEN];
  char text[MAXPATHLEN];
  FILE *xml,*snap;
  gui_menu *m = &menu_selector;

#ifdef HW_RVL
  int x,y;
  gui_butn *button;
#endif

  /* background type */
  if (config.bg_type > 0)
  {
    bg_filesel[0].state &= ~IMAGE_REPEAT;
    bg_filesel[0].data = (config.bg_type > 1) ? Bg_main_png : Bg_main_2_png;
    bg_filesel[0].x = 374;
    bg_filesel[0].y = 140;
    bg_filesel[0].w = 284;
    bg_filesel[0].h = 288;
  }
  else
  {
    bg_filesel[0].state |= IMAGE_REPEAT;
    bg_filesel[0].data = Bg_layer_png;
    bg_filesel[0].x = 0;
    bg_filesel[0].y = 0;
    bg_filesel[0].w = 640;
    bg_filesel[0].h = 480;
  }

  /* background overlay */
  if (config.bg_overlay)
  {
    bg_filesel[1].state |= IMAGE_VISIBLE;
  }
  else
  {
    bg_filesel[1].state &= ~IMAGE_VISIBLE;
  }

  /* Initialize Menu */
  GUI_InitMenu(m);
  string_offset = 0;

  while (1)
  {
    /* ROM file snapshot/database */
    if (old != selection)
    {
      old = selection;
      string_offset = 0;

      /* delete previous texture if any */
      gxTextureClose(&bg_filesel[8].texture);
      bg_filesel[8].state &= ~IMAGE_VISIBLE;
      bg_filesel[6].state &= ~IMAGE_VISIBLE;

      if (!filelist[selection].flags)
      {
        /* get ROM filename without extension */
        sprintf (text, "%s", filelist[selection].filename);
        if (strlen(text) >= 4)
          text[strlen(text) - 4] = 0;

        /* ROM database informations */
        sprintf (fname, "%s/db/%s.xml", DEFAULT_PATH, text);
        xml = fopen(fname, "rb");
        if (xml)
        {
          bg_filesel[6].state |= IMAGE_VISIBLE;
          fclose(xml); /* TODO */
        }

        /* open screenshot file */
        sprintf (fname, "%s/snaps/%s.png", DEFAULT_PATH, text);
        snap = fopen(fname, "rb");
        if (snap)
        {
          bg_filesel[8].texture = gxTextureOpenPNG(0,snap);
          if (bg_filesel[8].texture)
            bg_filesel[8].state |= IMAGE_VISIBLE;
          fclose(snap);
        }
      }
    }

    /* update helper */
    if (m->selected != -1)
    {
      /* out of focus */
      strcpy(action_select.comment,"");
    }
    else if (filelist[selection].flags)
    {
      /* this is a directory */
      strcpy(action_select.comment,"Open Directory");
    }
    else
    {
      /* this is a file */
      strcpy(action_select.comment,"Load ROM File");
    }

    /* Draw menu*/
    GUI_DrawMenu(m);

#ifdef HW_RVL
    if (Shutdown)
    {
      gxTextureClose(&w_pointer);
      GUI_DeleteMenu(m);
      GUI_FadeOut();
      shutdown();
      SYS_ResetSystem(SYS_POWEROFF, 0, 0);
    }
    else if (m_input.ir.valid)
    {
      /* get cursor position */
      x = m_input.ir.x;
      y = m_input.ir.y;

      /* draw wiimote pointer */
      gxDrawTextureRotate(w_pointer, x-w_pointer->width/2, y-w_pointer->height/2, w_pointer->width, w_pointer->height,m_input.ir.angle,255);

      /* ensure we are in the selectable area */
      if ((x < 380) && (y >= 108) && (y <= 368))
      {
        /* find selected item */
        selection = (y - 108) / 26;
        if (selection > 9) selection = 9;
        selection += offset;
        if (selection >= maxfiles) selection = old;

        /* reset selection */
        m->selected = -1;
      }
      else
      {
        /* disable selection */
        m->selected = m->max_buttons + 2;

        /* find selected button */
        for (i=0; i<2; i++)
        {
          button = m->arrows[i];
          if (button)
          {
            if (button->state & BUTTON_VISIBLE)
            {
              if ((x>=button->x)&&(x<=(button->x+button->w))&&(y>=button->y)&&(y<=(button->y+button->h)))
              {
                m->selected = m->max_buttons + i;
                break;
              }
            }
          }
        }
      }
    }
    else
    {
      /* reset selection */
      m->selected = -1;
    }
#endif

    /* copy EFB to XFB */
    gxSetScreen();

    p = m_input.keys;

    /* highlight next item */
    if (p & PAD_BUTTON_DOWN)
    {
      selection++;
      if (selection == maxfiles)
        selection = offset = 0;
      if ((selection - offset) >= 10)
        offset += 10;
    }

    /* highlight previous item */
    else if (p & PAD_BUTTON_UP)
    {
      selection--;
      if (selection < 0)
      {
        selection = maxfiles - 1;
        offset = selection - 10 + 1;
      }
      if (selection < offset)
        offset -= 10;
      if (offset < 0)
        offset = 0;
    }

    /* go back one page */
    else if (p & PAD_TRIGGER_L)
    {
      selection -= 10;
      if (selection < 0)
      {
        selection = maxfiles - 1;
        offset = selection - 10 + 1;
      }
      if (selection < offset)
        offset -= 10;
      if (offset < 0)
        offset = 0;
    }

    /* go forward one page */
    else if (p & PAD_TRIGGER_R)
    {
      selection += 10;
      if (selection > maxfiles - 1)
        selection = offset = 0;
      if ((selection - offset) >= 10)
        offset += 10;
    }

    /* quit */
    else if (p & PAD_TRIGGER_Z)
    {
      GUI_DeleteMenu(m);
      return 0;
    }

    /* previous directory */
    else if (p & PAD_BUTTON_B)
    {
      string_offset = 0;

      /* update browser directory (and get current folder)*/
      if (UpdateDirectory(1, prev_folder))
      {
        /* get directory entries */
        maxfiles = ParseDirectory();

        /* clear selection by default */
        selection = offset = 0;

        /* select previous directory */
        for (i=0; i<maxfiles; i++)
        {
          if (filelist[i].flags && !strcmp(prev_folder,filelist[i].filename))
          {
            selection = i;
            offset = (i / 10) * 10;
            i = maxfiles;
          }
        }
      }
      else
      {
        /* exit */
        GUI_DeleteMenu(m);
        return 0;
      }
    }

    /* open selected file or directory */
    else if (p & PAD_BUTTON_A)
    {
      string_offset = 0;

#ifdef HW_RVL
      /* arrow buttons selected */
      if (m->selected == m->max_buttons)
      {
        /* up arrow */
        selection--;
        if (selection < 0)
        {
          selection = maxfiles - 1;
          offset = selection - 10 + 1;
        }
        if (selection < offset) offset -= 10;
        if (offset < 0) offset = 0;
      }
      else if (m->selected == (m->max_buttons+1))
      {
        /* down arrow */
        selection++;
        if (selection == maxfiles)
          selection = offset = 0;
        if ((selection - offset) >= 10)
          offset += 10;
      }
#endif

      /* ensure we are in focus area */
      if (m->selected < m->max_buttons)
      {
        if (filelist[selection].flags)
        {
          /* get new directory */
          UpdateDirectory(0, filelist[selection].filename);

          /* get directory entries */
          maxfiles = ParseDirectory();

          /* clear selection by default */
          selection = offset = 0;
        }
        else 
        {
          /* clear existing patches before loading new ROM file */
          ggenie_shutdown();
          areplay_shutdown();
  
          /* load ROM file from device */
          size = LoadFile(cart.rom, selection, fname);

          /* exit menu */
          GUI_DeleteMenu(m);

          /* load new game */
          if (size)
          {
            /* save previous game state */
            if (config.s_auto & 2)
            {
              slot_autosave(config.s_default,config.s_device);
            }

            /* reinitialize emulation */
            reloadrom(size,fname);
          }

          return size;
        }
      }
    }
  }
}

void ClearSelector(u32 max)
{
  maxfiles = max;
  offset = 0;
  selection = 0;
}
