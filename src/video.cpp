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
#include "video.h"
#include "fceu.h"
#include "file.h"
#include "utils/memory.h"
#include "state.h"
#include "palette.h"
#include "input.h"
#include "drawing.h"
#include "driver.h"
#include "drivers/common/vidblit.h"

//XBuf:
//0-63 is reserved for 7 special colours used by FCEUX (overlay, etc.)
//64-127 is the most-used emphasis setting per frame
//128-195 is the palette with no emphasis
//196-255 is the palette with all emphasis bits on
u8 *XBuf=NULL; //used for current display
//u8 *XBackBuf=NULL; //ppu output is stashed here before drawing happens
//u8 *XDBuf=NULL; //corresponding to XBuf but with deemph bits
//u8 *XDBackBuf=NULL; //corresponding to XBackBuf but with deemph bits
int ClipSidesOffset=0;	//Used to move displayed messages when Clips left and right sides is checked
static u8 *xbsave=NULL;

void FCEU_KillVirtualVideo(void)
{
}

/**
* Return: Flag that indicates whether the function was succesful or not.
*
* TODO: This function is Windows-only. It should probably be moved.
**/
int FCEU_InitVirtualVideo(void)
{
	//Some driver code may allocate XBuf externally.
	//256 bytes per scanline, * 240 scanline maximum, +16 for alignment,
	if(XBuf)
		return 1;

	XBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	//XBackBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	//XDBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	//XDBackBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	if(!XBuf)
	//if(!XBuf || !XBackBuf || !XDBuf || !XDBackBuf)
	{
		return 0;
	}

	xbsave = XBuf;

	if( sizeof(uint8*) == 4 )
	{
		uintptr_t m = (uintptr_t)XBuf;
		m = ( 8 - m) & 7;
		XBuf+=m;
	}

	memset(XBuf,128,256*256);
	//memset(XBackBuf,128,256*256);

	return 1;
}

#ifdef FRAMESKIP
void FCEU_PutImageDummy(void)
{
	ShowFPS();
}
#endif

uint64 FCEUD_GetTime(void);
uint32 FCEUD_GetTimeFreq(void);
bool Show_FPS = false;

void FCEUI_SetShowFPS(bool showFPS)
{
	Show_FPS = showFPS;
}

static uint64 boop[60];
static int boopcount = 0;

void ShowFPS(void)
{
	static uint32 tsc = 0;
	if(Show_FPS == false)
		return;
	uint32 now = FCEUD_GetTime();
	uint32 da = now - boop[boopcount];
	int booplimit = PAL?50:60;
	boop[boopcount] = now;

	if (now - tsc > 1000) {
		tsc = now;
		for (int i = 0; i < 40; i ++) putch('\b');
		printf("(System time: %ds) FPS = %d", now / 1000, booplimit * FCEUD_GetTimeFreq() / da);
	}
	// It's not averaging FPS over exactly 1 second, but it's close enough.
	boopcount = (boopcount + 1) % booplimit;
}
