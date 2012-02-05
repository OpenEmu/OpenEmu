/****************************************************************************
 *  gui.c
 *
 *  generic GUI Engine (using GX rendering)
 *
 *  Eke-Eke (2009,2010)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/

#include "shared.h"
#include "gui.h"
#include "font.h"

#ifdef HW_RVL
gx_texture *w_pointer;
#endif

u8 SILENT = 0;

/* message box */
static gui_message message_box;
static lwp_t msgboxthread;

/* background color */
static GXColor bg_color;

/* various background colors */
static GXColor bg_colors[BG_COLOR_MAX]=
{
  {0x00,0x00,0x00,0xff}, /* black */
  {0xd4,0xd0,0xc8,0xff}, /* cream */
  {0xbb,0xb0,0x99,0xff}, /* gold */
  {0xd6,0xcb,0xba,0xff}, /* light gold */
  {0xcc,0xcc,0xcc,0xff}, /* light grey */
  {0x66,0x66,0x66,0xff}, /* faded grey */
  {0x50,0x51,0x5b,0xff}, /* grey blue */
  {0xb8,0xc7,0xda,0xff}, /* light blue */
  {0xc0,0xcf,0xe7,0xff}, /* sky blue */
  {0x98,0xb1,0xd8,0xff}, /* sea blue */
  {0x7b,0x8c,0xa6,0xff}, /* violet */
  {0xa9,0xc7,0xc6,0xff}, /* green blue */
  {0x7d,0xa4,0x9f,0xff}, /* darker green blue */
  {0x22,0x52,0x74,0xff}, /* dark blue */
  {0x33,0x33,0x33,0xff}  /* dark grey */
};


/*****************************************************************************/
/*  Generic GUI routines                                                     */
/*****************************************************************************/

/* Allocate Menu texture images data */
void GUI_InitMenu(gui_menu *menu)
{
  int i;
  gui_item *item;
  gui_butn *button;
  gui_image *image;

  /* background elements */
  for (i=0; i<menu->max_images; i++)
  {
    image = &menu->bg_images[i];
    image->texture = gxTextureOpenPNG(image->data,0);
  }

  for (i=0; i<2; i++)
  {
    /* key helpers */
    item = menu->helpers[i];
    if (item)
      item->texture = gxTextureOpenPNG(item->data,0);

    /* arrows */
    button = menu->arrows[i];
    if (button)
    {
      if (!button->data->texture[0])
        button->data->texture[0] = gxTextureOpenPNG(button->data->image[0],0);
      if (!button->data->texture[1])
        button->data->texture[1] = gxTextureOpenPNG(button->data->image[1],0);

      /* initial state */
      button->state &= ~BUTTON_VISIBLE;
      if (((i==0) && (menu->offset != 0)) || ((i==1) && (menu->offset + menu->max_buttons) < menu->max_items))
        button->state |= BUTTON_VISIBLE;
    }
  }

  /* menu buttons */
  for (i=0; i<menu->max_buttons; i++)
  {
    button = &menu->buttons[i];
    if (button->data)
    {
      if (!button->data->texture[0])
        button->data->texture[0] = gxTextureOpenPNG(button->data->image[0],0);
      if (!button->data->texture[1])
        button->data->texture[1] = gxTextureOpenPNG(button->data->image[1],0);
    }
  }

  /* menu items */
  for (i=0; i<menu->max_items; i++)
  {
    item = &menu->items[i];
    if (item->data)
      item->texture = gxTextureOpenPNG(item->data,0);
  }

  /* update message box */
  message_box.parent = menu;
}

/* Release Menu allocated memory */
void GUI_DeleteMenu(gui_menu *menu)
{
  int i;
  gui_butn *button;
  gui_item *item;
  gui_image *image;

  /* background elements */
  for (i=0; i<menu->max_images; i++)
  {
    image = &menu->bg_images[i];
    gxTextureClose(&image->texture);
  }

  for (i=0; i<2; i++)
  {
    /* key helpers */
    item = menu->helpers[i];
    if (item)
      gxTextureClose(&item->texture);

    /* arrows */
    button = menu->arrows[i];
    if (button)
    {
      gxTextureClose(&button->data->texture[0]);
      gxTextureClose(&button->data->texture[1]);
    }
  }

  /* menu buttons */
  for (i=0; i<menu->max_buttons; i++)
  {
    button = &menu->buttons[i];
    if (button->data)
    {
      gxTextureClose(&button->data->texture[0]);
      gxTextureClose(&button->data->texture[1]);
    }
  }

  /* menu items */
  for (i=0; i<menu->max_items; i++)
  {
    item = &menu->items[i];
    gxTextureClose(&item->texture);
  }
}

extern void gxSnapshot(void);

/* Draw Menu */
void GUI_DrawMenu(gui_menu *menu)
{
  int i;
  gui_item *item;
  gui_butn *button;
  gui_image *image;

  /* background color */
  if (menu->screenshot)
  {
    gxClearScreen((GXColor)BLACK);
    gxDrawScreenshot(menu->screenshot);
  }
  else
  {
    gxClearScreen(bg_color);
  }

  /* background elements */
  for (i=0; i<menu->max_images; i++)
  {
    image = &menu->bg_images[i];
    if (image->state & IMAGE_VISIBLE)
    {
      if (image->state & IMAGE_REPEAT)
        gxDrawTextureRepeat(image->texture,image->x,image->y,image->w,image->h,image->alpha);
      else 
        gxDrawTexture(image->texture,image->x,image->y,image->w,image->h,image->alpha);
    }
  }

  /* menu title */
  FONT_write(menu->title, 22,10,56,640,(GXColor)WHITE);

  /* draw buttons + items */
  for (i=0; i<menu->max_buttons; i++)
  {
    button = &menu->buttons[i];

    if (button->state & BUTTON_VISIBLE)
    {
      /* item select (text or image) */
      item = (menu->items) ? (&menu->items[menu->offset + i]) : NULL;
      
      /* draw button + items */ 
      if ((i == menu->selected) || (button->state & BUTTON_SELECTED))
      {
        if (button->data)
          gxDrawTexture(button->data->texture[1],button->x-4,button->y-4,button->w+8,button->h+8,255);

        if (item)
        {
          if (item->texture)
          {
            gxDrawTexture(item->texture, item->x-4,item->y-4,item->w+8,item->h+8,255);
            FONT_writeCenter(item->text,18,button->x+4,item->x-4,button->y+(button->h - 36)/2+18,(GXColor)DARK_GREY);
          }
          else
          {
            FONT_writeCenter(item->text,18,item->x-4,item->x+item->w+4,button->y+(button->h-18)/2+18,(GXColor)DARK_GREY);
          }
        }
      }
      else
      {
        if (button->data)
          gxDrawTexture(button->data->texture[0],button->x,button->y,button->w, button->h,255);

        if (item)
        {
          if (item->texture)
          {
            gxDrawTexture(item->texture,item->x,item->y,item->w,item->h,255);
            FONT_writeCenter(item->text,16,button->x+8,item->x,button->y+(button->h - 32)/2+16,(GXColor)DARK_GREY);
          }
          else
          {
            FONT_writeCenter(item->text,16,item->x,item->x+item->w,button->y+(button->h - 16)/2+16,(GXColor)DARK_GREY);
          }
        }
      }
    }
  }

  /* draw arrow */
  for (i=0; i<2; i++)
  {
    button = menu->arrows[i];
    if (button)
    {
      if (button->state & BUTTON_VISIBLE)
      {
        if (menu->selected == (menu->max_buttons + i))
          gxDrawTexture(button->data->texture[1],button->x-2,button->y-2,button->w+4,button->h+4,255);
        else
          gxDrawTexture(button->data->texture[0],button->x,button->y,button->w, button->h,255);
      }
    }
  }

  /* left comment */
  item = menu->helpers[0];
  if (item)
  {
    if (item->data && strlen(item->comment))
    {
      gxDrawTexture(item->texture,item->x,item->y,item->w,item->h,255);
      FONT_write(item->comment,16,item->x+item->w+6,item->y+(item->h-16)/2 + 16,640,(GXColor)WHITE);
    }
  }

  /* right comment */
  item = menu->helpers[1];
  if (item)
  {
    if (item->data && strlen(item->comment))
    {
      gxDrawTexture(item->texture,item->x,item->y,item->w,item->h,255);
      FONT_alignRight(item->comment,16,item->x-6,item->y+(item->h-16)/2+16,(GXColor)WHITE);
    }
  }

  if (menu->cb)
    menu->cb();
}

