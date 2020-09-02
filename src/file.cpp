/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "types.h"
#include "file.h"
#include "utils/memory.h"
#include "utils/md5.h"
#include "driver.h"
#include "types.h"
#include "fceu.h"
#include "state.h"
#include "driver.h"

using namespace std;

FCEUFILE * FCEU_fopen(const char *path, const char *ipsfn, const char *mode, char *ext, int index, const char** extensions, int* userCancel)
{
	FCEUFILE *fceufp=0;

  assert(ipsfn == NULL);

	bool read = !strcmp(mode, "rb");
	bool write = !strcmp(mode, "wb");
	if((read && write) || (!read && !write))
	{
		FCEU_PrintError("invalid file open mode specified (only wb and rb are supported)");
		return 0;
	}

	if(read)
	{
			//if the archive contained no files, try to open it the old fashioned way
			//EMUFILE* fp = FCEUD_UTF8_fstream(fileToOpen.c_str(),mode);
			EMUFILE_FILE* fp = FCEUD_UTF8_fstream(path,mode);
			if(!fp)
				return 0;
			if (!fp->is_open())
			{
				//fp is new'ed so it has to be deleted
				free(fp);
				return 0;
			}

			//open a plain old file
			fceufp = (FCEUFILE *)malloc(sizeof(FCEUFILE));
			fceufp->archiveIndex = -1;
			fceufp->archiveCount = -1;
			fceufp->stream = fp;
			FCEU_fseek(fceufp,0,SEEK_END);
			fceufp->size = FCEU_ftell(fceufp);
			FCEU_fseek(fceufp,0,SEEK_SET);

		return fceufp;
	}
	return 0;
}

int FCEU_fclose(FCEUFILE *fp)
{
  fp->~FCEUFILE();
  free(fp);
	return 1;
}

uint64 FCEU_fread(void *ptr, size_t size, size_t nmemb, FCEUFILE *fp)
{
	return fp->stream->_fread((char*)ptr,size*nmemb);
}

int FCEU_fseek(FCEUFILE *fp, long offset, int whence)
{
	fp->stream->fseek(offset,whence);

	return FCEU_ftell(fp);
}

uint64 FCEU_ftell(FCEUFILE *fp)
{
	return fp->stream->ftell();
}
