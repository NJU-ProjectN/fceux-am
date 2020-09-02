#ifndef _FCEU_FILE_H_
#define _FCEU_FILE_H_

#define MAX_MOVIEFILENAME_LEN 80

#include "types.h"
#include "emufile.h"

extern bool bindSavestate;

struct FCEUFILE {
	//the stream you can use to access the data
	//std::iostream *stream;
	EMUFILE_FILE *stream;

	//the number of files that were in the archive
	int archiveCount;

	//the index of the file within the archive
	int archiveIndex;

	//the size of the file
	int size;

	//whether the file is contained in an archive
	bool isArchive() { return archiveCount > 0; }

	FCEUFILE() {}

	~FCEUFILE()
	{
		if(stream) {
      stream->~EMUFILE_FILE();
      free(stream);
    }
	}

	enum {
		READ, WRITE, READWRITE
	} mode;
};

struct ArchiveScanRecord
{
	ArchiveScanRecord()
		: type(-1)
		, numFilesInArchive(0)
	{}
	ArchiveScanRecord(int _type, int _numFiles)
	{
		type = _type;
		numFilesInArchive = _numFiles;
	}
	int type;

	//be careful: this is the number of files in the archive.
	//the size of the files variable might be different.
	int numFilesInArchive;

	bool isArchive() { return type != -1; }
};


FCEUFILE *FCEU_fopen(const char *path, const char *ipsfn, const char *mode, char *ext, int index=-1, const char** extensions = 0, int* userCancel = 0);
bool FCEU_isFileInArchive(const char *path);
int FCEU_fclose(FCEUFILE*);
uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
uint64 FCEU_fwrite(void *ptr, size_t size, size_t nmemb, FCEUFILE*);
int FCEU_fseek(FCEUFILE*, long offset, int whence);
uint64 FCEU_ftell(FCEUFILE*);
int FCEU_read32le(uint32 *Bufo, FCEUFILE*);
int FCEU_read16le(uint16 *Bufo, FCEUFILE*);
int FCEU_fgetc(FCEUFILE*);
uint64 FCEU_fgetsize(FCEUFILE*);
int FCEU_fisarchive(FCEUFILE*);


void GetFileBase(const char *f);

#define FCEUMKF_STATE        1
#define FCEUMKF_SNAP         2
#define FCEUMKF_SAV          3
#define FCEUMKF_CHEAT        4
#define FCEUMKF_FDSROM       5
#define FCEUMKF_PALETTE      6
#define FCEUMKF_GGROM        7
#define FCEUMKF_IPS          8
#define FCEUMKF_FDS          9
#define FCEUMKF_MOVIE        10
//#define FCEUMKF_NPTEMP       11 //mbg 6/21/08 - who needs this..?
#define FCEUMKF_MOVIEGLOB    12
#define FCEUMKF_STATEGLOB    13
#define FCEUMKF_MOVIEGLOB2   14
#define FCEUMKF_AUTOSTATE	 15
#define FCEUMKF_MEMW         16
#define FCEUMKF_BBOT         17
#define FCEUMKF_ROMS         18
#define FCEUMKF_INPUT        19
#define FCEUMKF_LUA          20
#define FCEUMKF_AVI			 21
#define FCEUMKF_TASEDITOR    22
#define FCEUMKF_RESUMESTATE  23
#endif