/* Draw Menu with transitions effects */
void GUI_DrawMenuFX(gui_menu *menu, u8 speed, u8 out)
{
  int i,temp,xoffset,yoffset;
  int max_offset = 0;
  u8 item_alpha = 255;
  GXColor text_color = DARK_GREY;
  gui_item *item;
  gui_butn *button;
  gui_image *image;

  /* find maximal offset */
  for (i=0; i<menu->max_images; i++)
  {
    image = &menu->bg_images[i];

    if (image->state & IMAGE_SLIDE_LEFT)
    {
      temp = image->x + image->w;
      if (max_offset < temp)
        max_offset = temp;
    }
    else if (image->state & IMAGE_SLIDE_RIGHT)
    {
      temp = 640 - image->x;
      if (max_offset < temp)
        max_offset = temp;
    }

    if (image->state & IMAGE_SLIDE_TOP)
    {
      temp = image->y + image->h;
      if (max_offset < temp)
        max_offset = temp;
    }
    else if (image->state & IMAGE_SLIDE_BOTTOM)
    {
      temp = 480 - image->y;
      if (max_offset < temp)
        max_offset = temp;
    }
  }

  temp = max_offset;

  /* Alpha steps */
  int alpha = 0;
  int alpha_step = (255 * speed) / max_offset;
  if (out)
  {
    alpha = 255;
    alpha_step = -alpha_step;
  }

  /* Let's loop until final position has been reached */
  while (temp > 0)
  {
    /* background color */
    if (menu->screenshot)
    {
      gxClearScreen((GXColor)BLACK);
      if (alpha >= menu->screenshot)
        gxDrawScreenshot(menu->screenshot);
      else
        gxDrawScreenshot(255 - alpha);
    }
    else
    {
      gxClearScreen(bg_color);
    }

    /* background images */
    for (i=0; i<menu->max_images; i++)
    {
      image = &menu->bg_images[i];

      /* X offset */
      if (image->state & IMAGE_SLIDE_LEFT)
        xoffset = out ? (temp - max_offset) : (-temp);
      else if (image->state & IMAGE_SLIDE_RIGHT)
        xoffset = out ? (max_offset - temp) : (temp);
      else
        xoffset = 0;

      /* Y offset */
      if (image->state & IMAGE_SLIDE_TOP)
        yoffset = out ? (temp - max_offset) : (-temp);
      else if (image->state & IMAGE_SLIDE_BOTTOM)
        yoffset = out ? (max_offset - temp) : (temp);
      else
        yoffset = 0;

      /* draw image */
      if ((image->state & IMAGE_FADE) && ((out && (image->alpha > alpha)) || (!out && (image->alpha < alpha))))
      {
        /* FADE In-Out */
        if (image->state & IMAGE_VISIBLE)
        {
          if (image->state & IMAGE_REPEAT)
            gxDrawTextureRepeat(image->texture,image->x+xoffset,image->y+yoffset,image->w,image->h,alpha);
          else
          gxDrawTexture(image->texture,image->x+xoffset,image->y+yoffset,image->w,image->h,alpha);
        }
      }
      else
      {
        if (image->state & IMAGE_VISIBLE)
        {
          if (image->state & IMAGE_REPEAT)
            gxDrawTextureRepeat(image->texture,image->x+xoffset,image->y+yoffset,image->w,image->h,image->alpha);
          else
            gxDrawTexture(image->texture,image->x+xoffset,image->y+yoffset,image->w,image->h,image->alpha);
        }
      }
    }

    /* menu title */
    if ((menu->bg_images[2].state & IMAGE_SLIDE_TOP) || (menu->bg_images[3].state & IMAGE_SLIDE_TOP))
      FONT_write(menu->title, 22,10,out ? (56 + temp - max_offset) : (56 - temp),640,(GXColor)WHITE);
    else
      FONT_write(menu->title, 22,10,56,640,(GXColor)WHITE);

    /* draw buttons + items */
    for (i=0; i<menu->max_buttons; i++)
    {
      button = &menu->buttons[i];

      if (button->state & BUTTON_VISIBLE)
      {
        /* X offset */
        if (button->state & BUTTON_SLIDE_LEFT)
          xoffset = out ? (temp - max_offset) : (-temp);
        else if (button->state & BUTTON_SLIDE_RIGHT)
          xoffset = out ? (max_offset - temp) : (temp);
        else
          xoffset = 0;

        /* Y offset */
        if (button->state & BUTTON_SLIDE_TOP)
          yoffset = out ? (temp - max_offset) : (-temp);
        else if (button->state & BUTTON_SLIDE_BOTTOM)
          yoffset = out ? (max_offset - temp) : (temp);
        else
          yoffset = 0;

        /* Alpha transparency */
        if (button->state & BUTTON_FADE)
        {
          item_alpha = alpha;
          text_color.a = alpha;
        }
        else
        {
          item_alpha = 255;
          text_color.a = 255;
        }

        /* item select (text or image) */
        item = (menu->items) ? (&menu->items[menu->offset + i]) : NULL;

        /* draw button + items */ 
        if ((i == menu->selected) || (button->state & BUTTON_SELECTED))
        {
          if (button->data)
            gxDrawTexture(button->data->texture[1],button->x+xoffset-4,button->y+yoffset-4,button->w+8,button->h+8,item_alpha);

          if (item)
          {
            if (item->texture)
            {
              gxDrawTexture(item->texture, item->x+xoffset-4,item->y+yoffset-4,item->w+8,item->h+8,item_alpha);
              FONT_writeCenter(item->text,18,button->x+xoffset+4,item->x+xoffset-4,button->y+yoffset+(button->h - 36)/2+18,text_color);
            }
            else
            {
              FONT_writeCenter(item->text,18,item->x+xoffset+2,item->x+item->w+xoffset+2,button->y+yoffset+(button->h-18)/2+18,text_color);
            }
          }
        }
        else
        {
          if (button->data)
            gxDrawTexture(button->data->texture[0],button->x+xoffset,button->y+yoffset,button->w, button->h,item_alpha);

          if (item)
          {
            if (item->texture)
            {
              gxDrawTexture(item->texture,item->x+xoffset,item->y+yoffset,item->w,item->h,item_alpha);
              FONT_writeCenter(item->text,16,button->x+xoffset+8,item->x+xoffset,button->y+yoffset+(button->h - 32)/2+16,text_color);
            }
            else
            {
              FONT_writeCenter(item->text,16,item->x+xoffset,item->x+item->w+xoffset,button->y+yoffset+(button->h - 16)/2+16,text_color);
            }
          }
        }
      }
    }

    /* draw arrow */
    for (i=0; i<2; i++)
    {
      button = menu->arrows[i];
      if (button)
      {
        if (button->state & BUTTON_VISIBLE)
        {
          if (menu->selected == (menu->max_buttons + i))
            gxDrawTexture(button->data->texture[1],button->x-2,button->y-2,button->w+4,button->h+4,255);
          else
            gxDrawTexture(button->data->texture[0],button->x,button->y,button->w, button->h,255);
        }
      }
    }

    if (!(menu->bg_images[3].state & IMAGE_SLIDE_BOTTOM) && !(menu->bg_images[4].state & IMAGE_SLIDE_BOTTOM))
    {
      /* left comment */
      item = menu->helpers[0];
      if (item)
      {
        if (item->data && strlen(item->comment))
        {
          gxDrawTexture(item->texture,item->x,item->y,item->w,item->h,255);
          FONT_write(item->comment,16,item->x+item->w+6,item->y+(item->h-16)/2 + 16,640,(GXColor)WHITE);
        }
      }

      /* right comment */
      item = menu->helpers[1];
      if (item)
      {
        if (item->data && strlen(item->comment))
        {
          gxDrawTexture(item->texture,item->x,item->y,item->w,item->h,255);
          FONT_alignRight(item->comment,16,item->x-6,item->y+(item->h-16)/2+16,(GXColor)WHITE);
        }
      }
    }

    if (menu->cb)
      menu->cb();

    /* update offset */
    temp -= speed;

    /* update alpha */
    alpha += alpha_step;
    if (alpha > 255)
      alpha = 255;
    else if (alpha < 0)
      alpha = 0;


    /* copy EFB to XFB */
    gxSetScreen();
  }

  /* final position */
  if (!out) 
  {
    GUI_DrawMenu(menu);
    gxSetScreen();
  }
  else if (menu->screenshot)
  {
    gxClearScreen((GXColor)BLACK);
    gxDrawScreenshot(255);
    gxSetScreen();
  }
}

