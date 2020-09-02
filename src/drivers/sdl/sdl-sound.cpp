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

#include "sdl.h"
#include <amdev.h>
#include "../../config.h"

// unit: 16-bit
static unsigned int s_BufferSize;

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
  const int soundbufsize = 128;
  _DEV_AUDIO_INIT_t init;
  init.freq = soundrate;
  init.channels = 1;
  init.samples = 512;
  init.bufsize = soundbufsize * soundrate / 1000;
  if (init.bufsize < init.samples * 2) {
    init.bufsize = init.samples * 2;
  }
  s_BufferSize = init.bufsize;
  init.bufsize *= sizeof(int16_t);
  _io_write(_DEV_AUDIO, _DEVREG_AUDIO_INIT, &init, sizeof(init));
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
  _DEV_AUDIO_SBSTAT_t audio;
  _io_read(_DEV_AUDIO, _DEVREG_AUDIO_SBSTAT, &audio, sizeof(audio));
  audio.count /= sizeof(int16_t);
  return s_BufferSize - audio.count;
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
  extern int EmulationPaused;
  if (EmulationPaused == 0) {
    // FECUX stores each PCM sample as 32-bit data,
    // but it sets the audio type with AUDIO_S16SYS,
    // so we should transform the `buf` into a 16-bit
    // buffer before feeding it to the audio device
    int16_t *buf16 = (int16_t *)malloc(sizeof(buf16[0]) * Count);
    assert(buf16);
    int i;
    for (i = 0; i < Count; i ++) {
      buf16[i] = buf[i];
    }

    _DEV_AUDIO_SBCTRL_t audio;
    audio.stream = (uint8_t *)buf16;
    audio.wait = 1;
    audio.len = Count * sizeof(buf16[0]);
    _io_write(_DEV_AUDIO, _DEVREG_AUDIO_SBCTRL, &audio, sizeof(audio));
    free(buf16);
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
