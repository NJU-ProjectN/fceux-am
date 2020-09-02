#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SOUND_NONE 0
#define SOUND_LQ   1
#define SOUND_HQ   2

#if defined(__PLATFORM_NEMU__)
# define NR_FRAMESKIP 1
# define SOUND_CONFIG SOUND_LQ
#elif defined(__PLATFORM_NOOP__) || defined(__PLATFORM_SDI__) || defined(__PLATFORM_NAVY__)
# define NR_FRAMESKIP 2
# define SOUND_CONFIG SOUND_NONE
#else
# define NR_FRAMESKIP 0
# define SOUND_CONFIG SOUND_HQ
#endif

#endif