/* Basic menu title slide effect */
void GUI_SlideMenuTitle(gui_menu *m, int title_offset)
{
#ifdef HW_RVL
  gui_butn *button;
  int i,x,y;
#endif

  char title[64];
  strcpy(title,m->title);

  while (title_offset > 0)
  {
    /* update title */
    strcpy(m->title,title+title_offset);
    m->title[strlen(title)-title_offset-1] = 0;

    /* draw menu */
    GUI_DrawMenu(m);

#ifdef HW_RVL
    /* keep pointer active */
    if (m_input.ir.valid)
    {
      /* get cursor position */
      x = m_input.ir.x;
      y = m_input.ir.y;

      /* draw wiimote pointer */
      gxDrawTextureRotate(w_pointer, x-w_pointer->width/2, y-w_pointer->height/2, w_pointer->width, w_pointer->height,m_input.ir.angle,255);

      /* check for valid buttons */
      m->selected = m->max_buttons + 2;
      for (i=0; i<m->max_buttons; i++)
      {
        button = &m->buttons[i];
        if ((button->state & BUTTON_ACTIVE)&&(x>=button->x)&&(x<=(button->x+button->w))&&(y>=button->y)&&(y<=(button->y+button->h)))
        {
          m->selected = i;
          break;
        }
      }

      for (i=0; i<2; i++)
      {
        button = m->arrows[i];
        if (button)
        {
          if (button->state & BUTTON_VISIBLE)
          {
            if ((x<=(button->x+button->w))&&(y>=button->y)&&(y<=(button->y+button->h)))
            {
              m->selected = m->max_buttons + i;
              break;
            }
          }
        }
      }
    }
#endif
    gxSetScreen();
    usleep(6000);
    title_offset--;
  }
  strcpy(m->title,title);
}

