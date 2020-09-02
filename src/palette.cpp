/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 2002,2003 Xodnizel
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
#include "fceu.h"
#include "driver.h"
#include "boards/mapinc.h"

#include "palette.h"
#include "palettes/palettes.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//#include <math.h>

//the default basic palette
int default_palette_selection = 0;

//library of default palettes
static pal *default_palette[8]=
{
	palette,
	rp2c04001,
	rp2c04002,
	rp2c04003,
	rp2c05004,
};

static void ChoosePalette(void);
static void WritePalette(void);

//points to the actually selected current palette
pal *palo;

#define RGB_TO_YIQ( r, g, b, y, i ) (\
	(y = (r) * 0.299f + (g) * 0.587f + (b) * 0.114f),\
	(i = (r) * 0.596f - (g) * 0.275f - (b) * 0.321f),\
	((r) * 0.212f - (g) * 0.523f + (b) * 0.311f)\
)

#define YIQ_TO_RGB( y, i, q, to_rgb, type, r, g ) (\
	r = (type) (y + to_rgb [0] * i + to_rgb [1] * q),\
	g = (type) (y + to_rgb [2] * i + to_rgb [3] * q),\
	(type) (y + to_rgb [4] * i + to_rgb [5] * q)\
)

//float bisqwit_gammafix(float f, float gamma) { return f < 0.f ? 0.f : std::pow(f, 2.2f / gamma); }
int bisqwit_clamp(int v) { return v<0 ? 0 : v>255 ? 255 : v; }

// Calculate the luma and chroma by emulating the relevant circuits:
int bisqwit_wave(int p, int color) { return (color+p+8)%12 < 6; }

#if 0
static void ApplyDeemphasisBisqwit(int entry, u8& r, u8& g, u8& b)
{
	if(entry<64) return;
	int myr = 0, myg = 0, myb = 0;
	// The input value is a NES color index (with de-emphasis bits).
	// We need RGB values. Convert the index into RGB.
	// For most part, this process is described at:
	//    http://wiki.nesdev.com/w/index.php/NTSC_video

	// Decode the color index
	int color = (entry & 0x0F), level = color<0xE ? (entry>>4) & 3 : 1;

	// Voltage levels, relative to synch voltage
	static const float black=.518f, white=1.962f, attenuation=.746f,
		levels[8] = {.350f, .518f, .962f,1.550f,  // Signal low
		1.094f,1.506f,1.962f,1.962f}; // Signal high

	float lo_and_hi[2] = { levels[level + 4 * (color == 0x0)],
		levels[level + 4 * (color <  0xD)] };



	//fceux alteration: two passes
	//1st pass calculates bisqwit's base color
	//2nd pass calculates it with deemph
	//finally, we'll do something dumb: find a 'scale factor' between them and apply it to the input palette. (later, we could pregenerate the scale factors)
	//whatever, it gets the job done.
	for(int pass=0;pass<2;pass++)
	{
		float y=0.f, i=0.f, q=0.f, gamma=1.8f;
		for(int p=0; p<12; ++p) // 12 clock cycles per pixel.
		{
			// NES NTSC modulator (square wave between two voltage levels):
			float spot = lo_and_hi[bisqwit_wave(p,color)];

			// De-emphasis bits attenuate a part of the signal:
			if(pass==1)
			{
				if(((entry & 0x40) && bisqwit_wave(p,12))
					|| ((entry & 0x80) && bisqwit_wave(p, 4))
					|| ((entry &0x100) && bisqwit_wave(p, 8))) spot *= attenuation;
			}

			// Normalize:
			float v = (spot - black) / (white-black) / 12.f;

			// Ideal TV NTSC demodulator:
			y += v;
			i += v * std::cos(3.141592653 * p / 6);
			q += v * std::sin(3.141592653 * p / 6); // Or cos(... p-3 ... )
			// Note: Integrating cos() and sin() for p-0.5 .. p+0.5 range gives
			//       the exactly same result, scaled by a factor of 2*cos(pi/12).
		}

		// Convert YIQ into RGB according to FCC-sanctioned conversion matrix.

		int rt = bisqwit_clamp(255 * bisqwit_gammafix(y +  0.946882f*i +  0.623557f*q,gamma));
		int gt = bisqwit_clamp(255 * bisqwit_gammafix(y + -0.274788f*i + -0.635691f*q,gamma));
		int bt = bisqwit_clamp(255 * bisqwit_gammafix(y + -1.108545f*i +  1.709007f*q,gamma));

		if(pass==0) myr = rt, myg = gt, myb = bt;
		else
		{
			float rscale = (float)rt / myr;
			float gscale = (float)gt / myg;
			float bscale = (float)bt / myb;
			#define BCLAMP(x) ((x)<0?0:((x)>255?255:(x)))
			if(myr!=0) r = (u8)(BCLAMP(r*rscale));
			if(myg!=0) g = (u8)(BCLAMP(g*gscale));
			if(myb!=0) b = (u8)(BCLAMP(b*bscale));
		}
	}



}

