#include <math.h>
#include "main.h"
#include "console.h"


MDFNConsole::MDFNConsole(bool setshellstyle, bool setsmallfont)
{
 prompt_visible = TRUE;
 shellstyle = setshellstyle;
 Scrolled = 0;
 SmallFont = setsmallfont;
}

MDFNConsole::~MDFNConsole()
{
 kb_cursor_pos = 0;
}

bool MDFNConsole::TextHook(UTF8 *text)
{
 WriteLine(text);
 free((char *)text);
 return(1);
}

void MDFNConsole::Scroll(int amount)
{
 int64 ts = Scrolled;
 ts += amount;

 if(ts < 0)
  ts = 0;
 Scrolled = ts;
}

#include "../string/ConvertUTF.h"
int MDFNConsole::Event(const SDL_Event *event)
{
  switch(event->type)
  {
   case SDL_KEYDOWN:
                    if(event->key.keysym.mod & KMOD_ALT)
                     break;
                    switch(event->key.keysym.sym)
                    {
		     case SDLK_HOME:
			kb_cursor_pos = 0;
			break;
		     case SDLK_END:
			kb_cursor_pos = kb_buffer.size();
			break;
	             case SDLK_LEFT:
	                if(kb_cursor_pos)
	                 kb_cursor_pos--;
	                break;
	             case SDLK_RIGHT:
	                if(kb_cursor_pos < kb_buffer.size())
	                 kb_cursor_pos++;
	                break;

		     case SDLK_UP: Scroll(1); break;
		     case SDLK_DOWN: Scroll(-1); break;

                     case SDLK_RETURN:
                     {
                      std::string concat_str;
                      for(unsigned int i = 0; i < kb_buffer.size(); i++)
                       concat_str += kb_buffer[i];

		      TextHook((UTF8*)strdup(concat_str.c_str()));
                      kb_buffer.clear();
		      kb_cursor_pos = 0;
                     }
                     break;
	           case SDLK_BACKSPACE:
	                if(kb_buffer.size() && kb_cursor_pos)
	                {
	                  kb_buffer.erase(kb_buffer.begin() + kb_cursor_pos - 1, kb_buffer.begin() + kb_cursor_pos);
	                  kb_cursor_pos--;
	                }
	                break;
  	            case SDLK_DELETE:
	                if(kb_buffer.size() && kb_cursor_pos < kb_buffer.size())
	                {
	                 kb_buffer.erase(kb_buffer.begin() + kb_cursor_pos, kb_buffer.begin() + kb_cursor_pos + 1);
	                }
	                break;
                     default:
		     if(event->key.keysym.unicode >= 0x20)
                     {
                      uint8 utf8_buffer[8];
                      UTF8 *dest_ptr = utf8_buffer;
                      memset(utf8_buffer, 0, sizeof(utf8_buffer));
                      const UTF16 *start_utf16 = &event->key.keysym.unicode;
                      ConvertUTF16toUTF8(&start_utf16, (UTF16 *)&event->key.keysym.unicode + 1, &dest_ptr, &utf8_buffer[8], lenientConversion);
	              kb_buffer.insert(kb_buffer.begin() + kb_cursor_pos, std::string((char *)utf8_buffer));
	              kb_cursor_pos++;
                     }
                     break;
                    }
                    break;
  }
 return(1);
}

#define MK_COLOR_A(r,g,b,a) ( ((a)<<tmp_surface->format->Ashift) | ((r)<<tmp_surface->format->Rshift) | ((g) << tmp_surface->format->Gshift) | ((b) << tmp_surface->format->Bshift))

