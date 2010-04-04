/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "main.h"
#include <trio/trio.h>
#include "prompt.h"
#include "../string/ConvertUTF.h"

HappyPrompt::HappyPrompt(void)
{
        PromptText = "";
	kb_buffer.clear();
        kb_cursor_pos = 0;
}

HappyPrompt::HappyPrompt(const std::string &ptext, const std::string &zestring)
{
        PromptText = "";
        kb_buffer.clear();
        kb_cursor_pos = 0;
	SetText(ptext);
	SetKBB(zestring);
}

HappyPrompt::~HappyPrompt()
{

}

void HappyPrompt::TheEnd(const std::string &pstring)
{


}

void HappyPrompt::SetText(const std::string &ptext)
{
 PromptText = ptext;
}

void HappyPrompt::SetKBB(const std::string &zestring)
{
 UTF8 *utf8_buffer = (UTF8*)strdup(zestring.c_str());
 UTF32 *utf32_buffer;
 const UTF8 *sourceStart = utf8_buffer, *sourceEnd = utf8_buffer + zestring.size();
 UTF32 *targetStart, *targetEnd;

 utf32_buffer = (UTF32 *)calloc(1, zestring.size() * 4);
 targetStart = utf32_buffer;
 targetEnd = utf32_buffer + zestring.size() * 4;


 if(ConvertUTF8toUTF32(&sourceStart, sourceEnd, &targetStart, targetEnd,lenientConversion) == conversionOK)
 {
  size_t meow_count = targetStart - utf32_buffer;

  for(unsigned int x = 0; x < meow_count; x++)
  {
   kb_buffer.push_back(utf32_buffer[x]);
   kb_cursor_pos++;
  }
 }

 free(utf32_buffer);
}

#define MK_COLOR_A(r,g,b,a) ( ((a)<<surface->format->Ashift) | ((r)<<surface->format->Rshift) | ((g) << surface->format->Gshift) | ((b) << surface->format->Bshift))

void HappyPrompt::Draw(SDL_Surface *surface, const SDL_Rect *rect)
{
 std::vector<uint32> PromptAnswer;
 std::vector<uint32> PromptAnswerOffsets;
 std::vector<uint32> PromptAnswerWidths;
 uint32 offset_accum = 0;

 PromptAnswerOffsets.clear();
 PromptAnswerWidths.clear();

 for(unsigned int i = 0; i < kb_buffer.size(); i++)
 {
  uint32 gw;
  UTF32 tmp_str[2];

  PromptAnswer.push_back(kb_buffer[i]);
  PromptAnswerOffsets.push_back(offset_accum);

  tmp_str[0] = kb_buffer[i];
  tmp_str[1] = 0;

  gw = GetTextPixLength(tmp_str, MDFN_FONT_6x13_12x13);

  offset_accum += gw;
  PromptAnswerWidths.push_back(gw);
 }
 PromptAnswer.push_back(0);
 PromptAnswerOffsets.push_back(offset_accum);

/*
******************************
* PC                         *
* F00F                       *
******************************
*/
 uint32 *pixels = (uint32 *)surface->pixels;
 uint32 pitch32 = surface->pitch >> 2;

 pixels += ((rect->h / 2) - (4 * 13 / 2)) * pitch32;
 pixels += ((rect->w / 2) - (6 * 39 / 2));

 for(unsigned int y = 0; y < 4 * 13; y++)
 {
  uint32 *row = pixels + y * pitch32;
  for(unsigned int x = 0; x < 39 * 6; x++)
   row[x] = MK_COLOR_A(0x00, 0x00, 0x00, 0xFF);
 }

 DrawTextTrans(pixels, surface->pitch, 39 * 6, (UTF8 *)     	          "╭────────────────────────────────────╮", MK_COLOR_A(0xFF,0xFF,0xFF,0xFF), 0, MDFN_FONT_6x13_12x13);
 DrawTextTrans(pixels + 13 * 1 * pitch32, surface->pitch, 39 * 6, (UTF8 *)"│                                    │", MK_COLOR_A(0xFF,0xFF,0xFF,0xFF), 0, MDFN_FONT_6x13_12x13);
 DrawTextTrans(pixels + 13 * 2 * pitch32, surface->pitch, 39 * 6, (UTF8 *)"│                                    │", MK_COLOR_A(0xFF,0xFF,0xFF,0xFF), 0, MDFN_FONT_6x13_12x13);
 DrawTextTrans(pixels + 13 * 3 * pitch32, surface->pitch, 39 * 6, (UTF8 *)"╰────────────────────────────────────╯", MK_COLOR_A(0xFF,0xFF,0xFF,0xFF), 0, MDFN_FONT_6x13_12x13);

 DrawTextTrans(pixels + 13 * 1 * pitch32 + 2 * 6, surface->pitch, (39 - 4) * 6, (UTF8 *)PromptText.c_str(), MK_COLOR_A(0xFF, 0x00, 0xFF, 0xFF), 0, MDFN_FONT_6x13_12x13);
 DrawTextTrans(pixels + (13 * 2 + 2) * pitch32 + 2 * 6, surface->pitch, (39 - 4) * 6, &PromptAnswer[0], MK_COLOR_A(0x00, 0xFF, 0x00, 0xFF), 0, MDFN_FONT_6x13_12x13);

 if(SDL_GetTicks() & 0x80)
 {
  uint32 xpos;

  xpos = PromptAnswerOffsets[kb_cursor_pos];
  if(xpos < ((39 - 4) * 6))
  {
   UTF8 *blinky_thingy = (UTF8*)"▉";
   if(PromptAnswerWidths.size() > kb_cursor_pos && PromptAnswerWidths[kb_cursor_pos] == 12)
    blinky_thingy = (UTF8*)"▉▉";
   DrawTextTrans(pixels + (13 * 2 + 2) * pitch32 + 2 * 6 + xpos, surface->pitch, 9, blinky_thingy, MK_COLOR_A(0x00, 0xFF, 0x00, 0xFF), 0, MDFN_FONT_6x13_12x13);
  }
 }
}

void HappyPrompt::Event(const SDL_Event *event)
{
 if(event->type == SDL_KEYDOWN)
 {
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
   case SDLK_RETURN:
        {
         std::string concat_str;

         for(unsigned int i = 0; i < kb_buffer.size(); i++)
          concat_str += kb_buffer[i];

	 TheEnd(concat_str);
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
	        if(event->key.keysym.unicode)
        	{
	         kb_buffer.insert(kb_buffer.begin() + kb_cursor_pos, event->key.keysym.unicode);
        	 kb_cursor_pos++;
	        }
        	break;
  }
 }
}
