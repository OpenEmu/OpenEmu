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

//don't use emufile for files bigger than 2GB! you have been warned! some day this will be fixed.

#ifndef EMUFILE_H
#define EMUFILE_H

#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <string>
#include <stdarg.h>
#include <stdlib.h>

#include "emufile_types.h"

#ifdef _MSC_VER
#include <io.h>
#endif

class EMUFILE
{
	protected:
	bool failbit;

	public:
	EMUFILE()
		: failbit(false)
	{}


	//returns a new EMUFILE which is guranteed to be in memory. the EMUFILE you call this on may be deleted. use the returned EMUFILE in its place

	virtual ~EMUFILE() {}

	bool fail(bool unset=false) { bool ret = failbit; if(unset) failbit = false; return ret; }

	size_t fread(void *ptr, size_t bytes){
		return _fread(ptr,bytes);
	}

	//virtuals
	public:

	virtual FILE *get_fp() = 0;


	virtual int fgetc() = 0;
	virtual int fputc(int c) = 0;

	virtual size_t _fread(void *ptr, size_t bytes) = 0;

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	virtual void fwrite(const void *ptr, size_t bytes) = 0;

	virtual int fseek(int offset, int origin) = 0;

	virtual int ftell() = 0;
	virtual int size() = 0;

	virtual void truncate(s32 length) = 0;
};

//todo - handle read-only specially?
class EMUFILE_MEMORY : public EMUFILE
{ 
	protected:
	std::vector<u8> *vec;
	bool ownvec;
	s32 pos, len;

	void reserve(u32 amt) {
		if(vec->size() < amt)
			vec->resize(amt);
	}

	public:

	EMUFILE_MEMORY(std::vector<u8> *underlying) : vec(underlying), ownvec(false), pos(0), len((s32)underlying->size()) { }
	EMUFILE_MEMORY(u32 preallocate) : vec(new std::vector<u8>()), ownvec(true), pos(0), len(0) { 
		vec->resize(preallocate);
		len = preallocate;
	}
	EMUFILE_MEMORY() : vec(new std::vector<u8>()), ownvec(true), pos(0), len(0) { vec->reserve(1024); }

	EMUFILE_MEMORY(const void* buf, s32 size) : vec(new std::vector<u8>()), ownvec(true), pos(0), len(size)
   { 
		vec->resize(size);
		if(size != 0)
			memcpy(&(*vec)[0],buf,size);
	}

	~EMUFILE_MEMORY() {
		if(ownvec) delete vec;
	}

	virtual void truncate(s32 length)
	{
		vec->resize(length);
		len = length;
		if(pos>length) pos=length;
	}

	u8* buf() { 
		if(size()==0) reserve(1);
		return &(*vec)[0];
	}

	std::vector<u8>* get_vec() { return vec; };

	virtual FILE *get_fp() { return NULL; }

	virtual int fgetc() {
		u8 temp;

		//need an optimized codepath
		//if(_fread(&temp,1) != 1)
		//	return EOF;
		//else return temp;
		u32 remain = len-pos;
		if(remain<1) {
			failbit = true;
			return -1;
		}
		temp = buf()[pos];
		pos++;
		return temp;
	}
	virtual int fputc(int c) {
		u8 temp = (u8)c;
		//TODO
		//if(fwrite(&temp,1)!=1) return EOF;
		fwrite(&temp,1);

		return 0;
	}

	virtual size_t _fread(void *ptr, size_t bytes);

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	virtual void fwrite(const void *ptr, size_t bytes){
		reserve(pos+(s32)bytes);
		memcpy(buf()+pos,ptr,bytes);
		pos += (s32)bytes;
		len = std::max(pos,len);
	}

	virtual int fseek(int offset, int origin){ 
		//work differently for read-only...?
		switch(origin) {
			case SEEK_SET:
				pos = offset;
				break;
			case SEEK_CUR:
				pos += offset;
				break;
			case SEEK_END:
				pos = size()+offset;
				break;
			default:
				break;
		}
		reserve(pos);
		return 0;
	}

	virtual int ftell() {
		return pos;
	}

	virtual int size() { return (int)len; }
};

class EMUFILE_FILE : public EMUFILE
{
	protected:
	FILE* fp;
	std::string fname;
	char mode[16];

	private:
	void open(const char* fname, const char* mode)
	{
		fp = fopen(fname,mode);
		if(!fp)
			failbit = true;
		this->fname = fname;
		strcpy(this->mode,mode);
	}

	public:

	EMUFILE_FILE(const std::string& fname, const char* mode) { open(fname.c_str(),mode); }
	EMUFILE_FILE(const char* fname, const char* mode) { open(fname,mode); }

	virtual ~EMUFILE_FILE() {
		if(NULL != fp)
			fclose(fp);
	}

	virtual FILE *get_fp() {
		return fp; 
	}

	virtual void truncate(s32 length);

	virtual int fgetc() {
		return ::fgetc(fp);
	}
	virtual int fputc(int c) {
		return ::fputc(c, fp);
	}

	virtual size_t _fread(void *ptr, size_t bytes){
		size_t ret = ::fread((void*)ptr, 1, bytes, fp);
		if(ret < bytes)
			failbit = true;
		return ret;
	}

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	virtual void fwrite(const void *ptr, size_t bytes){
		size_t ret = ::fwrite((void*)ptr, 1, bytes, fp);
		if(ret < bytes)
			failbit = true;
	}

	virtual int fseek(int offset, int origin){ 
		return ::fseek(fp, offset, origin);
	}

	virtual int ftell() {
		return (u32)::ftell(fp);
	}

	virtual int size() { 
		int oldpos = ftell();
		fseek(0,SEEK_END);
		int len = ftell();
		fseek(oldpos,SEEK_SET);
		return len;
	}

};

#endif