void MDFNConsole::Draw(SDL_Surface *surface, const SDL_Rect *src_rect)
{
 uint32 pitch32 = surface->pitch >> 2;
 uint32 w = src_rect->w;
 uint32 h = src_rect->h;
 uint32 *pixels = (uint32 *)surface->pixels + src_rect->x + src_rect->y * pitch32;
 unsigned int font_width;
 unsigned int font_height;

 if(SmallFont)
 {
  font_width = 5;
  font_height = 7;
 }
 else
 {
  font_width = 9;
  font_height = 18;
 }
 SDL_Surface *tmp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, 1024, font_height + 1, 32, 
	surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);

 SDL_SetColorKey(tmp_surface, SDL_SRCCOLORKEY, 0);

 for(unsigned int y = 0; y < h; y++)
 {
  uint32 *row = pixels + y * pitch32;
  for(unsigned int x = 0; x < w; x++)
  {
   //printf("%d %d %d\n", y, x, pixels);
   row[x] = MK_COLOR_A(0, 0, 0, 0xc0);
   //row[x] = MK_COLOR_A(0x00, 0x00, 0x00, 0x7F);
  }
 }
 int32 destline;
 int32 vec_index = TextLog.size() - 1;

 if(vec_index > 0)
 {
  vec_index -= Scrolled;
 }

 destline = ((h - font_height) / font_height) - 1;

 if(shellstyle)
  vec_index--;
 else if(!prompt_visible)
 {
  destline = (h / font_height) - 1;
  //vec_index--;
 }

 while(destline >= 0 && vec_index >= 0)
 {
  int32 pw = GetTextPixLength((UTF8 *)TextLog[vec_index].c_str(), SmallFont ? MDFN_FONT_5x7 : MDFN_FONT_9x18_18x18) + 1;

  if(pw > tmp_surface->w)
  {
   SDL_FreeSurface(tmp_surface);
   tmp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, pw, font_height + 1, 32, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
   SDL_SetColorKey(tmp_surface, SDL_SRCCOLORKEY, 0);
  }
  DrawTextTransShadow((uint32 *)tmp_surface->pixels, tmp_surface->pitch, tmp_surface->w, (UTF8 *)TextLog[vec_index].c_str(), MK_COLOR_A(0xff, 0xff, 0xff, 0xFF), MK_COLOR_A(0x00, 0x00, 0x01, 0xFF), 0, SmallFont);
  int32 numlines = (uint32)ceil((double)pw / w);

  while(numlines > 0 && destline >= 0)
  {
   int32 offs = (numlines - 1) * w;
   SDL_Rect tmp_rect, dest_rect;
   tmp_rect.x = offs;
   tmp_rect.y = 0;
   tmp_rect.h = font_height;
   tmp_rect.w = (pw - offs) > (int32)w ? w : pw - offs;

   dest_rect.x = src_rect->x;
   dest_rect.y = src_rect->y + destline * font_height;
   dest_rect.w = src_rect->w;
   dest_rect.h = src_rect->h;

   SDL_SetAlpha(tmp_surface, 0, SDL_ALPHA_OPAQUE);
   SDL_BlitSurface(tmp_surface, &tmp_rect, surface, &dest_rect);
   numlines--;
   destline--;
  }
  SDL_FillRect(tmp_surface, NULL, 0);
  vec_index--;
 }

 SDL_FreeSurface(tmp_surface);

 if(prompt_visible)
 {
  std::string concat_str;

  if(shellstyle)
  {
   int t = TextLog.size() - 1;
   if(t >= 0)
    concat_str = TextLog[t];
   else
    concat_str = "";
  }
  else
   concat_str = "#>";
  for(unsigned int i = 0; i < kb_buffer.size(); i++)
  {
   if(i == kb_cursor_pos && (SDL_GetTicks() & 0x100))
    concat_str += "▉";
   else
    concat_str += kb_buffer[i];
  }

  if(kb_cursor_pos == kb_buffer.size())
  {
   if(SDL_GetTicks() & 0x100)
    concat_str += "▉";
   else
    concat_str += " ";
  }
  tmp_surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, GetTextPixLength((UTF8*)concat_str.c_str()) + 1, font_height + 1, 32,
        surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);

  SDL_SetColorKey(tmp_surface, SDL_SRCCOLORKEY, 0);
  SDL_FillRect(tmp_surface, NULL, 0x00);


  SDL_Rect tmp_rect, dest_rect;

  tmp_rect.w = DrawTextTransShadow((uint32 *)tmp_surface->pixels, tmp_surface->pitch, tmp_surface->w, (UTF8*)concat_str.c_str(),MK_COLOR_A(0xff, 0xff, 0xff, 0xff), MK_COLOR_A(0x00, 0x00, 0x01, 0xFF), 0, SmallFont);
  tmp_rect.h = dest_rect.h = font_height;
  tmp_rect.x = 0;
  tmp_rect.y = 0;

  if(tmp_rect.w >= w)
  {
   tmp_rect.x = tmp_rect.w - w;
   tmp_rect.w -= tmp_rect.x;
  }
  dest_rect.w = tmp_rect.w;
  dest_rect.x = src_rect->x;
  dest_rect.y = src_rect->y + h - (font_height + 1);

  SDL_SetAlpha(tmp_surface, 0, SDL_ALPHA_OPAQUE);
  SDL_BlitSurface(tmp_surface, &tmp_rect, surface, &dest_rect);
  SDL_FreeSurface(tmp_surface);
 }
}

void MDFNConsole::WriteLine(UTF8 *text)
{
 TextLog.push_back(std::string((char *)text));
}

void MDFNConsole::AppendLastLine(UTF8 *text)
{
 if(TextLog.size()) // Should we throw an exception if this isn't true?
  TextLog[TextLog.size() - 1] += std::string((char*)text);
}

void MDFNConsole::ShowPrompt(bool shown)
{
 prompt_visible = shown;
}