/* Update current menu */
int GUI_UpdateMenu(gui_menu *menu)
{
  u16 p;
  int ret = 0;
  int selected = menu->selected;
  int max_items = menu->max_items;
  int max_buttons = menu->max_buttons;
  gui_butn *button;

#ifdef HW_RVL
  if (Shutdown)
  {
    GUI_DeleteMenu(menu);
    GUI_FadeOut();
    shutdown();
    SYS_ResetSystem(SYS_POWEROFF, 0, 0);
  }
  else if (m_input.ir.valid)
  {
    /* get cursor position */
    int x = m_input.ir.x;
    int y = m_input.ir.y;

    /* draw wiimote pointer */
    gxDrawTextureRotate(w_pointer, x-w_pointer->width/2, y-w_pointer->height/2, w_pointer->width, w_pointer->height,m_input.ir.angle,255);

    /* check for valid buttons */
    selected = max_buttons + 2;
    int i;
    for (i=0; i<max_buttons; i++)
    {
      button = &menu->buttons[i];
      if ((button->state & BUTTON_ACTIVE) && (button->state & BUTTON_VISIBLE))
      {
        if((x>=button->x)&&(x<=(button->x+button->w))&&(y>=button->y)&&(y<=(button->y+button->h)))
        {
          selected = i;
          break;
        }
      }
    }

    for (i=0; i<2; i++)
    {
      button = menu->arrows[i];
      if (button)
      {
        if ((button->state & BUTTON_ACTIVE) && (button->state & BUTTON_VISIBLE))
        {
          if ((x<=(button->x+button->w))&&(y>=button->y)&&(y<=(button->y+button->h)))
          {
            selected = max_buttons + i;
            break;
          }
        }
      }
    }
  }
  else
  {
    /* reinitialize selection */
    if (selected >= menu->max_buttons)
    {
      selected = 0;
      while ((selected < (menu->max_buttons + 2)) && 
             (!(menu->buttons[selected].state & BUTTON_ACTIVE) || 
              !(menu->buttons[selected].state & BUTTON_VISIBLE)))
        selected++;
    }
  }
#endif

  /* update screen */
  gxSetScreen();

  /* update menu */
  p = m_input.keys;

  if (selected < max_buttons)
  {
    button = &menu->buttons[selected];
    if (p & PAD_BUTTON_UP)
    {
      selected -= button->shift[0];
      if (selected < 0)
      {
        selected = 0;
        if (menu->offset)
          menu->offset --;
      }
    }
    else if (p & PAD_BUTTON_DOWN)
    {
      selected += button->shift[1];
      if (selected >= max_buttons)
      {
        selected = max_buttons - 1;
        if ((menu->offset + selected) < (max_items - 1))
          menu->offset ++;
      }
    }
    else if (p & PAD_BUTTON_LEFT)
    {
      selected -= button->shift[2];
      if (selected < 0)
      {
        selected = 0;
        if (menu->offset)
          menu->offset --;
      }
    }
    else if (p & PAD_BUTTON_RIGHT)
    {
      selected += button->shift[3];
      if (selected >= max_buttons)
      {
        selected = max_buttons - 1;
        if ((menu->offset + selected) < (max_items - 1))
          menu->offset ++;
      }
    }
  }

  if (p & PAD_BUTTON_A)
  {
    if (selected < max_buttons)
      ret = 1; /* menu clicked */
    else if (selected == max_buttons)
      menu->offset --; /* up arrow */
    else if (selected == (max_buttons+1))
      menu->offset ++; /* down arrow */
  }
  else if ((p & PAD_BUTTON_B) || (p & PAD_TRIGGER_Z))
  {
    /* quit menu */
    ret = -1;
  }

  /* selected item has changed ? */
  if (menu->selected != selected)
  {
    if (selected < max_buttons)
    {
      /* sound fx */
      button = &menu->buttons[selected];
      if (button->state & BUTTON_OVER_SFX)
      {
        ASND_SetVoice(ASND_GetFirstUnusedVoice(),VOICE_MONO_16BIT,22050,0,(u8 *)button_over_pcm,button_over_pcm_size,
                      ((int)config.sfx_volume * 255) / 100,((int)config.sfx_volume * 255) / 100,NULL);
      }
    }
    else if (selected < (max_buttons + 2))
    {
      /* sound fx */
      button = menu->arrows[selected-max_buttons];
      if (button->state & BUTTON_OVER_SFX)
      {
        ASND_SetVoice(ASND_GetFirstUnusedVoice(),VOICE_MONO_16BIT,22050,0,(u8 *)button_over_pcm,button_over_pcm_size,
                      ((int)config.sfx_volume * 255) / 100,((int)config.sfx_volume * 255) / 100,NULL);
      }
    }

    /* update selection */
    menu->selected = selected;
  }

  /* update helper comment */
  if (menu->helpers[1])
  {
    if ((menu->offset + selected) < max_items)
    {
      gui_item *item = &menu->items[menu->offset + selected];
      strcpy(menu->helpers[1]->comment,item->comment);
    }
    else
    {
      strcpy(menu->helpers[1]->comment,"");
    }
  }

  if (ret > 0)
  {
    if (selected < max_buttons)
    {
      /* sound fx */
      button = &menu->buttons[selected];
      if (button->state & BUTTON_SELECT_SFX)
      {
        ASND_SetVoice(ASND_GetFirstUnusedVoice(),VOICE_MONO_16BIT,22050,0,(u8 *)button_select_pcm,button_select_pcm_size,
                      ((int)config.sfx_volume * 255) / 100,((int)config.sfx_volume * 255) / 100,NULL);
      }
    }
  }

  return ret;
}

/* Generic routine to render & update menus */
int GUI_RunMenu(gui_menu *menu)
{
  int update = 0;

  /* update menu */
  while (!update)
  {
    GUI_DrawMenu(menu);
    update = GUI_UpdateMenu(menu);

    /* update arrows buttons status (items list) */
    if (menu->arrows[0])
    {
      if (menu->offset > 0)
        menu->arrows[0]->state |= BUTTON_VISIBLE;
      else
        menu->arrows[0]->state &= ~BUTTON_VISIBLE;
    }

    if (menu->arrows[1])
    {
      if ((menu->offset + menu->max_buttons) < menu->max_items)
        menu->arrows[1]->state |= BUTTON_VISIBLE;
      else
        menu->arrows[1]->state &= ~BUTTON_VISIBLE;
    }
  }

  if (update == 2)
    return (-2-menu->offset-menu->selected);
  else if (update == 1)
    return (menu->offset + menu->selected);
  else
    return -1;
 }

/* Text Window */
void GUI_TextWindow(gui_menu *parent, char *title, char items[][64], u8 nb_items, u8 fontsize)
{
  int i, quit = 0;

#ifdef HW_RVL
  int x,y;
#endif

  /* initialize window */
  gx_texture *window = gxTextureOpenPNG(Frame_s1_png,0);
  gx_texture *top = gxTextureOpenPNG(Frame_s1_title_png,0);

  /* window position */
  int xwindow = (640 - window->width) /2;
  int ywindow = (480 - window->height)/2;

  /* text position */
  int ypos  = ywindow + top->height + (window->height - top->height - fontsize*nb_items) / 2 + fontsize/2;

  /* disable helper comment */
  const u8 *data = NULL;
  if (parent->helpers[1])
  {
    data = parent->helpers[1]->data;
    parent->helpers[1]->data = NULL;
  }

  /* slide in */
  int yoffset = ywindow + window->height;
  while (yoffset > 0)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,230);
    gxDrawTexture(top,xwindow,ywindow-yoffset,top->width,top->height,255);

    /* draw title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20-yoffset,(GXColor)WHITE);

    /* draw  text */
    for (i=0; i<nb_items; i++)
      FONT_writeCenter(items[i],fontsize,xwindow,xwindow+window->width,ypos+i*fontsize-yoffset,(GXColor)WHITE);

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset -= 60;
  }

  /* draw menu + text window */
  while (quit == 0)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow,window->width,window->height,230);
    gxDrawTexture(top,xwindow,ywindow,top->width,top->height,255);

    /* draw title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20,(GXColor)WHITE);

    /* draw text */
    for (i=0; i<nb_items; i++)
    {
      FONT_writeCenter(items[i],fontsize,xwindow,xwindow+window->width,ypos+i*fontsize,(GXColor)WHITE);
    }

#ifdef HW_RVL
    if (Shutdown)
    {
      gxTextureClose(&window);
      gxTextureClose(&top);
      gxTextureClose(&w_pointer);
      GUI_DeleteMenu(parent);
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
    }
#endif

    /* update screen */
    gxSetScreen();

    /* wait for exit buttons */
    if (m_input.keys)
      quit = 1;
  }

  /* reset initial vertical offset */

  /* slide out */
  yoffset = 0;
  while (yoffset < (ywindow + window->height))
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,230);
    gxDrawTexture(top,xwindow,ywindow-yoffset,top->width,top->height,255);

    /* draw title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20-yoffset,(GXColor)WHITE);

    /* draw  text */
    for (i=0; i<nb_items; i++)
      FONT_writeCenter(items[i],fontsize,xwindow,xwindow+window->width,ypos+i*fontsize-yoffset,(GXColor)WHITE);

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset += 60;
  }

  /* restore helper comment */
  if (parent->helpers[1])
    parent->helpers[1]->data = data;

  /* final position */
  GUI_DrawMenu(parent);
  gxSetScreen();

  /* close textures */
  gxTextureClose(&window);
  gxTextureClose(&top);
}

