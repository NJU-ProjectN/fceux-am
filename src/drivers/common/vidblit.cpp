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

#include "../../fceu.h"
#include "../../types.h"
#include "../../palette.h"
#include "../../utils/memory.h"

extern u8 *XBuf;
//extern u8 *XDBuf;
extern pal *palo;

static uint32 CBM[3];
static uint32 *palettetranslate=0;
static int Bpp;	// BYTES per pixel

bool   paldeemphswap   = 0;

static void CalculateShift(uint32 *CBM, int *cshiftr, int *cshiftl)
{
	int a,x,z;
	cshiftl[0]=cshiftl[1]=cshiftl[2]=-1;
	for(a=0;a<3;a++)
	{
		for(x=0,z=0;x<32;x++)
		{
			if(CBM[a]&(1<<x))
			{
				if(cshiftl[a]==-1) cshiftl[a]=x;
				z++;
			}
		}
		cshiftr[a]=(8-z);
	}
}

int InitBlitToHigh(int b, uint32 rmask, uint32 gmask, uint32 bmask, int efx, int specfilt, int specfilteropt)
{
  assert(b == 4);

	Bpp=b;

	if(Bpp<=1 || Bpp>4)
		return(0);

	//allocate adequate room for 32bpp palette
	palettetranslate=(uint32*)FCEU_dmalloc(256*4 + 512*4);

	if(!palettetranslate)
		return(0);


	CBM[0]=rmask;
	CBM[1]=gmask;
	CBM[2]=bmask;
	return(1);
}

void KillBlitToHigh(void)
{
	if(palettetranslate)
	{
		free(palettetranslate);
		palettetranslate=NULL;
	}
}


void SetPaletteBlitToHigh(uint8 *src)
{
	int cshiftr[3];
	int cshiftl[3];

	CalculateShift(CBM, cshiftr, cshiftl);

  assert(Bpp == 4);
		for(int x=0;x<256;x++)
		{
			uint32 r=src[x<<2];
			uint32 g=src[(x<<2)+1];
			uint32 b=src[(x<<2)+2];
			palettetranslate[x]=(r<<cshiftl[0])|(g<<cshiftl[1])|(b<<cshiftl[2]);
		}

		//full size deemph palette
		if(palo)
		{
			for(int x=0;x<512;x++)
			{
				uint32 r=palo[x].r;
				uint32 g=palo[x].g;
				uint32 b=palo[x].b;
				palettetranslate[256+x]=(r<<cshiftl[0])|(g<<cshiftl[1])|(b<<cshiftl[2]);
			}
		}
}

void Blit8ToHigh(uint8 *src, uint8 *dest, int xr, int yr, int pitch, int xscale, int yscale)
{
	int x,y;

  assert(Bpp == 4);
  uint32 * dest32 = (uint32 *)dest;
  int pinc=pitch-(xr<<2);
  assert(xr % 8 == 0);
  assert(pinc % 4 == 0);
  for(y=yr;y;y--,src+=256-xr) {
    for(x=xr;x;x-=8) {
      //THE MAIN BLITTING CODEPATH (there may be others that are important)
      //*(uint32 *)dest = ModernDeemphColorMap(src,XBuf,1,1);

      //look up the legacy translation,
      //do not support deemph palette to optimize performance
#define macro() *dest32 ++ = palettetranslate[*src ++]
      macro(); macro(); macro(); macro();
      macro(); macro(); macro(); macro();
    }
    dest32+=(pinc / 4);
  }
}