static void ApplyDeemphasisComplete(pal* pal512)
{
	//for each deemph level beyond 0
	for(int i=0,idx=0;i<8;i++)
	{
		//for each palette entry
		for(int p=0;p<64;p++,idx++)
		{
			pal512[idx] = pal512[p];
			ApplyDeemphasisBisqwit(idx,pal512[idx].r,pal512[idx].g,pal512[idx].b);
		}
	}
}
#endif

void FCEU_LoadGamePalette(void)
{
	FCEU_ResetPalette();
}

void FCEUI_SetNTSCTH(bool en, int tint, int hue)
{
	FCEU_ResetPalette();
}

//this prepares the 'deemph' palette which was a horrible idea to jam a single deemph palette into 0xC0-0xFF of the 8bpp palette.
//its needed for GUI and lua and stuff, so we're leaving it, despite having a newer codepath for applying deemph
static uint8 lastd=0;
void SetNESDeemph_OldHacky(uint8 d, int force)
{
	static uint16 rtmul[]={
        static_cast<uint16>(32768*1.239),
        static_cast<uint16>(32768*.794),
        static_cast<uint16>(32768*1.019),
        static_cast<uint16>(32768*.905),
        static_cast<uint16>(32768*1.023),
        static_cast<uint16>(32768*.741),
        static_cast<uint16>(32768*.75)
    };
	static uint16 gtmul[]={
        static_cast<uint16>(32768*.915),
        static_cast<uint16>(32768*1.086),
        static_cast<uint16>(32768*.98),
        static_cast<uint16>(32768*1.026),
        static_cast<uint16>(32768*.908),
        static_cast<uint16>(32768*.987),
        static_cast<uint16>(32768*.75)
    };
	static uint16 btmul[]={
        static_cast<uint16>(32768*.743),
        static_cast<uint16>(32768*.882),
        static_cast<uint16>(32768*.653),
        static_cast<uint16>(32768*1.277),
        static_cast<uint16>(32768*.979),
        static_cast<uint16>(32768*.101),
        static_cast<uint16>(32768*.75)
    };

	uint32 r,g,b;
	int x;

	/* If it's not forced(only forced when the palette changes),
	don't waste cpu time if the same deemphasis bits are set as the last call.
	*/
	if(!force)
	{
		if(d==lastd)
			return;
	}
	else   /* Only set this when palette has changed. */
	{
		r=rtmul[6];
		g=rtmul[6];
		b=rtmul[6];

		for(x=0;x<0x40;x++)
		{
			uint32 m,n,o;
			m=palo[x].r;
			n=palo[x].g;
			o=palo[x].b;
			m=(m*r)>>15;
			n=(n*g)>>15;
			o=(o*b)>>15;
			if(m>0xff) m=0xff;
			if(n>0xff) n=0xff;
			if(o>0xff) o=0xff;
			FCEUD_SetPalette(x|0xC0,m,n,o);
		}
	}
	if(!d) return; /* No deemphasis, so return. */

	r=rtmul[d-1];
	g=gtmul[d-1];
	b=btmul[d-1];

	for(x=0;x<0x40;x++)
	{
		uint32 m,n,o;

		m=palo[x].r;
		n=palo[x].g;
		o=palo[x].b;
		m=(m*r)>>15;
		n=(n*g)>>15;
		o=(o*b)>>15;
		if(m>0xff) m=0xff;
		if(n>0xff) n=0xff;
		if(o>0xff) o=0xff;

		FCEUD_SetPalette(x|0x40,m,n,o);
	}

	lastd=d;
}

void FCEU_ResetPalette(void)
{
	if(GameInfo)
	{
		ChoosePalette();
		WritePalette();
	}
}

static void ChoosePalette(void)
{
  palo = default_palette[default_palette_selection];
  //need to calcualte a deemph on the fly.. sorry. maybe support otherwise later
  //ApplyDeemphasisComplete(palo);
}

void WritePalette(void)
{
	int x;

	//set the 'unvarying' palettes to low < 64 palette entries
	const int unvaried = sizeof(palette_unvarying)/sizeof(palette_unvarying[0]);
	for(x=0;x<unvaried;x++)
		FCEUD_SetPalette(x,palette_unvarying[x].r,palette_unvarying[x].g,palette_unvarying[x].b);

	//clear everything else to a deterministic state.
	//it seems likely that the text rendering on NSF has been broken since the beginning of fceux, depending on palette entries 205,205,205 everywhere
	//this was just whatever msvc filled malloc with. on non-msvc platforms, there was no backdrop on the rendering.
	for(x=unvaried;x<256;x++)
		FCEUD_SetPalette(x,205,205,205);

	//sets palette entries >= 128 with the 64 selected main colors
	for(x=0;x<64;x++)
		FCEUD_SetPalette(128+x,palo[x].r,palo[x].g,palo[x].b);
	SetNESDeemph_OldHacky(lastd,1);
}