/* Option Window (returns selected item) */
int GUI_OptionWindow(gui_menu *parent, char *title, char *items[], u8 nb_items)
{
  int i, ret, quit = 0;
  int old, selected = 0;
  s16 p;
  butn_data button;

#ifdef HW_RVL
  int x,y;
#endif

  /* initialize buttons data */
  button.texture[0] = gxTextureOpenPNG(Button_text_png,0);
  button.texture[1] = gxTextureOpenPNG(Button_text_over_png,0);

  /* initialize texture window */
  gx_texture *window = gxTextureOpenPNG(Frame_s1_png,0);
  gx_texture *top = gxTextureOpenPNG(Frame_s1_title_png,0);

  /* get initial positions */
  int w = button.texture[0]->width;
  int h = button.texture[0]->height;
  int xwindow = (640 - window->width)/2;
  int ywindow = (480 - window->height)/2;
  int xpos = xwindow + (window->width - w)/2;
  int ypos = (window->height - top->height - (h*nb_items) - (nb_items-1)*20)/2;
  ypos = ypos + ywindow + top->height;

  /* disable helper comment */
  const u8 *data = NULL;
  if (parent->helpers[1])
  {
    data = parent->helpers[1]->data;
    parent->helpers[1]->data = 0;
  }

  /* slide in */
  int yoffset = ywindow + window->height;
  while (yoffset > 0)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,230);
    gxDrawTexture(top,xwindow,ywindow-yoffset,top->width,top->height,255);

    /* draw title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20-yoffset,(GXColor)WHITE);

    /* draw buttons + text */
    for (i=0; i<nb_items; i++)
    {
      gxDrawTexture(button.texture[0],xpos,ypos+i*(20 + h)-yoffset,w,h,255);
      FONT_writeCenter(items[i],18,xpos,xpos+w,ypos+i*(20 + h)+(h + 18)/2- yoffset,(GXColor)DARK_GREY);
    }

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset -= 60;
  }

  /* draw menu  */
  while (quit == 0)
  {
    /* draw parent menu (should have been initialized first) */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow,window->width,window->height,230);
    gxDrawTexture(top,xwindow,ywindow,top->width,top->height,255);

    /* draw title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20,(GXColor)WHITE);

    /* draw buttons + text */
    for (i=0; i<nb_items; i++)
    {
      if (i==selected)
      {
        gxDrawTexture(button.texture[1],xpos-4,ypos+i*(20+h)-4,w+8,h+8,255);
        FONT_writeCenter(items[i],22,xpos,xpos+w,ypos+i*(20+h)+(h+22)/2,(GXColor)DARK_GREY);
      }
      else
      {
        gxDrawTexture(button.texture[0],xpos,ypos+i*(20 + h),w,h,255);
        FONT_writeCenter(items[i],18,xpos,xpos+w,ypos+i*(20+h)+(h+18)/2,(GXColor)DARK_GREY);
      }
    }

    old = selected;
    p = m_input.keys;

#ifdef HW_RVL
    if (Shutdown)
    {
      gxTextureClose(&window);
      gxTextureClose(&top);
      gxTextureClose(&button.texture[0]);
      gxTextureClose(&button.texture[1]);
      gxTextureClose(&w_pointer);
      GUI_DeleteMenu(parent);
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

      /* check for valid buttons */
      selected = -1;
      for (i=0; i<nb_items; i++)
      {
        if ((x>=xpos)&&(x<=(xpos+w))&&(y>=ypos+i*(20 + h))&&(y<=(ypos+i*(20+h)+h)))
        {
          selected = i;
          break;
        }
      }
    }
    else
    {
      /* reinitialize selection */
      if (selected == -1)
        selected = 0;
    }
#endif

    /* update screen */
    gxSetScreen();

    /* update selection */
    if (p & PAD_BUTTON_UP)
    {
      if (selected > 0)
        selected --;
    }
    else if (p & PAD_BUTTON_DOWN)
    {
      if (selected < (nb_items -1))
        selected ++;
    }

    /* sound fx */
    if (selected != old)
    {
      if (selected >= 0)
      {
        ASND_SetVoice(ASND_GetFirstUnusedVoice(),VOICE_MONO_16BIT,22050,0,(u8 *)button_over_pcm,button_over_pcm_size,
                      ((int)config.sfx_volume * 255) / 100,((int)config.sfx_volume * 255) / 100,NULL);
      }
    }

    if (p & PAD_BUTTON_A)
    {
      if (selected >= 0)
      {
        quit = 1;
        ret = selected;
      }
    }
    else if (p & PAD_BUTTON_B)
    {
      quit = 1;
      ret = -1;
    }
  }

  /* slide out */
  yoffset = 0;
  while (yoffset < (ywindow + window->height))
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window + header */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,230);
    gxDrawTexture(top,xwindow,ywindow-yoffset,top->width,top->height,255);

    /* draw title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20-yoffset,(GXColor)WHITE);

    /* draw buttons + text */
    for (i=0; i<nb_items; i++)
    {
      gxDrawTexture(button.texture[0],xpos,ypos+i*(20+h)-yoffset,w,h,255);
      FONT_writeCenter(items[i],18,xpos,xpos+w,ypos+i*(20+h)+(h+18)/2-yoffset,(GXColor)WHITE);
    }

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset += 60;
  }

  /* restore helper comment */
  if (parent->helpers[1])
    parent->helpers[1]->data = data;

  /* final position */
  GUI_DrawMenu(parent);
  gxSetScreen();

  /* close textures */
  gxTextureClose(&window);
  gxTextureClose(&top);
  gxTextureClose(&button.texture[0]);
  gxTextureClose(&button.texture[1]);

  return ret;
}

/* Option Box */
void GUI_OptionBox(gui_menu *parent, optioncallback cb, char *title, void *option, float step, float min, float max, u8 type)
{
  gx_texture *arrow[2];
  arrow[0] = gxTextureOpenPNG(Button_arrow_png,0);
  arrow[1] = gxTextureOpenPNG(Button_arrow_over_png,0);
  gx_texture *window = gxTextureOpenPNG(Frame_s2_png,0);
  gx_texture *top = gxTextureOpenPNG(Frame_s2_title_png,0);

  /* window position */
  int xwindow = 166;
  int ywindow = 160;

  /* arrows position */
  int xleft   = 206;
  int xright  = 392;
  int yleft   = 238;
  int yright  = 238;

  /* disable action button helper */
  if (parent->helpers[1])
    parent->helpers[1]->data = 0;

  /* slide in */
  char msg[16];
  int yoffset = ywindow + window->height;
  while (yoffset > 0)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,225);
    gxDrawTexture(top,xwindow,ywindow-yoffset,top->width,top->height,255);

    /* display title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20-yoffset,(GXColor)WHITE);

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset -= 60;
  }

  /* display option box */
  int quit = 0;
  int modified = 0;
  int selected = -1;
  s16 p;
#ifdef HW_RVL
  int x,y;
