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

#include <stdarg.h>
#include "types.h"

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

class EMUFILE {
protected:
	bool failbit;

public:
	EMUFILE()
		: failbit(false)
	{}


	virtual ~EMUFILE() {}

	bool fail(bool unset=false) { bool ret = failbit; if(unset) unfail(); return ret; }
	void unfail() { failbit=false; }

	bool eof() { return size()==ftell(); }

public:

	virtual bool is_open() = 0;

	virtual int fprintf(const char *format, ...) = 0;

	virtual int fgetc() = 0;
	virtual int fputc(int c) = 0;

	virtual size_t _fread(const void *ptr, size_t bytes) = 0;

	//removing these return values for now so we can find any code that might be using them and make sure
	//they handle the return values correctly

	virtual void fwrite(const void *ptr, size_t bytes) = 0;

	virtual int fseek(int offset, int origin) = 0;

	virtual int ftell() = 0;
	virtual int size() = 0;
	virtual void fflush() = 0;
};

#ifdef __NO_FILE_SYSTEM__

class EMUFILE_FILE {
protected:
	u8* data;
	const char* fname;
	char mode[16];
	bool failbit;

private:
  int curpos;
  int filesize;

public:

	void open(const char* fname, const char* mode);

	EMUFILE_FILE(const char* fname, const char* mode) { open(fname,mode); }

	~EMUFILE_FILE() { }

bool is_open() { return true; }
	int fprintf(const char *format, ...) { return 0; };

	int fgetc() {
  if (size()==ftell()) {
      printf("%s: Can not read pass EOF\n", __func__);
      return -1;
    }
    int ret = (int)data[curpos];
    curpos ++;
		return ret;
	}

	int fputc(int c) {
    assert(0);
    return 0;
	}

	size_t _fread(const void *ptr, size_t bytes){
    size_t remain = filesize - curpos;
    size_t ret = (bytes <= remain ? bytes : remain);
    memcpy(const_cast<void *>(ptr), data + curpos, ret);
    curpos += ret;
		if(ret < bytes)
			failbit = true;
		return ret;
	}

	void fwrite(const void *ptr, size_t bytes){
    failbit = true;
    assert(0);
	}

	int fseek(int offset, int origin) {
    switch (origin) {
      case SEEK_END: curpos = filesize; break;
      case SEEK_SET: curpos = offset; break;
      case SEEK_CUR: curpos += offset; break;
    }
    if (curpos < 0) curpos = 0;
    else if (curpos > filesize) curpos = filesize;
    return 0;
	}

	int ftell() { return curpos; }
	int size() { return filesize; }
	void fflush() { }

};

#else

class EMUFILE_FILE : public EMUFILE {
protected:
	FILE* fp;
	const char* fname;
	char mode[16];

private:
	void open(const char* fname, const char* mode);

public:

	//EMUFILE_FILE(const std::string& fname, const char* mode) { open(fname.c_str(),mode); }
	EMUFILE_FILE(const char* fname, const char* mode) { open(fname,mode); }

	virtual ~EMUFILE_FILE() {
		if(NULL != fp)
			fclose(fp);
	}

	bool is_open() { return fp != NULL; }

	virtual int fprintf(const char *format, ...) {
		va_list argptr;
		va_start(argptr, format);
		int ret = ::vfprintf(fp, format, argptr);
		va_end(argptr);
		return ret;
	};

	virtual int fgetc() {
		return ::fgetc(fp);
	}
	virtual int fputc(int c) {
		return ::fputc(c, fp);
	}

	virtual size_t _fread(const void *ptr, size_t bytes){
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

	virtual int fseek(int offset, int origin) {
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

	virtual void fflush() {
		::fflush(fp);
	}
};

#endif

#endif
