/* 
Copyright (C) 2009-2010 DeSmuME team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "emufile.h"

size_t EMUFILE_MEMORY::_fread(void *ptr, size_t bytes){
	u32 remain = len-pos;
	u32 todo = std::min<u32>(remain,(u32)bytes);
	if(len==0)
	{
		failbit = true;
		return 0;
	}
	if(todo<=4)
	{
		u8* src = buf()+pos;
		u8* dst = (u8*)ptr;
		for(size_t i=0;i<todo;i++)
			*dst++ = *src++;
	}
	else
	{
		memcpy(ptr,buf()+pos,todo);
	}
	pos += todo;
	if(todo<bytes)
		failbit = true;
	return todo;
}

void EMUFILE_FILE::truncate(s32 length)
{
	fflush(fp);
	#ifdef _MSC_VER
		_chsize(_fileno(fp),length);
	#else
		ftruncate(fileno(fp),length);
	#endif
	fclose(fp);
	fp = NULL;
	open(fname.c_str(),mode);
}
