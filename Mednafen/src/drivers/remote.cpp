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
#include "../string/escape.h"

#if defined(HAVE_FCNTL) && defined(HAVE_FCNTL_H)
#include <trio/trio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

// Code for the stdio emulator interface.
static char *InputBuffer;
static int64 InputBufferLen;
static int64 InputBufferOffset;

// This kludge gives me nightmares.
//static bool CaptureErrorMessages = FALSE;
static std::string CapturedError;

static void Remote_SendCommand(const char *command, int nargs, ...)
{
 va_list ap;
 va_start(ap, nargs);

 printf("%s ", command);

 for(int i = 0; i < nargs; i++)
 {
  char *escaped_string = escape_string(va_arg(ap, char *));

  fputc('"', stdout);
  fputs(escaped_string, stdout);
  fputc('"', stdout);
  fputc(' ', stdout);

  free(escaped_string);
 }
 printf("\n");
 va_end(ap);
}

static void ParseSTDIOCommand(char *buf)
{
 char *arguments[8];
 int which_argument = 0;
 size_t blen = strlen(buf);
 bool InQuote = 0;
 char *InBegin = NULL;
 char last_char = 0;

 memset(arguments, 0, sizeof(arguments));

 for(size_t x = 0; x <= blen; x++)
 {
  if((buf[x] == ' ' || buf[x] == 0) && last_char != '\\' && !InQuote && InBegin)
  {
   buf[x] = 0;

   unescape_string(InBegin);
   arguments[which_argument] = InBegin;
   which_argument++;
   if(which_argument == 8)
   {
    break;
   }
   InBegin = NULL;
  }
  else if(buf[x] != ' ' && buf[x] != '\t' && buf[x] != '"' && last_char != '\\' && !InQuote && !InBegin)
  {
   InBegin = &buf[x];
  }
  else if(buf[x] == '"' && last_char != '\\')
  {
   if(InBegin)
   {
    buf[x] = 0;
    unescape_string(InBegin);
    arguments[which_argument] = InBegin;
    which_argument++;
    if(which_argument == 8)
    {
     break;
    }
    InQuote = 0;
    InBegin = NULL;
   }
   else
   {
    InQuote = TRUE;
    InBegin = &buf[x + 1];
   }
  }
  last_char = buf[x];
 }

 int numargs = which_argument; // numargs count includes the command.  Misleading, yes!
 bool success = 0;
 bool suppress_success = 0;

 if(numargs)
 {
  if(!strcasecmp(arguments[0], "exit"))
  {
   MainRequestExit();
   suppress_success = TRUE;
  }
  else if(!strcasecmp(arguments[0], "sync_video"))
  {
   success = MT_FromRemote_VideoSync();
  }
  else if(!strcasecmp(arguments[0], "sync_sound"))
  {
   success = MT_FromRemote_SoundSync();
  }
  else if(!strcasecmp(arguments[0], "get_setting"))
  {
   if(numargs == 2)
   {
    std::string sval;

    LockGameMutex(TRUE);

    sval = MDFN_GetSettingS(arguments[1]);
    success = TRUE;

    LockGameMutex(FALSE);

    Remote_SendCommand("return", 2, "success", sval.c_str());
    return;
   }
   else
    puts("Invalid number of arguments");
  }
  else if(!strcasecmp(arguments[0], "set_setting"))
  {
   if(numargs == 3)
   {
    LockGameMutex(TRUE);
    success = MDFNI_SetSetting(arguments[1], arguments[2]);
    LockGameMutex(FALSE);
   }
   else
    puts("Invalid number of arguments");
  }
  else if(!strcasecmp(arguments[0], "get_known_fileexts"))
  {
   uint32 count = 0;
   char count_string[64];

   // TODO:  Move to core
   for(unsigned int i = 0; i < MDFNSystems.size(); i++)
   {
    const FileExtensionSpecStruct *FileExtensions = MDFNSystems[i]->FileExtensions;
    if(FileExtensions)
     while(FileExtensions->extension)
     {
      count++;
      FileExtensions++;
     }
   }
   trio_snprintf(count_string, 64, "%u", count);
   Remote_SendCommand("result_count", 1, count_string);

   for(unsigned int i = 0; i < MDFNSystems.size(); i++)
   {
    const FileExtensionSpecStruct *FileExtensions = MDFNSystems[i]->FileExtensions;
    if(FileExtensions)
     while(FileExtensions->extension)
     {
      Remote_SendCommand("result", 3, MDFNSystems[i]->shortname, FileExtensions->extension, FileExtensions->description);
      FileExtensions++;
     }
   }
   success = TRUE;
  }
  else
  {
   puts("Unknown command!");
  }
 }

 if(!suppress_success)
 {
  if(success)
  {
   Remote_SendCommand("return", 1, "success");
  }
  else
  {
   Remote_SendCommand("return", 1, "failure");
  }
 }
 //printf("%s %s %s %s %s %s\n", arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
}

// Called from the main thread.
void CheckForSTDIOMessages(void)
{
 char inval;

 while(read(fileno(stdin), &inval, 1) == 1)
 {
  if(InputBufferOffset == InputBufferLen)
  {
   InputBufferLen += 256;
   InputBuffer = (char*)realloc(InputBuffer, InputBufferLen);
  }

  if(inval == '\n')
  {
   InputBuffer[InputBufferOffset] = 0;

   for(int64 x = 0; x < InputBufferOffset; x++)
    if(InputBuffer[x] == '\r')
     InputBuffer[x] = 0;

   ParseSTDIOCommand(InputBuffer);
   InputBufferOffset = 0;
  }
  else
   InputBuffer[InputBufferOffset++] = inval;

 }
}

bool InitSTDIOInterface(void)
{
 if(fcntl(fileno(stdin), F_SETFL, O_NONBLOCK | O_RDONLY))
 {
  puts("fcntl error");
  return(0);
 }

 InputBuffer = NULL;
 InputBufferLen = 0; 
 InputBufferOffset = 0;
 
 return(TRUE);
}

void Remote_SendStatusMessage(const char *message)
{
 Remote_SendCommand("status_message", 1, message);
}

void Remote_SendErrorMessage(const char *message)
{
 Remote_SendCommand("error_message", 1, message);
}


#else

void CheckForSTDIOMessages(void)
{

}

bool InitSTDIOInterface(void)
{
 return(0);
}

void Remote_SendStatusMessage(const char *message)
{

}

void Remote_SendErrorMessage(const char *message)
{

}

#endif
