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
#include "main.h"
#include "dface.h"
#include "input.h"

#include "sdl-video.h"
#include "sdl.h"

#include "../../fceu.h"
#include "../../driver.h"

/** GLOBALS **/
int NoWaiting = 1;

/* UsrInputType[] is user-specified.  CurInputType[] is current
        (game loading can override user settings)
*/
static int UsrInputType[NUM_INPUT_DEVICES];
static int CurInputType[NUM_INPUT_DEVICES];
static int cspec = 0;

extern int gametype;

/**
 * Necessary for proper GUI functioning (configuring when a game isn't loaded).
 */
void
InputUserActiveFix ()
{
	int x;
	for (x = 0; x < 3; x++)
	{
		CurInputType[x] = UsrInputType[x];
	}
}

/**
 * Parse game information and configure the input devices accordingly.
 */
void
ParseGIInput (FCEUGI * gi)
{
	gametype = gi->type;

	CurInputType[0] = UsrInputType[0];
	CurInputType[1] = UsrInputType[1];
	CurInputType[2] = UsrInputType[2];

	if (gi->input[0] >= 0)
	{
		CurInputType[0] = gi->input[0];
	}
	if (gi->input[1] >= 0)
	{
		CurInputType[1] = gi->input[1];
	}
	if (gi->inputfc >= 0)
	{
		CurInputType[2] = gi->inputfc;
	}
	cspec = gi->cspecial;
}


static void UpdateGamepad (void);

static uint32 JSreturn = 0;

#include "keyscan.h"
static uint8 g_keyState[256] = {};

/**
* Hook for transformer board
*/
unsigned int *GetKeyboard(void)
{
	return (unsigned int*)(g_keyState);
}

/**
 * Parse keyboard commands and execute accordingly.
 */
static void KeyboardCommands ()
{
	// get the keyboard input
  int keycode;
  do {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    assert(ev.keycode < 256);
    keycode = ev.keycode;
    g_keyState[keycode] = ev.keydown;
    if (ev.keydown && (keycode == AM_KEY_ESCAPE || keycode == AM_KEY_Q)) halt(0);
  } while (keycode != AM_KEY_NONE);

	// Toggle throttling
	NoWaiting &= ~1;
}

/**
 * Return the state of the mouse buttons.  Input 'd' is an array of 3
 * integers that store <x, y, button state>.
 */
void				// removed static for a call in lua-engine.cpp
GetMouseData (uint32 (&d)[3])
{
  return;
}

/**
 * Tests to see if a specified button is currently pressed.
 */
static int
DTestButton (ButtConfig * bc)
{
	int x;

	for (x = 0; x < (int)bc->NumC; x++)
	{
		if (bc->ButtType[x] == BUTTC_KEYBOARD)
		{
			if (g_keyState[bc->ButtonNum[x]])
			{
				return 1;
			}
		}
    }
  return 0;
}


#define MK(x)       {{BUTTC_KEYBOARD},{0},{MKK(x)},1}
#define MK2(x1,x2)  {{BUTTC_KEYBOARD},{0},{MKK(x1),MKK(x2)},2}
#define MKZ()       {{0},{0},{0},0}
#define GPZ()       {MKZ(), MKZ(), MKZ(), MKZ()}

ButtConfig GamePadConfig[4][10] = {
	/* Gamepad 1 */
	{MK (J), MK (K), MK (U), MK (I),
	MK (W), MK (S), MK (A), MK (D), MKZ (), MKZ ()},

	/* Gamepad 2 */
	GPZ (),

	/* Gamepad 3 */
	GPZ (),

	/* Gamepad 4 */
	GPZ ()
};

/**
 * Update the status of the gamepad input devices.
 */
static void
UpdateGamepad(void)
{
	static int rapid = 0;
	uint32 JS = 0;
	int x;
	int wg;

	rapid ^= 1;

	// go through each of the four game pads
	for (wg = 0; wg < 4; wg++)
	{
		// a, b, select, start, up, down, left, right
		for (x = 0; x < 8; x++)
		{
			if (DTestButton (&GamePadConfig[wg][x]))
			{
				JS |= (1 << x) << (wg << 3);
			}
		}

		// rapid-fire a, rapid-fire b
		if (rapid)
		{
			for (x = 0; x < 2; x++)
			{
				if (DTestButton (&GamePadConfig[wg][8 + x]))
				{
					JS |= (1 << x) << (wg << 3);
				}
			}
		}
	}

	JSreturn = JS;
}

/**
 * Update all of the input devices required for the active game.
 */
void FCEUD_UpdateInput ()
{
	int x;
	int t = 0;

	KeyboardCommands ();

	for (x = 0; x < 2; x++)
	{
		switch (CurInputType[x])
		{
			case SI_GAMEPAD:
				t |= 1;
				break;
		}
	}

	if (t & 1)
	{
		UpdateGamepad ();
	}
}

/**
 * Initialize the input device interface between the emulation and the driver.
 */
void InitInputInterface ()
{
	void *InputDPtr;

	int x;
	int attrib;

	for (x = 0; x < 2; x++)
	{
		attrib = 0;
		InputDPtr = 0;

		switch (CurInputType[x])
		{
			case SI_GAMEPAD:
				InputDPtr = &JSreturn;
				break;
		}
		FCEUI_SetInput (x, (ESI) CurInputType[x], InputDPtr, attrib);
	}

	attrib = 0;
	InputDPtr = 0;

	FCEUI_SetInputFC ((ESIFC) CurInputType[2], InputDPtr, attrib);
	FCEUI_SetInputFourscore ((eoptions & EO_FOURSCORE) != 0);
}

/**
 * Hack to map the new configuration onto the existing button
 * configuration management.  Will probably want to change this in the
 * future - soules.
 */
	void UpdateInput ()
{
	for (unsigned int i = 0; i < 3; i++) {
    UsrInputType[i] = (i < 2) ? (int) SI_GAMEPAD : (int) SIFC_NONE;
	}

	// gamepad 0 - 3
	for (unsigned int i = 0; i < GAMEPAD_NUM_DEVICES; i++) {
    int type = (i == 0 ? BUTTC_KEYBOARD : 0);

		for (unsigned int j = 0; j < GAMEPAD_NUM_BUTTONS; j++) {
			GamePadConfig[i][j].ButtType[0] = type;
			GamePadConfig[i][j].DeviceNum[0] = 0;
			GamePadConfig[i][j].ButtonNum[0] = DefaultGamePad[i][j];
			GamePadConfig[i][j].NumC = 1;
		}
	}
}

// Definitions from main.h:
// GamePad defaults
const char *GamePadNames[GAMEPAD_NUM_BUTTONS] = { "A", "B", "Select", "Start",
	"Up", "Down", "Left", "Right", "TurboA", "TurboB"
};
const char *DefaultGamePadDevice[GAMEPAD_NUM_DEVICES] =
{ "Keyboard", "None", "None", "None" };
const int DefaultGamePad[GAMEPAD_NUM_DEVICES][GAMEPAD_NUM_BUTTONS] =
{ {AM_KEY_J, AM_KEY_K, AM_KEY_U, AM_KEY_I,
	AM_KEY_W, AM_KEY_S, AM_KEY_A, AM_KEY_D, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
