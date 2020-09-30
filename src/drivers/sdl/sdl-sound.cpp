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

/// \file
/// \brief Handles sound emulation using the SDL.

#include <klib-macros.h>
#include "sdl.h"
#include "../../config.h"

// unit: 16-bit
static int s_BufferSize;

/**
 * Initialize the audio subsystem.
 */
  int
InitSound()
{
#if SOUND_CONFIG == SOUND_NONE
  return 1;
#endif

#if SOUND_CONFIG == SOUND_HQ
  const int soundrate = 44100;
  const int soundq = 1;
#else
  const int soundrate = 11025;
  const int soundq = 0;
#endif
  const int soundvolume = 150;
  const int soundtrianglevolume = 256;
  const int soundsquare1volume = 256;
  const int soundsquare2volume = 256;
  const int soundnoisevolume = 256;
  const int soundpcmvolume = 256;

#if SOUND_CONFIG != SOUND_NONE
  if (!io_read(AM_AUDIO_CONFIG).present) {
    printf("WARNING: %s does not support audio\n", TOSTRING(__ARCH__));
    return 1;
  }

  const int soundbufsize = 128;
  int samples = 512;
  s_BufferSize = soundbufsize * soundrate / 1000;
  if (s_BufferSize < samples * 2) {
    s_BufferSize = samples * 2;
  }
  io_write(AM_AUDIO_CTRL, soundrate, 1, samples);
#endif

  FCEUI_SetSoundVolume(soundvolume);
  FCEUI_SetSoundQuality(soundq);
  FCEUI_Sound(soundrate);
  FCEUI_SetTriangleVolume(soundtrianglevolume);
  FCEUI_SetSquare1Volume(soundsquare1volume);
  FCEUI_SetSquare2Volume(soundsquare2volume);
  FCEUI_SetNoiseVolume(soundnoisevolume);
  FCEUI_SetPCMVolume(soundpcmvolume);
  return 1;
}


/**
 * Returns the size of the audio buffer.
 */
  uint32
GetMaxSound(void)
{
  return(s_BufferSize);
}

/**
 * Returns the amount of free space in the audio buffer.
 */
  uint32
GetWriteSound(void)
{
#if SOUND_CONFIG != SOUND_NONE
  int count = io_read(AM_AUDIO_STATUS).count / sizeof(int16_t);
  return s_BufferSize - count;
#else
  return 0;
#endif
}

/**
 * Send a sound clip to the audio subsystem.
 */
  void
WriteSound(int32 *buf,
    int Count)
{
#if SOUND_CONFIG != SOUND_NONE
  static int16_t buf16[2048+512] = {};
  extern int EmulationPaused;
  if (EmulationPaused == 0) {
    // FECUX stores each PCM sample as 32-bit data,
    // but it sets the audio type with AUDIO_S16SYS,
    // so we should transform the `buf` into a 16-bit
    // buffer before feeding it to the audio device
    assert(Count < (int)LENGTH(buf16));
    int i;
    for (i = 0; i < Count; i ++) {
      buf16[i] = buf[i];
    }

    Area sbuf;
    sbuf.start = buf16;
    int free;
    while (Count) {
      while ((free = GetWriteSound()) == 0);  // wait until there is free space
      int nrplay = (free > Count ? Count : free);
      sbuf.end = (uint16_t *)sbuf.start + nrplay;
      io_write(AM_AUDIO_PLAY, sbuf);
      Count -= nrplay;
      sbuf.start = sbuf.end;
    }
  }
#endif
}

/**
 * Pause (1) or unpause (0) the audio output.
 */
  void
SilenceSound(int n)
{
  //SDL_PauseAudio(n);
}

/**
 * Shut down the audio subsystem.
 */
  int
KillSound(void)
{
  FCEUI_Sound(0);
  return 0;
}


/**
 * Adjust the volume either down (-1), up (1), or to the default (0).
 * Unmutes if mute was active before.
 */
  void
FCEUD_SoundVolumeAdjust(int n)
{
}

/**
 * Toggles the sound on or off.
 */
  void
FCEUD_SoundToggle(void)
{
}
