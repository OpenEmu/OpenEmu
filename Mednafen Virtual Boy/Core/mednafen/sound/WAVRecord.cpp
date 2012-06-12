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

#include "../mednafen.h"
#include "WAVRecord.h"
#include <algorithm>

WAVRecord::WAVRecord(const char *path, double SoundRate_arg, uint32 SoundChan_arg) : wavfile(path, FileWrapper::MODE_WRITE_SAFE)
{
 Finished = false;
 PCMBytesWritten = 0;

 SoundRate = SoundRate_arg;
 SoundChan = SoundChan_arg;

 memset(&raw_headers, 0, sizeof(raw_headers));

 MDFN_en32msb(&raw_headers[0x00], 0x52494646);	// "RIFF"
 // @ 0x04 = total file size - 8 bytes
 MDFN_en32msb(&raw_headers[0x08], 0x57415645);	// "WAVE"


 MDFN_en32msb(&raw_headers[0x0C], 0x666d7420);	// "fmt "
 MDFN_en32lsb(&raw_headers[0x10], 16);
 MDFN_en16lsb(&raw_headers[0x14], 1);		// PCM format
 MDFN_en16lsb(&raw_headers[0x16], SoundChan);	// Number of sound channels
 MDFN_en32lsb(&raw_headers[0x18], SoundRate);	// Sampling rate
 MDFN_en32lsb(&raw_headers[0x1C], SoundRate * SoundChan * sizeof(int16));	//Byte rate
 MDFN_en16lsb(&raw_headers[0x20], SoundChan * sizeof(int16));	// Block("audio frame" in Mednafen) alignment
 MDFN_en16lsb(&raw_headers[0x22], sizeof(int16) * 8);	// Bits per sample.

 MDFN_en32msb(&raw_headers[0x24], 0x64617461);	// "data"
 // @ 0x28 = bytes of PCM data following

 wavfile.write(raw_headers, sizeof(raw_headers));
}

void WAVRecord::WriteSound(const int16 *SoundBuf, uint32 NumSoundFrames)
{
 uint32 NumSoundSamples = NumSoundFrames * SoundChan;

 while(NumSoundSamples > 0)
 {
  int16 swap_buf[256];
  uint32 s_this_time = std::min((uint32)NumSoundSamples, (uint32)256);

  for(uint32 i = 0; i < s_this_time; i++)
   MDFN_en16lsb((uint8 *)&swap_buf[i], SoundBuf[i]);

  wavfile.write(swap_buf, s_this_time * sizeof(int16));
  PCMBytesWritten += s_this_time * sizeof(int16);
  NumSoundSamples -= s_this_time;
  SoundBuf += s_this_time;
 }

}


void WAVRecord::Finish(void)
{
 if(Finished)
  return;

 MDFN_en32lsb(&raw_headers[0x04], std::min(wavfile.tell() - 8, (int64)0xFFFFFFFFLL));

 MDFN_en32lsb(&raw_headers[0x28], std::min(PCMBytesWritten, (int64)0xFFFFFFFFLL));

 wavfile.seek(0, SEEK_SET);
 wavfile.write(raw_headers, sizeof(raw_headers));
 wavfile.close();

 Finished = true;
}

WAVRecord::~WAVRecord()
{
 try
 {
  Finish();
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
 }
}