#endif

  while (!quit)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow,window->width,window->height,225);
    gxDrawTexture(top,xwindow,ywindow,top->width,top->height,255);

    /* display title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20,(GXColor)WHITE);

    /* option type */
    if (type)
    {
      /* integer type */
      if (*(s16 *)option < 0)
        sprintf(msg,"-%d",abs(*(s16 *)option));
      else
        sprintf(msg,"%d",abs(*(s16 *)option));
    }
    else
    {
      /* float type */
      if (*(float *)option < 0.0)
        sprintf(msg,"-%1.2f",fabs(*(float *)option));
      else
        sprintf(msg,"%1.2f",fabs(*(float *)option));
    }

    /* draw option text */
    FONT_writeCenter(msg,24,xwindow,xwindow+window->width,272,(GXColor)WHITE);

    /* update inputs */
    p = m_input.keys;

    /* draw buttons */
    if (selected < 0)
    {
      /* nothing selected */
      gxDrawTexture(arrow[0],xleft,yleft,arrow[0]->width,arrow[0]->height,255);
      gxDrawTextureRotate(arrow[0],xright,yright,arrow[0]->width,arrow[0]->height,180.0,255);
    }

#ifdef HW_RVL
    else if (selected)
    {
      /* right button selected */
      gxDrawTexture(arrow[0],xleft,yleft,arrow[0]->width,arrow[0]->height,255);
      gxDrawTextureRotate(arrow[1],xright-4,yright-4,arrow[1]->width+8,arrow[1]->height+8,180.0,255);
    }
    else
    {
      /* left button selected */
      gxDrawTexture(arrow[1],xleft-4,yleft-4,arrow[1]->width+8,arrow[1]->height+8,255);
      gxDrawTextureRotate(arrow[0],xright,yright,arrow[0]->width,arrow[0]->height,180.0,255);
    }

    selected = -1;
    if (Shutdown)
    {
      gxTextureClose(&arrow[0]);
      gxTextureClose(&arrow[1]);
      gxTextureClose(&window);
      gxTextureClose(&top);
      gxTextureClose(&w_pointer);
      GUI_DeleteMenu(parent);
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

      /* check for valid buttons */
      if ((x>=xleft)&&(x<=(xleft+arrow[0]->width))&&(y>=yleft)&&(y<=(yleft+arrow[0]->height)))
      {
        selected = 0;
        if (p & PAD_BUTTON_A)
          p |= PAD_BUTTON_LEFT;
      }
      else if ((x>=xright)&&(x<=(xright+arrow[0]->width))&&(y>=yright)&&(y<=(yright+arrow[0]->height)))
      {
        selected = 1;
        if (p & PAD_BUTTON_A)
          p |= PAD_BUTTON_RIGHT;
      }
    }
#endif

    /* update screen */
    gxSetScreen();

    /* check input */
    if (p&PAD_BUTTON_LEFT)
    {
      /* decrement option value */
      if (type)
      {
        /* integer type */
        *(s16 *)option -= (s16)step;
        if (*(s16 *)option < (s16)min)
          *(s16 *)option = (s16)max;
      }
      else
      {
        /* float type */
        *(float *)option -= step;
        if (*(float *)option < min)
          *(float *)option = max;
      }

      modified = 1;
    }
    else if (p&PAD_BUTTON_RIGHT)
    {
      /* increment option value */
      if (type)
      {
        /* integer type */
        *(s16 *)option += (s16)step;
        if (*(s16 *)option > (s16)max)
          *(s16 *)option = (s16)min;
      }
      else
      {
        /* float type */
        *(float *)option += step;
        if (*(float *)option > max)
          *(float *)option = min;
      }

      modified = 1;
    }
    else if (p & PAD_BUTTON_B)
    {
      quit = 1;
    }

    if (modified)
    {
      modified = 0;

      /* play sound effect */
      ASND_SetVoice(ASND_GetFirstUnusedVoice(),VOICE_MONO_16BIT,22050,0,(u8 *)button_over_pcm,button_over_pcm_size,
                    ((int)config.sfx_volume * 255) / 100,((int)config.sfx_volume * 255) / 100,NULL);

      /* option callback */
      if (cb)
        cb();
    }
  }

  /* slide out */
  yoffset = 0; ;
  while (yoffset < (ywindow + window->height))
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,225);
    gxDrawTexture(top,xwindow,ywindow-yoffset,top->width,top->height,255);

    /* display title */
    FONT_writeCenter(title,20,xwindow,xwindow+window->width,ywindow+(top->height-20)/2+20-yoffset,(GXColor)WHITE);

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset += 60;
  }

  /* restore action button helper */
  if (parent->helpers[1])
    parent->helpers[1]->data = Key_A_png;

  /* final position */
  GUI_DrawMenu(parent);
  gxSetScreen();

  /* close textures */
  gxTextureClose(&arrow[0]);
  gxTextureClose(&arrow[1]);
  gxTextureClose(&window);
  gxTextureClose(&top);
}

/* Option Box with two parameters */
void GUI_OptionBox2(gui_menu *parent, char *text_1, char *text_2, s16 *option_1, s16 *option_2, s16 step, s16 min, s16 max)
{
  gx_texture *arrow[2];
  arrow[0] = gxTextureOpenPNG(Button_arrow_png,0);
  arrow[1] = gxTextureOpenPNG(Button_arrow_over_png,0);
  gx_texture *window = gxTextureOpenPNG(Frame_s2_png,0);

  /* window position */
  int xwindow = 166;
  int ywindow = 160;

  /* arrows position */
  int arrow_pos[4][2] =
  {
    {144,218},
    {452,218},
    {298,138},
    {298,298}
  };

  /* disable action button helper */
  if (parent->helpers[1])
    parent->helpers[1]->data = 0;

  /* slide in */
  char msg[16];
  int yoffset = ywindow + window->height;
  while (yoffset > 0)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,225);

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset -= 60;
  }

  /* display option box */
  int quit = 0;
  int modified = 0;
  s16 p;
#ifdef HW_RVL
  int selected = -1;
  int i,x,y;
#endif

  while (!quit)
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow,window->width,window->height,225);

    /* draw options text */
    if (*option_1 < 0)
      sprintf(msg,"%s: -%02d",text_1,abs(*option_1));
    else
      sprintf(msg,"%s: +%02d",text_1,abs(*option_1));
    FONT_writeCenter(msg,24,xwindow,xwindow+window->width,240,(GXColor)WHITE);
    if (*option_2 < 0)
      sprintf(msg,"%s: -%02d",text_2,abs(*option_2));
    else
      sprintf(msg,"%s: +%02d",text_2,abs(*option_2));
    FONT_writeCenter(msg,24,xwindow,xwindow+window->width,264,(GXColor)WHITE);

    /* update inputs */
    p = m_input.keys;

    /* draw buttons */
