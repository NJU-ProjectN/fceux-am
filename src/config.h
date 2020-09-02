#ifndef __CONFIG_H__
#define __CONFIG_H__

#define HAS_GUI

#define SOUND_NONE 0
#define SOUND_LQ   1
#define SOUND_HQ   2

#if defined(__PLATFORM_NEMU)
# define NR_FRAMESKIP 1
# define SOUND_CONFIG SOUND_NONE
#elif defined(__PLATFORM_NAVY)
# define NR_FRAMESKIP 2
# define SOUND_CONFIG SOUND_NONE
#else
# define NR_FRAMESKIP 0
//# define SOUND_CONFIG SOUND_HQ
# define SOUND_CONFIG SOUND_NONE
#endif

#endif
