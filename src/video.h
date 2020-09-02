#ifndef _VIDEO_H_
#define _VIDEO_H_
int FCEU_InitVirtualVideo(void);
void FCEU_KillVirtualVideo(void);
int SaveSnapshot(void);
int SaveSnapshot(char[]);
void ResetScreenshotsCounter();
uint32 GetScreenPixel(int x, int y, bool usebackup);
int GetScreenPixelPalette(int x, int y, bool usebackup);
extern uint8 *XBuf;
extern uint8 *XBackBuf;
extern uint8 *XDBuf;
extern uint8 *XDBackBuf;
extern int ClipSidesOffset;

void FCEU_DrawNumberRow(uint8 *XBuf, int *nstatus, int cur);

bool FCEUI_ShowFPS();
void FCEUI_SetShowFPS(bool showFPS);
void FCEUI_ToggleShowFPS();
void ShowFPS();
void snapAVI();
#endif