#ifdef HW_RVL
    switch (selected)
    {
      case 0:   /* left button */
        gxDrawTexture(arrow[1],arrow_pos[0][0]-4,arrow_pos[0][1]-4,arrow[0]->width+8,arrow[0]->height+8,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[1][0],arrow_pos[1][1],arrow[0]->width,arrow[0]->height,180.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[2][0],arrow_pos[2][1],arrow[0]->width,arrow[0]->height,90.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[3][0],arrow_pos[3][1],arrow[0]->width,arrow[0]->height,270.0,255);
        if (p & PAD_BUTTON_A) p |= PAD_BUTTON_LEFT;
        break;

      case 1:   /* right button */
        gxDrawTexture(arrow[0],arrow_pos[0][0],arrow_pos[0][1],arrow[0]->width,arrow[0]->height,255);
        gxDrawTextureRotate(arrow[1],arrow_pos[1][0]-4,arrow_pos[1][1]-4,arrow[0]->width+8,arrow[0]->height+8,180.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[2][0],arrow_pos[2][1],arrow[0]->width,arrow[0]->height,90.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[3][0],arrow_pos[3][1],arrow[0]->width,arrow[0]->height,270.0,255);
        if (p & PAD_BUTTON_A) p |= PAD_BUTTON_RIGHT;
        break;

      case 2:   /* up button */
        gxDrawTexture(arrow[0],arrow_pos[0][0],arrow_pos[0][1],arrow[0]->width,arrow[0]->height,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[1][0],arrow_pos[1][1],arrow[0]->width,arrow[0]->height,180.0,255);
        gxDrawTextureRotate(arrow[1],arrow_pos[2][0]-4,arrow_pos[2][1]-4,arrow[0]->width+8,arrow[0]->height+8,90.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[3][0],arrow_pos[3][1],arrow[0]->width,arrow[0]->height,270.0,255);
        if (p & PAD_BUTTON_A) p |= PAD_BUTTON_UP;
        break;

      case 3:   /* down button */
        gxDrawTexture(arrow[0],arrow_pos[0][0],arrow_pos[0][1],arrow[0]->width,arrow[0]->height,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[1][0],arrow_pos[1][1],arrow[0]->width,arrow[0]->height,180.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[2][0],arrow_pos[2][1],arrow[0]->width,arrow[0]->height,90.0,255);
        gxDrawTextureRotate(arrow[1],arrow_pos[3][0]-4,arrow_pos[3][1]-4,arrow[0]->width+8,arrow[0]->height+8,270.0,255);
        if (p & PAD_BUTTON_A) p |= PAD_BUTTON_DOWN;
        break;

      default:  /* nothing selected */
        gxDrawTexture(arrow[0],arrow_pos[0][0],arrow_pos[0][1],arrow[0]->width,arrow[0]->height,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[1][0],arrow_pos[1][1],arrow[0]->width,arrow[0]->height,180.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[2][0],arrow_pos[2][1],arrow[0]->width,arrow[0]->height,90.0,255);
        gxDrawTextureRotate(arrow[0],arrow_pos[3][0],arrow_pos[3][1],arrow[0]->width,arrow[0]->height,270.0,255);
        break;
    }

    if (Shutdown)
    {
      gxTextureClose(&arrow[0]);
      gxTextureClose(&arrow[1]);
      gxTextureClose(&window);
      gxTextureClose(&w_pointer);
      GUI_DeleteMenu(parent);
      GUI_FadeOut();
      shutdown();
      SYS_ResetSystem(SYS_POWEROFF, 0, 0);
    }

    /* update selection */
    selected = -1;
    if (m_input.ir.valid)
    {
      /* get cursor position */
      x = m_input.ir.x;
      y = m_input.ir.y;

      /* draw wiimote pointer */
      gxDrawTextureRotate(w_pointer, x-w_pointer->width/2, y-w_pointer->height/2, w_pointer->width, w_pointer->height,m_input.ir.angle,255);

      /* check for valid buttons */
      for (i=0; i<4; i++)
      {
        if ((x>=arrow_pos[i][0])&&(x<=(arrow_pos[i][0]+arrow[0]->width))&&(y>=arrow_pos[i][1])&&(y<=(arrow_pos[i][1]+arrow[0]->height)))
          selected = i;
      }
    }
#else
    gxDrawTexture(arrow[0],arrow_pos[0][0],arrow_pos[0][1],arrow[0]->width,arrow[0]->height,255);
    gxDrawTextureRotate(arrow[0],arrow_pos[1][0],arrow_pos[1][1],arrow[0]->width,arrow[0]->height,180.0,255);
    gxDrawTextureRotate(arrow[0],arrow_pos[2][0],arrow_pos[2][1],arrow[0]->width,arrow[0]->height,90.0,255);
    gxDrawTextureRotate(arrow[0],arrow_pos[3][0],arrow_pos[3][1],arrow[0]->width,arrow[0]->height,270.0,255);
#endif

    /* update screen */
    gxSetScreen();

    if (p&PAD_BUTTON_LEFT)
    {
      /* decrement option 1 value */
      *option_1 -= step;
      if (*option_1 < min)
        *option_1 = max;
      modified = 1;
    }
    else if (p&PAD_BUTTON_RIGHT)
    {
      /* decrement option 1 value */
      *option_1 += step;
      if (*option_1 > max)
        *option_1 = min;
      modified = 1;
    }
    else if (p&PAD_BUTTON_UP)
    {
      /* decrement option 2 value */
      *option_2 -= step;
      if (*option_2 < min)
        *option_2 = max;
      modified = 1;
    }
    else if (p&PAD_BUTTON_DOWN)
    {
      /* increment option 2 value */
      *option_2 += step;
      if (*option_2 > max)
        *option_2 = min;
      modified = 1;
    }
    else if (p & PAD_BUTTON_B)
    {
      quit = 1;
    }

    if (modified)
    {
      modified = 0;
      /* play sound effect */
      ASND_SetVoice(ASND_GetFirstUnusedVoice(),VOICE_MONO_16BIT,22050,0,(u8 *)button_over_pcm,button_over_pcm_size,
                    ((int)config.sfx_volume * 255) / 100,((int)config.sfx_volume * 255) / 100,NULL);
    }
  }

  /* slide out */
  yoffset = 0; ;
  while (yoffset < (ywindow + window->height))
  {
    /* draw parent menu */
    GUI_DrawMenu(parent);

    /* draw window */
    gxDrawTexture(window,xwindow,ywindow-yoffset,window->width,window->height,225);

    /* update display */
    gxSetScreen();

    /* slide speed */
    yoffset += 60;
  }

  /* restore action button helper */
  if (parent->helpers[1])
    parent->helpers[1]->data = Key_A_png;

  /* final position */
  GUI_DrawMenu(parent);
  gxSetScreen();

  /* close textures */
  gxTextureClose(&arrow[0]);
  gxTextureClose(&arrow[1]);
  gxTextureClose(&window);
}

/* Interactive Message Box */
/* Message Box displays a message until a specific action is completed */

