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

#include <klib-macros.h>
#include "sdl.h"
#include "../common/vidblit.h"
#include "../../fceu.h"
#include "../../version.h"
#include "../../video.h"

#include "../../utils/memory.h"

#include "sdl-icon.h"
#include "dface.h"

#include "sdl-video.h"
#include "../../config.h"

static int s_srendline;
//static int s_erendline;
static const int s_tlines = 240;
static int s_inited;

#define s_clipSides 0

#define NWIDTH	(256 - (s_clipSides ? 16 : 0))
#define NOFFSET	(s_clipSides ? 8 : 0)

static int s_paletterefresh;

int
KillVideo()
{
	// return failure if the video system was not initialized
	if(s_inited == 0)
		return -1;

	// if the rest of the system has been initialized, shut it down
		// shut down the system that converts from 8 to 16/32 bpp
  KillBlitToHigh();

	s_inited = 0;
	return 0;
}


void FCEUD_VideoChanged()
{
  PAL = 0; // NTSC and Dendy
}

/**
 * Attempts to initialize the graphical video display.  Returns 0 on
 * success, -1 on failure.
 */
int
InitVideo(FCEUGI *gi)
{
	FCEUI_printf("Initializing video...");

	// check the starting, ending, and total scan lines
	//FCEUI_GetCurrentVidSystem(&s_srendline, &s_erendline);
	//s_tlines = s_erendline - s_srendline + 1;

	s_inited = 1;

	FCEUI_SetShowFPS(true);

	s_paletterefresh = 1;

	 // if using more than 8bpp, initialize the conversion routines
	InitBlitToHigh(4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0, 0, 0);
	return 0;
}

static struct {
  uint8 r, g, b, unused;
} s_psdl[256];

/**
 * Sets the color for a particular index in the palette.
 */
void
FCEUD_SetPalette(uint8 index,
                 uint8 r,
                 uint8 g,
                 uint8 b)
{
	s_psdl[index].r = r;
	s_psdl[index].g = g;
	s_psdl[index].b = b;

	s_paletterefresh = 1;
}

static uint32_t canvas[NWIDTH * 240];

/**
 * Pushes the given buffer of bits to the screen.
 */
void
BlitScreen(uint8 *XBuf)
{
	// refresh the palette if required
	if(s_paletterefresh) {
    SetPaletteBlitToHigh((uint8*)s_psdl);
		s_paletterefresh = 0;
	}

	// XXX soules - not entirely sure why this is being done yet
	XBuf += s_srendline * 256;

	// XXX soules - again, I'm surprised SDL can't handle this
	// perform the blit, converting bpp if necessary
  Blit8ToHigh(XBuf + NOFFSET, (uint8 *)canvas, NWIDTH, s_tlines, NWIDTH * 4, 1, 1);

	int scrw = NWIDTH;


  // ensure that the display is updated
#ifdef HAS_GUI
  int x = (io_read(AM_GPU_CONFIG).width - 256) / 2;
  int y = (io_read(AM_GPU_CONFIG).height - 240) / 2;
  io_write(AM_GPU_FBDRAW, x, y, canvas, scrw, s_tlines, true);
#else
  printf("\033[0;0H");
  for (int y = 0; y < s_tlines; y += 4) {
    //draw_rect(&screen[y][8], xpad, ypad + y, W, 1);
    for (int x = 0; x < scrw; x += 2) {
      uint32_t color = canvas[y * 256 + x];
      const char *list = "o. *O0@#";
      char c = list[color / 0x222222u];
      putch(c);
    }
    putch('\n');
  }
#endif
}