/* Message Box LWP Thread */
static void *MsgBox_Thread(gui_message *message_box)
{
  while (message_box->refresh)
  {
    /* draw parent menu */
    if (message_box->parent)
    {
      GUI_DrawMenu(message_box->parent);
    }
    else
    {
      gxClearScreen(bg_color);
    }

    /* draw window */
    gxDrawTexture(message_box->window,166,160,message_box->window->width,message_box->window->height,230);
    gxDrawTexture(message_box->top,166,160,message_box->top->width,message_box->top->height,255);

    /* draw title */
    if (message_box->title)
      FONT_writeCenter(message_box->title,20,166,166+message_box->window->width,160+(message_box->top->height-20)/2+20,(GXColor)WHITE);

    /* draw box message */
    if (message_box->msg)
      FONT_writeCenter(message_box->msg,18,166,166+message_box->window->width,248,(GXColor)WHITE);

    /* draw throbber */
    if (message_box->throbber)
      gxDrawTextureRotate(message_box->throbber,166+(message_box->window->width-message_box->throbber->width)/2,160+message_box->window->height-message_box->throbber->height-20,message_box->throbber->width,message_box->throbber->height,(message_box->progress * 360.0) / 100.0, 255);

    /* draw exit message */
    if (message_box->buttonA)
    {
      FONT_writeCenter("Press    to continue.",18,166,166+message_box->window->width,248+22,(GXColor)WHITE);
      gxDrawTexture(message_box->buttonA, 166+116, 248+4+(18-message_box->buttonA->height)/2,message_box->buttonA->width, message_box->buttonA->height,255);
    }

    /* update display */
    gxSetScreen();

    /* update progression */
    message_box->progress++;
    if (message_box->progress > 100)
      message_box->progress = 0;
    usleep(10);
  }

  return 0;
}

/* update current Message Box */
void GUI_MsgBoxUpdate(char *title, char *msg)
{
  if (title)
    strncpy(message_box.title,title,64);
  if (msg)
    strncpy(message_box.msg,msg,64);
}

/* setup current Message Box */
void GUI_MsgBoxOpen(char *title, char *msg, bool throbber)
{
  if (SILENT)
    return;

  /* update text */
  GUI_MsgBoxUpdate(title,msg);

  /* ensure we are not already running */
  if (!message_box.refresh)
  {
    /* initialize default textures */
    message_box.window = gxTextureOpenPNG(Frame_s2_png,0);
    message_box.top = gxTextureOpenPNG(Frame_s2_title_png,0);
    if (throbber)
      message_box.throbber = gxTextureOpenPNG(Frame_throbber_png,0);

    /* window position */
    int xwindow = 166;
    int ywindow = 160;
    int ypos = 248;

    /* disable helper comments */
    if (message_box.parent)
    {
      if (message_box.parent->helpers[0])
        message_box.parent->helpers[0]->data = 0;
      if (message_box.parent->helpers[1])
        message_box.parent->helpers[1]->data = 0;
    }

    /* slide in */
    int yoffset = ywindow + message_box.window->height;
    while (yoffset > 0)
    {
      /* draw parent menu */
      if (message_box.parent)
      {
        GUI_DrawMenu(message_box.parent);
      }
      else
      {
        gxClearScreen(bg_color);
      }

      /* draw window */
      gxDrawTexture(message_box.window,xwindow,ywindow-yoffset,message_box.window->width,message_box.window->height,230);
      gxDrawTexture(message_box.top,xwindow,ywindow-yoffset,message_box.top->width,message_box.top->height,255);

      /* draw title */
      if (title)
        FONT_writeCenter(title,20,xwindow,xwindow+message_box.window->width,ywindow+(message_box.top->height-20)/2+20-yoffset,(GXColor)WHITE);

      /* draw box message */
      if (msg)
        FONT_writeCenter(msg,18,xwindow,xwindow+message_box.window->width,ypos-yoffset,(GXColor)WHITE);

      /* update display */
      gxSetScreen();

      /* slide speed */
      yoffset -= 60;
    }

    /* create LWP thread for MessageBox refresh */
    message_box.refresh = TRUE;
    LWP_CreateThread (&msgboxthread, (void *)MsgBox_Thread, &message_box, NULL, 0, 70);
  }
}

/* Close current messagebox */
void GUI_MsgBoxClose(void)
{
  if (message_box.refresh)
  {
    /* suspend MessageBox refresh */
    message_box.refresh = FALSE;
    LWP_JoinThread(msgboxthread, NULL);

    /* window position */
    int xwindow = 166;
    int ywindow = 160;
    int ypos = 248;

    /* slide out */
    int yoffset = 0;
    while (yoffset < (ywindow + message_box.window->height))
    {
      /* draw parent menu */
      if (message_box.parent)
      {
        GUI_DrawMenu(message_box.parent);
      }
      else
      {
        gxClearScreen(bg_color);
      }

      /* draw window */
      gxDrawTexture(message_box.window,xwindow,ywindow-yoffset,message_box.window->width,message_box.window->height,230);
      gxDrawTexture(message_box.top,xwindow,ywindow-yoffset,message_box.top->width,message_box.top->height,255);

      /* draw title */
      if (message_box.title)
        FONT_writeCenter(message_box.title,20,xwindow,xwindow+message_box.window->width,ywindow+(message_box.top->height-20)/2+20-yoffset,(GXColor)WHITE);

      /* draw text */
      if (message_box.msg)
        FONT_writeCenter(message_box.msg,18,xwindow,xwindow+message_box.window->width,ypos-yoffset,(GXColor)WHITE);

      /* update display */
      gxSetScreen();

      /* slide speed */
      yoffset += 60;
    }

    if (message_box.parent)
    {
      /* restore helper comment */
      if (message_box.parent->helpers[0])
        message_box.parent->helpers[0]->data = Key_B_png;
      if (message_box.parent->helpers[1])
        message_box.parent->helpers[1]->data = Key_A_png;
    
      /* final position */
      GUI_DrawMenu(message_box.parent);
    }
    else
    {
      gxClearScreen(bg_color);
    }
    
    gxSetScreen();

    /* clear all textures */
    gxTextureClose(&message_box.window);
    gxTextureClose(&message_box.top);
    gxTextureClose(&message_box.buttonA);
    gxTextureClose(&message_box.throbber);
  }
}

void GUI_WaitPrompt(char *title, char *msg)
{
  if (SILENT)
    return;

  /* clear unused texture */
  gxTextureClose(&message_box.throbber);

  /* open or update message box */
  GUI_MsgBoxOpen(title, msg, 0);

  /* allocate texture */
  message_box.buttonA = gxTextureOpenPNG(Key_A_png,0);

  /* wait for button A */
  while (m_input.keys & PAD_BUTTON_A)
    VIDEO_WaitVSync();
  while (!(m_input.keys & PAD_BUTTON_A))
    VIDEO_WaitVSync();

  /* always close message box */
  GUI_MsgBoxClose();
}

/* Basic Fading */
void GUI_FadeOut()
{
  int alpha = 0;
  while (alpha < 256)
  {
    gxDrawRectangle(0, 0, 640, 480, alpha, (GXColor)BLACK);
    gxSetScreen();
    alpha +=3;
  }
}

/* Return background color */
GXColor *GUI_GetBgColor(void)
{
  return &bg_color;
}

/* Select background color */
void GUI_SetBgColor(u8 color)
{
  if (color < BG_COLOR_MAX)
  {
    bg_color.r = bg_colors[color].r;
    bg_color.g = bg_colors[color].g;
    bg_color.b = bg_colors[color].b;
    bg_color.a = bg_colors[color].a;
  }
}

